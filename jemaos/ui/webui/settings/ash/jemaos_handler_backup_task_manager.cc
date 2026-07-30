// Copyright (c) 2023 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "jemaos/ui/webui/settings/ash/jemaos_handler_backup_task_manager.h"

#include <sys/stat.h>

#include "ash/constants/notifier_catalogs.h"
#include "ash/public/cpp/notification_utils.h"
#include "base/base64.h"
#include "base/files/file_util.h"
#include "base/hash/sha1.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/notreached.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/thread_pool.h"
#include "base/time/time.h"
#include "chrome/browser/notifications/notification_display_service.h"
#include "chrome/browser/notifications/notification_display_service_factory.h"
#include "chrome/browser/notifications/notification_handler.h"
#include "chrome/browser/platform_util.h"
#include "chrome/grit/generated_resources.h"
#include "chrome/grit/theme_resources.h"
#include "components/vector_icons/vector_icons.h"
#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/jemaos_shell_client.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/message_center/message_center.h"
#include "ui/message_center/public/cpp/notification.h"

using message_center::MessageCenter;
using message_center::Notification;

namespace ash::settings {

namespace {
const char kJemaOSBackupScriptPath[] = "/tmp/jemaos-backup-script.sh";
// Installed copy shipped in the OS image by the jemaos-utils package. Prefer
// it over the embedded fallback written to /tmp.
const char kJemaOSInstalledBackupScriptPath[] =
    "/usr/share/jemaos-backup/jemaos-backup.sh";
const char kJemaOSBackupCommandFormat[] =
    "/bin/bash %s backup --email %s --key %s --target %s";
constexpr int kJemaOSBackupTaskTrackIntervalSeconds = 5;
constexpr int kJemaOSBackupTaskOutputLines = 10;

// Unified full-snapshot backup/restore script. Keep in sync with
// jemaos_handler.cc (kRestoreScript) and
// overlays/project-jemaos/chromeos-base/jemaos-utils/files/scripts/jemaos-backup.sh
const char kJemaOSBackupScript[] = R"SCRIPT(#!/bin/bash
# JemaOS unified backup/restore script (full environment snapshot).
#
# backup:  snapshots the whole user profile: user files, browser settings
#          (Preferences, Secure Preferences, Local State), shelf pins,
#          launcher state, installed PWA web apps, bookmarks, history,
#          favicons, wallpaper, dark/light mode, language prefs, plus a
#          best-effort capture of brightness and volume.
# restore: applies the snapshot and restarts the session. Chrome must NOT
#          be running while the files are swapped, otherwise it overwrites
#          the restored files with its in-memory state at shutdown (which is
#          why previous versions "only restored files"). The script freezes
#          Chrome (SIGSTOP), swaps the files in, then kills Chrome (SIGKILL)
#          so no shutdown flush can happen; session_manager immediately
#          respawns Chrome, which reads the restored profile from disk.

LOGFILE="/tmp/jemaos-backup.log"
echo "=== $(date) ===" >> "$LOGFILE"
echo "Script version: unified snapshot v4" >> "$LOGFILE"
echo "Args: $@" >> "$LOGFILE"

EMAIL=""
KEY=""
TARGET=""
PROFILE_PATH="/home/chronos/user"
COMMAND=""

while [[ $# -gt 0 ]]; do
  case $1 in
    backup|restore) COMMAND="$1"; shift ;;
    --email) EMAIL="$2"; shift 2 ;;
    --key) KEY="$2"; shift 2 ;;
    --target)
      DECODED=$(echo "$2" | base64 -d 2>/dev/null)
      if [ -n "$DECODED" ] && [[ "$DECODED" == /* ]]; then
        TARGET="$DECODED"
      else
        TARGET="$2"
      fi
      shift 2 ;;
    --profile) PROFILE_PATH="$2"; shift 2 ;;
    *) shift ;;
  esac
done

echo "Cmd=$COMMAND, Email=$EMAIL, Target=$TARGET" >> "$LOGFILE"

[ -z "$TARGET" ] && echo "Error: No target" && exit 1

# --- Best-effort capture / re-apply of system state -------------------------

read_brightness_percent() {
  # $1: output file
  local b cur max
  for b in /sys/class/backlight/*; do
    if [ -f "$b/brightness" ] && [ -f "$b/max_brightness" ]; then
      cur=$(cat "$b/brightness" 2>/dev/null)
      max=$(cat "$b/max_brightness" 2>/dev/null)
      if [ -n "$cur" ] && [ -n "$max" ] && [ "$max" -gt 0 ] 2>/dev/null; then
        echo $(( cur * 100 / max )) > "$1"
      fi
      return 0
    fi
  done
  return 0
}

read_volume_percent() {
  # $1: output file
  if command -v cras_test_client >/dev/null 2>&1; then
    local vol
    vol=$(cras_test_client --dump_server_state 2>/dev/null \
        | grep -i "output volume" | head -1 | grep -o "[0-9]\+" | head -1)
    [ -n "$vol" ] && echo "$vol" > "$1"
  fi
  return 0
}

apply_brightness_percent() {
  # $1: percent (0-100). Never fails the restore.
  local pct="$1"
  [ -z "$pct" ] && return 0
  local b max
  for b in /sys/class/backlight/*; do
    if [ -f "$b/brightness" ] && [ -f "$b/max_brightness" ]; then
      max=$(cat "$b/max_brightness" 2>/dev/null)
      if [ -n "$max" ] && [ "$max" -gt 0 ] 2>/dev/null; then
        echo $(( pct * max / 100 )) > "$b/brightness" 2>/dev/null || true
      fi
      return 0
    fi
  done
  # Fall back to powerd over D-Bus (callable as chronos).
  dbus-send --system --type=method_call \
    --dest=org.chromium.PowerManager /org/chromium/PowerManager \
    org.chromium.PowerManager.SetScreenBrightnessPercent \
    "double:${pct}.0" int32:0 >/dev/null 2>&1 || true
  return 0
}

apply_volume_percent() {
  # $1: volume (0-100). Never fails the restore.
  local vol="$1"
  [ -z "$vol" ] && return 0
  if command -v cras_test_client >/dev/null 2>&1; then
    cras_test_client --output_volume "$vol" >/dev/null 2>&1 || true
  fi
  return 0
}

# --- backup ------------------------------------------------------------------

if [ "$COMMAND" = "backup" ]; then
  [ ! -d "$PROFILE_PATH" ] && echo "Error: Profile not found" && exit 1
  mkdir -p "$(dirname "$TARGET")" 2>> "$LOGFILE"

  TEMP_DIR="/tmp/jemaos_backup_$$"
  mkdir -p "$TEMP_DIR/system_state"
  echo -e "User: $EMAIL\nDate: $(date)\nSource: full-profile\nVersion: 3" \
    > "$TEMP_DIR/backup_info.txt"

  # Best-effort system state capture for a true snapshot.
  read_brightness_percent "$TEMP_DIR/system_state/brightness_percent"
  read_volume_percent "$TEMP_DIR/system_state/volume_percent"

  EXCLUDES=(
    --exclude='.cache/*'
    --exclude='*/Cache/*'
    --exclude='*/GPUCache/*'
    --exclude='*.bak'
    --exclude='*/backup/*.bak'
    --exclude='.Trash*'
    --exclude='*.tmp'
    --exclude='*.temp'
    --exclude='.jemaos_restore_staging*'
  )

  # Manifest so the archive content is explicit and verifiable.
  {
    echo '{'
    echo "  \"user\": \"$EMAIL\","
    echo "  \"date\": \"$(date -Iseconds)\","
    echo "  \"version\": 3,"
    echo '  "included": ['
    echo '    "user_files",'
    echo '    "browser_settings",'
    echo '    "shelf_pins",'
    echo '    "launcher_state",'
    echo '    "installed_pwa_apps",'
    echo '    "bookmarks",'
    echo '    "history",'
    echo '    "favicons",'
    echo '    "wallpaper",'
    echo '    "dark_light_mode",'
    echo '    "language_prefs",'
    echo '    "brightness",'
    echo '    "volume"'
    echo '  ]'
    echo '}'
  } > "$TEMP_DIR/manifest.json"

  if [ -n "$KEY" ]; then
    tar -czf - "${EXCLUDES[@]}" -C "$PROFILE_PATH" . \
        -C "$TEMP_DIR" backup_info.txt manifest.json system_state \
        2>> "$LOGFILE" \
      | openssl enc -aes-256-cbc -salt -pbkdf2 -pass pass:"$KEY" \
        > "$TARGET" 2>> "$LOGFILE"
  else
    tar -czf "$TARGET" "${EXCLUDES[@]}" -C "$PROFILE_PATH" . \
        -C "$TEMP_DIR" backup_info.txt manifest.json system_state \
        2>> "$LOGFILE"
  fi
  rm -rf "$TEMP_DIR"

  if [ -f "$TARGET" ] && [ -s "$TARGET" ]; then
    echo "Backup completed: $(du -h "$TARGET" | cut -f1)" >> "$LOGFILE"
    echo "Backup completed: $TARGET"
    exit 0
  fi
  echo "Backup failed" >> "$LOGFILE"
  exit 1

# --- restore -----------------------------------------------------------------

elif [ "$COMMAND" = "restore" ]; then
  [ ! -f "$TARGET" ] && echo "Error: File not found: $TARGET" && exit 1

  # Stage 1: decrypt + extract into a staging area on the stateful partition
  # (NOT /tmp, which is a small tmpfs), while Chrome is still running.
  STAGING="/home/chronos/.jemaos_restore_staging_$$"
  rm -rf /home/chronos/.jemaos_restore_staging_* 2>/dev/null || true
  mkdir -p "$STAGING"

  echo "Extracting backup to staging directory..." >> "$LOGFILE"
  if [ -n "$KEY" ]; then
    openssl enc -aes-256-cbc -d -salt -pbkdf2 -pass pass:"$KEY" \
        -in "$TARGET" 2>> "$LOGFILE" \
      | tar -xzf - -C "$STAGING" 2>> "$LOGFILE" \
      || { echo "Decrypt failed"; rm -rf "$STAGING"; exit 1; }
  else
    tar -xzf "$TARGET" -C "$STAGING" 2>> "$LOGFILE" \
      || { echo "Extract failed"; rm -rf "$STAGING"; exit 1; }
  fi

  # Backward compatibility: "v2" backups contain only the MyFiles content
  # (no manifest.json, no Preferences, no MyFiles directory at the root).
  RESTORE_ROOT="$PROFILE_PATH"
  if [ ! -f "$STAGING/manifest.json" ] && \
     [ ! -f "$STAGING/Preferences" ] && \
     [ ! -d "$STAGING/MyFiles" ]; then
    echo "Detected v2 MyFiles-only backup; restoring into MyFiles" >> "$LOGFILE"
    RESTORE_ROOT="$PROFILE_PATH/MyFiles"
    mkdir -p "$RESTORE_ROOT"
  fi

  # Stage 2: freeze Chrome so it cannot rewrite its in-memory settings over
  # the restored files while we swap them in.
  echo "Freezing Chrome..." >> "$LOGFILE"
  killall -STOP chrome 2>> "$LOGFILE" || true

  # Swap the restored content into the profile (merge; staging metadata is
  # not written back).
  (cd "$STAGING" && tar -cf - \
      --exclude='./backup_info.txt' --exclude='./manifest.json' \
      --exclude='./system_state' .) 2>> "$LOGFILE" \
    | tar -xf - -C "$RESTORE_ROOT" 2>> "$LOGFILE"

  # Best-effort re-apply of system state.
  [ -f "$STAGING/system_state/brightness_percent" ] && \
    apply_brightness_percent \
      "$(cat "$STAGING/system_state/brightness_percent" 2>/dev/null)"
  [ -f "$STAGING/system_state/volume_percent" ] && \
    apply_volume_percent \
      "$(cat "$STAGING/system_state/volume_percent" 2>/dev/null)"

  sync
  rm -rf "$STAGING"

  # Kill Chrome without giving it a chance to flush: session_manager
  # immediately respawns it and the new instance reads the restored profile.
  echo "Killing Chrome without shutdown flush; session will restart" \
    >> "$LOGFILE"
  CHROME_PIDS=$(pgrep -x chrome 2>/dev/null || true)
  killall -9 chrome 2>> "$LOGFILE" || true

  # Drop the dead instance's singleton markers right away so the respawned
  # Chrome never tries to hand off to a dying predecessor: a process stuck
  # in uninterruptible I/O can survive SIGKILL for a while and would make
  # the new instance hang on a black screen waiting for a hand-off reply.
  rm -f "$PROFILE_PATH"/SingletonLock "$PROFILE_PATH"/SingletonSocket \
      "$PROFILE_PATH"/SingletonCookie 2>> "$LOGFILE" || true

  # Wait for the killed processes to actually disappear (bounded): SIGKILL
  # teardown is not instantaneous and stragglers still hold DRM/profile
  # locks while session_manager is already respawning Chrome.
  for _ in $(seq 1 150); do
    ALIVE=0
    for pid in $CHROME_PIDS; do
      if [ -e "/proc/$pid" ]; then ALIVE=1; break; fi
    done
    [ "$ALIVE" -eq 0 ] && break
    sleep 0.1
  done

  # Watchdog: if session_manager did not respawn Chrome (crash-loop give
  # up, stuck predecessor, ...), recover instead of leaving a permanent
  # black screen: restart the ui job, and reboot as a last resort (the
  # known-good manual recovery). Detached with stdio redirected so the
  # daemon does not block on our pipes waiting for EOF.
  (
    sleep 40
    if ! pgrep -x chrome >/dev/null 2>&1; then
      logger -t jemaos-restore \
        "Chrome did not respawn after restore; restarting ui"
      restart ui || start ui || true
      sleep 30
      if ! pgrep -x chrome >/dev/null 2>&1; then
        logger -t jemaos-restore \
          "ui restart did not bring Chrome back; rebooting"
        reboot
      fi
    fi
  ) </dev/null >>"$LOGFILE" 2>&1 &

  echo "Restore completed" >> "$LOGFILE"
  echo "Restore completed"
  exit 0
else
  echo "Usage: backup|restore --email EMAIL --key KEY --target PATH"
  exit 1
fi
)SCRIPT";

// Returns the backup/restore script to use: the image-installed copy when
// present, otherwise the embedded fallback (written to /tmp, always refreshed
// so an outdated copy left by an older version is never reused). Returns an
// empty string on failure.
std::string GetJemaOSBackupScriptPath() {
  if (base::PathExists(base::FilePath(kJemaOSInstalledBackupScriptPath))) {
    return kJemaOSInstalledBackupScriptPath;
  }
  base::FilePath script_path(kJemaOSBackupScriptPath);
  if (!base::WriteFile(script_path, kJemaOSBackupScript)) {
    LOG(ERROR) << "Failed to create backup script at "
               << kJemaOSBackupScriptPath;
    return std::string();
  }
  // Make executable
  chmod(kJemaOSBackupScriptPath, 0755);
  return kJemaOSBackupScriptPath;
}

JemaOSShellClient* GetShellClient() {
  return JemaOSShellClient::Get();
}
BackupTaskManager* g_backup_task_manager = nullptr;

const char kJemaOSBackupNotificationId[] = "jemaos.os-settings.backup";
const char kJemaOSBackupNotifierId[] = "jemaos.os-settings.backup";
std::unique_ptr<Notification> CreateNotification(
    BackupTaskManager::TaskState state,
    const std::u16string& title,
    const std::u16string& message,
    scoped_refptr<message_center::NotificationDelegate> delegate) {
  message_center::NotificationType type =
      message_center::NOTIFICATION_TYPE_SIMPLE;
  message_center::SystemNotificationWarningLevel warning_level =
      message_center::SystemNotificationWarningLevel::NORMAL;
  if (state == BackupTaskManager::TaskState::kRunning) {
    type = message_center::NotificationType::NOTIFICATION_TYPE_PROGRESS;
  }
  if (state == BackupTaskManager::TaskState::kFailed) {
    warning_level = message_center::SystemNotificationWarningLevel::WARNING;
  }
  // No vector icon: the Jema logo (raster) is set as the small image below,
  // replacing the default Chrome product icon.
  std::unique_ptr<Notification> notification =
      ::ash::CreateSystemNotificationPtr(
          type, kJemaOSBackupNotificationId, title, message, std::u16string(),
          GURL(),
          message_center::NotifierId(
              message_center::NotifierType::SYSTEM_COMPONENT,
              kJemaOSBackupNotifierId,
              ash::NotificationCatalogName::kJemaOSDataBackup),
          message_center::RichNotificationData(), nullptr,
          gfx::VectorIcon(), warning_level);
  notification->SetSmallImage(
      ui::ResourceBundle::GetSharedInstance().GetImageNamed(IDR_JEMAOS_LOGO));
  if (state == BackupTaskManager::TaskState::kRunning) {
    notification->set_progress(-1);
    notification->set_pinned(true);
    notification->set_never_timeout(true);
  } else {
    notification->set_never_timeout(false);
    notification->set_pinned(false);
    if (delegate) {
      notification->set_delegate(std::move(delegate));
    }
  }
  return notification;
}
const std::string GenerateKey(const std::string& email,
                              const std::string& password) {
  std::string key = email + ":" + password;
  std::string hex_encoded_hash =
      base::HexEncode(base::SHA1Hash(base::as_byte_span(key)));
  hex_encoded_hash.resize(16);
  return base::ToLowerASCII(hex_encoded_hash);
}

bool WriteFile_(const base::FilePath& path, const std::string& content) {
  int ret = base::WriteFile(path, content);
  if (ret != static_cast<int>(content.size())) {
    return false;
  }
  return base::PathExists(path);
}
}  // namespace

void BackupTaskManager::ShowSimpleNotification(
    TaskState state,
    const std::u16string& title,
    const std::u16string& message) {
  // For a new running operation, force remove and re-add the notification
  // to make sure it pops up even if a previous one is still visible.
  if (state == TaskState::kRunning &&
      MessageCenter::Get()->FindVisibleNotificationById(
          kJemaOSBackupNotificationId)) {
    MessageCenter::Get()->RemoveNotification(kJemaOSBackupNotificationId,
                                             false);
  }
  DisplayNotification(CreateNotification(state, title, message, nullptr));
}

BackupTaskManager* BackupTaskManager::GetInstance() {
  if (!g_backup_task_manager) {
    g_backup_task_manager = new BackupTaskManager();
  }
  return g_backup_task_manager;
}

void BackupTaskManager::DestroyInstance() {
  if (g_backup_task_manager) {
    delete g_backup_task_manager;
    g_backup_task_manager = nullptr;
  }
}

BackupTaskManager::BackupTaskManager() {
  task_runner_ = base::ThreadPool::CreateSequencedTaskRunner(
      {base::MayBlock(), base::TaskPriority::BEST_EFFORT});
}

BackupTaskManager::~BackupTaskManager() {
  if (MessageCenter::Get()->FindVisibleNotificationById(
          kJemaOSBackupNotificationId)) {
    MessageCenter::Get()->RemoveNotification(kJemaOSBackupNotificationId,
                                             false);
  }
}

void BackupTaskManager::StartTask(Profile* profile,
                                  const std::string& email,
                                  const std::string& password) {
  profile_ = profile;
  if (!shell_client_) {
    shell_client_ = GetShellClient();
  }

  // Resolve the backup/restore script (image-installed copy, or embedded
  // fallback written to /tmp).
  const std::string script_path = GetJemaOSBackupScriptPath();
  if (script_path.empty()) {
    LOG(ERROR) << "Failed to ensure backup script exists";
    task_state_ = TaskState::kFailed;
    return;
  }

  std::string encoded_filepath = base::Base64Encode(backup_path_.value());
  task_state_ = TaskState::kRunning;
  const std::string key = GenerateKey(email, password);
  const std::string command =
      base::StringPrintf(kJemaOSBackupCommandFormat, script_path.c_str(),
                         email.c_str(), key.c_str(), encoded_filepath.c_str());
  shell_client_->AsyncExec(command,
                           base::BindOnce(&BackupTaskManager::OnTaskStarted,
                                          weak_ptr_factory_.GetWeakPtr()));
}

void BackupTaskManager::OnTaskStarted(std::optional<ShellState> state) {
  if (!state || state->code == -1) {
    LOG(ERROR) << "start backup task error, "
               << (state ? state->result : "state is null");
    task_state_ = TaskState::kFailed;
    return;
  }
  task_id_ = state->code;

  // for starting a new backup task,
  // force remove and add notification to make sure it will popup
  if (MessageCenter::Get()->FindVisibleNotificationById(
          kJemaOSBackupNotificationId)) {
    MessageCenter::Get()->RemoveNotification(kJemaOSBackupNotificationId,
                                             false);
  }
  DisplayNotification(CreateNotification(
      task_state_,
      l10n_util::GetStringUTF16(IDS_JEMAOS_BACKUP_NOTIFICATION_RUNNING_TITLE),
      u"", nullptr));
  GetShellClientTaskState();
  GetTaskOutputAndState();
}

void BackupTaskManager::ScheduleGetTaskOutputAndState() {
  base::SequencedTaskRunner::GetCurrentDefault()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&BackupTaskManager::GetTaskOutputAndState,
                     weak_ptr_factory_.GetWeakPtr()),
      base::Seconds(kJemaOSBackupTaskTrackIntervalSeconds));
}

void BackupTaskManager::GetTaskOutputAndState() {
  shell_client_->GetTaskOutput(
      task_id_, kJemaOSBackupTaskOutputLines,
      base::BindOnce(&BackupTaskManager::OnGetTaskOutputAndState,
                     weak_ptr_factory_.GetWeakPtr()));
}

void BackupTaskManager::OnGetTaskOutputAndState(
    std::optional<ShellState> state) {
  if (!state || state->code == -1) {
    LOG(ERROR) << "get backup task output and state error, "
               << (state ? state->result : "state is null");
    return;
  }
  // see jemaos/extensions/common/api/shell_client.json
  switch (state->code) {
    case 0:
      // ON_NONE
      task_state_ = TaskState::kFailed;
      OnBackupError();
      break;
    case 1:
      // ON_PROGRESS
      task_state_ = TaskState::kRunning;
      ScheduleGetTaskOutputAndState();
      break;
    case 2:
      // ON_CLOSED
      task_state_ = TaskState::kFinished;
      OnBackupFinished();
      break;
    case 3:
      // ON_ERROR
      task_state_ = TaskState::kFailed;
      OnBackupError();
      break;
    default:
      NOTREACHED_IN_MIGRATION();
      break;
  }
  if (!callback_.is_null() && task_state_ != TaskState::kRunning &&
      task_state_ != TaskState::kIdle) {
    std::move(callback_).Run(task_state_);
  }
}

void BackupTaskManager::GetShellClientTaskState() {
  shell_client_->GetTaskState(
      task_id_, base::BindOnce(&BackupTaskManager::OnGetShellClientTaskState,
                               weak_ptr_factory_.GetWeakPtr()));
}

void BackupTaskManager::OnGetShellClientTaskState(
    std::optional<ShellState> state) {
  if (!state || state->code == -1) {
    LOG(ERROR) << "get backup task final state error, "
               << (state ? state->result : "state is null");
    return;
  }
  std::optional<base::Value::Dict> json =
      base::JSONReader::ReadDict(state->result);
  if (!json) {
    LOG(ERROR) << "get backup task final state error, json is invalid";
    return;
  }
  if (json->FindInt("key") == task_id_) {
    const std::string* tmpFile = json->FindString("tmpFile");
    if (tmpFile) {
      tmp_log_path_ = base::FilePath(*tmpFile);
    }
    return;
  }
}

void BackupTaskManager::OnBackupFinished() {
  task_runner_->PostTaskAndReplyWithResult(
      FROM_HERE, base::BindOnce(&base::PathExists, backup_path_),
      base::BindOnce(&BackupTaskManager::OnCheckBackupFile,
                     weak_ptr_factory_.GetWeakPtr()));
}

void BackupTaskManager::OnCheckBackupFile(const bool ok) {
  ShowBackupFinishedNotification(backup_path_, ok);
}

void BackupTaskManager::OnBackupError() {
  task_runner_->PostTaskAndReplyWithResult(
      FROM_HERE, base::BindOnce(&base::PathExists, tmp_log_path_),
      base::BindOnce(&BackupTaskManager::OnCheckTmpLogFile,
                     weak_ptr_factory_.GetWeakPtr()));
}

void BackupTaskManager::OnCheckTmpLogFile(const bool ok) {
  if (ok) {
    ProcessTmpLogFileByShellClient(
        tmp_log_path_,
        base::BindOnce(&BackupTaskManager::ShowBackupErrorNotification,
                       weak_ptr_factory_.GetWeakPtr()));
  } else {
    VLOG(2) << "tmp log file not exist";
    ShowBackupErrorNotification(base::FilePath(), false);
  }
}

void BackupTaskManager::ProcessTmpLogFileByShellClient(
    const base::FilePath& tmp,
    ReadTmpLogCallback callback) {
  const std::string command = base::StringPrintf("cat %s", tmp.value().c_str());
  shell_client_->SyncExec(
      command,
      base::BindOnce(&BackupTaskManager::OnTmpLogFileRead,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
}

void BackupTaskManager::OnTmpLogFileRead(ReadTmpLogCallback callback,
                                         std::optional<ShellState> state) {
  if (!state || state->code == -1) {
    LOG(ERROR) << "read tmp log file error, "
               << (state ? state->result : "state is null");
    std::move(callback).Run(base::FilePath(), false);
    return;
  }
  const std::string name = backup_path_.value();
  base::FilePath log_path;
  if (base::EndsWith(name, ".tar.gz.gpg")) {
    log_path =
        base::FilePath(name.substr(0, name.size() - 11)).AddExtension("txt");
  } else {
    log_path = base::FilePath(name).ReplaceExtension("txt");
  }
  if (state->result.size() > 0) {
    task_runner_->PostTaskAndReplyWithResult(
        FROM_HERE, base::BindOnce(&WriteFile_, log_path, state->result),
        base::BindOnce(&BackupTaskManager::OnLogFileCopied,
                       weak_ptr_factory_.GetWeakPtr(), std::move(callback),
                       log_path));
  } else {
    std::move(callback).Run(base::FilePath(), false);
  }
}

void BackupTaskManager::OnLogFileCopied(ReadTmpLogCallback callback,
                                        const base::FilePath& log,
                                        const bool exists) {
  if (!exists) {
    VLOG(2) << "tmp log file copy failed";
  }
  std::move(callback).Run(log, exists);
}

void BackupTaskManager::ShowBackupErrorNotification(
    const base::FilePath& log_path,
    const bool exists) {
  scoped_refptr<message_center::NotificationDelegate> delegate;
  std::u16string title =
      l10n_util::GetStringUTF16(IDS_JEMAOS_BACKUP_NOTIFICATION_FAILED_TITLE);
  std::u16string message =
      l10n_util::GetStringUTF16(IDS_JEMAOS_BACKUP_NOTIFICATION_FAILED_MESSAGE);
  if (exists) {
    delegate =
        base::MakeRefCounted<message_center::HandleNotificationClickDelegate>(
            base::BindRepeating(
                [](Profile* profile, base::FilePath path) {
                  platform_util::ShowItemInFolder(profile, path);
                },
                profile_, log_path));
  } else {
    delegate = nullptr;
    message = u"";
  }
  DisplayNotification(
      CreateNotification(TaskState::kFailed, title, message, delegate));
}

void BackupTaskManager::ShowBackupFinishedNotification(
    const base::FilePath& backup_path,
    const bool exists) {
  scoped_refptr<message_center::NotificationDelegate> delegate;
  std::u16string title =
      l10n_util::GetStringUTF16(IDS_JEMAOS_BACKUP_NOTIFICATION_FINISHED_TITLE);
  std::u16string message = l10n_util::GetStringUTF16(
      IDS_JEMAOS_BACKUP_NOTIFICATION_FINISHED_MESSAGE);
  if (exists) {
    delegate =
        base::MakeRefCounted<message_center::HandleNotificationClickDelegate>(
            base::BindRepeating(
                [](Profile* profile, base::FilePath path) {
                  platform_util::ShowItemInFolder(profile, path);
                },
                profile_, backup_path));
  } else {
    message = u"";
    delegate = nullptr;
  }
  DisplayNotification(
      CreateNotification(TaskState::kFinished, title, message, delegate));
}

void BackupTaskManager::DisplayNotification(
    std::unique_ptr<message_center::Notification> notification) {
  if (MessageCenter::Get()->FindVisibleNotificationById(
          kJemaOSBackupNotificationId)) {
    MessageCenter::Get()->UpdateNotification(kJemaOSBackupNotificationId,
                                             std::move(notification));
  } else {
    MessageCenter::Get()->AddNotification(std::move(notification));
  }
}

}  // namespace ash::settings
