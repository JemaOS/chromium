// Copyright (c) 2021 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/ui/webui/settings/ash/jemaos_handler.h"

#include <algorithm>

#include "base/logging.h"
#include "base/notreached.h"
#include "base/values.h"
#include "base/files/file_util.h"
#include "base/task/thread_pool.h"
#include "ash/shell.h"
#include "chrome/browser/lifetime/application_lifetime.h"
#include "chrome/browser/profiles/profile.h"
#include "chromeos/ash/components/cryptohome/error_util.h"
#include "chromeos/ash/components/cryptohome/userdataauth_util.h"
#include "components/prefs/pref_service.h"
#include "chrome/browser/browser_process.h"
#include "components/user_manager/user_manager.h"
#include "ash/wm/tablet_mode/tablet_mode_controller.h"
#include "jemaos/prefs/jemaos_pref_names.h"
#include "chrome/browser/ash/profiles/profile_helper.h"
#include "ui/base/l10n/l10n_util.h"
// #include "chromeos/cryptohome/system_salt_getter.h"
// #include "chrome/browser/ash/settings/token_encryptor.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/chrome_select_file_policy.h"
#include "content/public/browser/browser_thread.h"
#include "chrome/browser/ash/file_manager/path_util.h"
#include "chrome/grit/generated_resources.h"
#include "ui/shell_dialogs/selected_file_info.h"
#include "chrome/browser/ash/file_manager/volume_manager.h"
#include "jemaos/switches/misc/misc_constants.h"
#include "jemaos/misc/jemaos_dev_mode.h"
#include "chromeos/ash/components/login/auth/auth_factor_editor.h"
#include "chromeos/ash/components/cryptohome/auth_factor_conversions.h"
#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/jemaos_shell_client.h"
#include "base/hash/sha1.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/json/json_reader.h"
#include <sys/stat.h>

namespace ash::settings {

namespace {

bool SuitableForBackupVolume(const file_manager::Volume* volume) {
  if (!volume) {
    return false;
  }
  return volume->type() == file_manager::VOLUME_TYPE_REMOVABLE_DISK_PARTITION
    && base::StartsWith(volume->mount_path().value(),
        "/media/removable", base::CompareCase::INSENSITIVE_ASCII)
    && !volume->hidden()
    && !volume->is_read_only_removable_device()
    && !volume->is_read_only()
    && volume->has_media();
}

const char kJemaOSArcMediaAutoScanIndicatorFile[] = "/home/chronos/user/.enable_arc_media_auto_scan";

bool ArcMediaAutoScanIndicatorFileExists() {
  return base::PathExists(base::FilePath(kJemaOSArcMediaAutoScanIndicatorFile));
}

bool DeleteArcMediaAutoScanIndicatorFile() {
  return base::DeleteFile(base::FilePath(kJemaOSArcMediaAutoScanIndicatorFile));
}

bool CreateArcMediaAutoScanIndicatorFile() {
  return base::WriteFile(base::FilePath(kJemaOSArcMediaAutoScanIndicatorFile), "");
}

// Embedded backup/restore script for restore operations
// SAFE VERSION: Only backs up and restores MyFiles folder contents
// Never touches browser profile, auth files, or system directories
const char kRestoreScriptPath[] = "/tmp/jemaos-backup-script.sh";
const char kRestoreScript[] = R"SCRIPT(#!/bin/bash
LOGFILE="/tmp/jemaos-backup.log"
echo "=== $(date) ===" >> "$LOGFILE"
echo "Script version: MyFiles-only v2 (safe)" >> "$LOGFILE"
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

# MyFiles is the main user files directory in ChromeOS
MYFILES_PATH="$PROFILE_PATH/MyFiles"

# Fallback to Downloads if MyFiles doesn't exist
if [ ! -d "$MYFILES_PATH" ]; then
  MYFILES_PATH="$PROFILE_PATH/Downloads"
fi

echo "Using files path: $MYFILES_PATH" >> "$LOGFILE"

if [ "$COMMAND" = "backup" ]; then
  [ ! -d "$MYFILES_PATH" ] && echo "Error: MyFiles folder not found" && exit 1
  mkdir -p "$(dirname "$TARGET")" 2>> "$LOGFILE"
  
  TEMP_DIR="/tmp/jemaos_backup_$$"
  mkdir -p "$TEMP_DIR"
  echo -e "User: $EMAIL\nDate: $(date)\nSource: MyFiles\nVersion: 2" > "$TEMP_DIR/backup_info.txt"
  
  # Only exclude cache files, no need for auth excludes since we only backup MyFiles
  EXCLUDES=(
    --exclude='.cache/*'
    --exclude='*/Cache/*'
    --exclude='*.bak'
    --exclude='.Trash*'
    --exclude='*.tmp'
    --exclude='*.temp'
  )
  
  echo "Backing up MyFiles folder only..." >> "$LOGFILE"
  
  if [ -n "$KEY" ]; then
    tar -czf - "${EXCLUDES[@]}" -C "$MYFILES_PATH" . -C "$TEMP_DIR" backup_info.txt 2>> "$LOGFILE" \
      | openssl enc -aes-256-cbc -salt -pbkdf2 -pass pass:"$KEY" > "$TARGET" 2>> "$LOGFILE"
  else
    tar -czf "$TARGET" "${EXCLUDES[@]}" -C "$MYFILES_PATH" . -C "$TEMP_DIR" backup_info.txt 2>> "$LOGFILE"
  fi
  rm -rf "$TEMP_DIR"
  
  if [ -f "$TARGET" ] && [ -s "$TARGET" ]; then
    SIZE=$(du -h "$TARGET" | cut -f1)
    echo "Backup completed: $SIZE" >> "$LOGFILE"
    echo "Backup completed: $TARGET ($SIZE)"
    exit 0
  fi
  echo "Backup failed" >> "$LOGFILE"
  exit 1

elif [ "$COMMAND" = "restore" ]; then
  [ ! -f "$TARGET" ] && echo "Error: File not found: $TARGET" && exit 1
  
  TEMP_DIR="/tmp/jemaos_restore_$$"
  mkdir -p "$TEMP_DIR"
  
  echo "Extracting backup to temp directory..." >> "$LOGFILE"
  
  if [ -n "$KEY" ]; then
    openssl enc -aes-256-cbc -d -salt -pbkdf2 -pass pass:"$KEY" -in "$TARGET" 2>> "$LOGFILE" \
      | tar -xzf - -C "$TEMP_DIR" 2>> "$LOGFILE" || { echo "Decrypt failed"; rm -rf "$TEMP_DIR"; exit 1; }
  else
    tar -xzf "$TARGET" -C "$TEMP_DIR" 2>> "$LOGFILE" || { echo "Extract failed"; rm -rf "$TEMP_DIR"; exit 1; }
  fi
  
  # Check if this is an old full-profile backup that contains MyFiles folder
  # If so, we should restore from MyFiles subfolder, not the root
  if [ -d "$TEMP_DIR/MyFiles" ]; then
    echo "Detected old full-profile backup with MyFiles folder inside" >> "$LOGFILE"
    echo "Will restore from MyFiles subfolder only" >> "$LOGFILE"
    
    # Move MyFiles content to a safe location
    MYFILES_CONTENT="/tmp/jemaos_myfiles_content_$$"
    mkdir -p "$MYFILES_CONTENT"
    cp -a "$TEMP_DIR/MyFiles"/. "$MYFILES_CONTENT"/ 2>> "$LOGFILE"
    
    # Clear temp dir and use only MyFiles content
    rm -rf "$TEMP_DIR"
    mkdir -p "$TEMP_DIR"
    cp -a "$MYFILES_CONTENT"/. "$TEMP_DIR"/ 2>> "$LOGFILE"
    rm -rf "$MYFILES_CONTENT"
    
    echo "Extracted MyFiles content for restore" >> "$LOGFILE"
  fi
  
  # COMPREHENSIVE CLEANUP: Remove ALL system/browser files
  echo "Safety check: removing ALL system/browser files from backup..." >> "$LOGFILE"
  
  # Remove ALL directories that are not typical user folders
  # Keep ONLY: Downloads, Documents, Pictures, Music, Videos, and user-created folders
  # Remove everything that looks like a system/browser folder
  
  # Known system directories (extensive list)
  SYSTEM_DIRS=(
    # Hidden directories
    ".pki" ".config" ".local" ".cache" ".shadow" ".dmrc" ".android"
    # Browser/Chrome directories
    "GCache" "Cache" "cache" "Code Cache" "code_cache" "CodeCache"
    "GPUCache" "gpu_cache" "GpuCache"
    "DawnGraphiteCache" "DawnWebGPUCache" "dawn_cache"
    "app_service" "app_service_storage" "AppServiceStorage"
    "Application Cache" "application_cache"
    "blob_storage" "BlobStorage"
    "databases" "Databases"
    "Extensions" "extensions" "Extension Scripts" "extension_scripts"
    "Extension State" "extension_state"
    "Extension Rules" "extension_rules"
    "Feature Engagement Tracker" "feature_engagement"
    "File System" "file_system"
    "GCM Store" "gcm_store"
    "IndexedDB" "indexeddb"
    "Local Extension Settings" "local_extension_settings"
    "Local Storage" "local_storage" "LocalStorage"
    "Login Data" "login_data"
    "Network" "network"
    "Network Action Predictor" "network_action_predictor"
    "optimization_guide" "OptimizationGuide"
    "Platform Notifications" "platform_notifications"
    "Safe Browsing" "safe_browsing"
    "Service Worker" "service_worker" "ServiceWorker"
    "Session Storage" "session_storage" "SessionStorage"
    "Sessions" "sessions"
    "Shortcuts" "shortcuts"
    "Site Characteristics Database" "site_characteristics"
    "Storage" "storage"
    "Sync Data" "sync_data" "SyncData"
    "TransportSecurity" "transport_security"
    "Trust Tokens" "trust_tokens"
    "Visited Links" "visited_links"
    "Web Data" "web_data" "WebData"
    "WebRTC Logs" "webrtc_logs"
    "component_updater" "ComponentUpdater"
    "data_reduction_proxy" "DataReductionProxy"
    "Default" "Profile" "Guest Profile" "System Profile"
    "cros-components" "cros_components"
    "log" "LOG" "logs"
    "shared_proto_db" "SharedProtoDb"
    "Site Engagement" "site_engagement"
    "Top Sites" "top_sites"
    "History" "Thumbnails" "Favicons"
    "Bookmarks" "Preferences" "Secure Preferences"
    "Cookies" "cookies"
    "Affiliation Database" "affiliation_database"
    "AutofillStrikeDatabase" "autofill"
    "BudgetDatabase" "budget_database"
    "Download Service" "download_service"
    "heavy_ad_intervention" "HeavyAdIntervention"
    "media_device_salts" "MediaDeviceSalts"
    "Pepper Data" "pepper_data"
    "previews_opt_out" "PreviewsOptOut"
    "QuotaManager" "quota_manager"
    "Reporting and NEL" "reporting_nel"
    "SmartScreenState" "smartscreen"
    "VideoDecodeStats" "video_decode_stats"
    "webdata"
    "ZxcvbnData" "zxcvbn_data"
    # ChromeOS specific system folders
    "autobrightness" "birch" "commerce_subscriptions" "commerce_subscription"
    "discounts_db" "launcher_ranking" "app_launch_automation"
    "user" "crash" "crosvm.sock" "shill" "session_manager" "flimflam"
    "segmentation_platform" "optimization_guide_model_store"
    "optimization_guide_prediction_model_downloads"
    "component_crl" "FileTypePolicies" "OriginTrials" "SSLErrorAssistant"
    "SafetyTips" "Subresource Filter" "TrustTokenKeyCommitments"
    "first_party_sets" "PrivacySandboxAttestations"
    "CertificateRevocation" "CRLSet" "MEIPreload" "OnDeviceHeadModel"
    "pnacl" "recovery_component" "widevine"
    "MyFiles"
    # Common temp/system patterns
    "tmp" "temp" "README" "backup_info.txt"
  )
  
  # Remove each system directory
  for dir in "${SYSTEM_DIRS[@]}"; do
    if [ -e "$TEMP_DIR/$dir" ]; then
      echo "Removing system dir: $dir" >> "$LOGFILE"
      rm -rf "$TEMP_DIR/$dir" 2>/dev/null
    fi
  done
  
  # Remove ALL hidden directories and files
  find "$TEMP_DIR" -maxdepth 1 -name ".*" -exec rm -rf {} + 2>/dev/null
  
  # Remove specific file patterns
  find "$TEMP_DIR" -name "*.keyring" -delete 2>/dev/null
  find "$TEMP_DIR" -name "Cookies*" -delete 2>/dev/null
  find "$TEMP_DIR" -name "Login Data*" -delete 2>/dev/null
  find "$TEMP_DIR" -name "Web Data*" -delete 2>/dev/null
  find "$TEMP_DIR" -name "Preferences" -delete 2>/dev/null
  find "$TEMP_DIR" -name "Secure Preferences" -delete 2>/dev/null
  find "$TEMP_DIR" -name "*.log" -delete 2>/dev/null
  find "$TEMP_DIR" -name "*.ldb" -delete 2>/dev/null
  find "$TEMP_DIR" -name "*.leveldb" -delete 2>/dev/null
  find "$TEMP_DIR" -name "LOCK" -delete 2>/dev/null
  find "$TEMP_DIR" -name "MANIFEST*" -delete 2>/dev/null
  find "$TEMP_DIR" -name "CURRENT" -delete 2>/dev/null
  find "$TEMP_DIR" -name "LOG*" -type f -delete 2>/dev/null
  find "$TEMP_DIR" -name "*.sqlite*" -delete 2>/dev/null
  find "$TEMP_DIR" -name "*.db" -delete 2>/dev/null
  find "$TEMP_DIR" -name "*.db-*" -delete 2>/dev/null
  find "$TEMP_DIR" -name "Singleton*" -delete 2>/dev/null
  find "$TEMP_DIR" -name "backup_info.txt" -delete 2>/dev/null
  find "$TEMP_DIR" -name "*.crx" -delete 2>/dev/null
  find "$TEMP_DIR" -name "*.pem" -delete 2>/dev/null
  
  # Ensure MyFiles directory exists
  mkdir -p "$MYFILES_PATH" 2>> "$LOGFILE"
  
  echo "Restoring ONLY user files to MyFiles folder: $MYFILES_PATH" >> "$LOGFILE"
  echo "Remaining items after cleanup:" >> "$LOGFILE"
  ls -la "$TEMP_DIR" >> "$LOGFILE" 2>&1
  
  # Count files to restore
  FILE_COUNT=$(find "$TEMP_DIR" -type f 2>/dev/null | wc -l)
  DIR_COUNT=$(find "$TEMP_DIR" -mindepth 1 -type d 2>/dev/null | wc -l)
  echo "Files to restore: $FILE_COUNT, Directories: $DIR_COUNT" >> "$LOGFILE"
  
  # Only restore if there's something to restore
  if [ "$FILE_COUNT" -eq 0 ] && [ "$DIR_COUNT" -eq 0 ]; then
    echo "No user files found in backup after cleanup" >> "$LOGFILE"
    rm -rf "$TEMP_DIR"
    echo "Restore completed but no user files were found in backup."
    exit 0
  fi
  
  # Copy remaining files (should only be user files) to MyFiles
  for item in "$TEMP_DIR"/*; do
    if [ -e "$item" ]; then
      basename=$(basename "$item")
      echo "Restoring to MyFiles: $basename" >> "$LOGFILE"
      cp -a "$item" "$MYFILES_PATH/" 2>> "$LOGFILE"
    fi
  done
  
  # Fix ownership only for MyFiles directory (not profile root!)
  if id chronos &>/dev/null; then
    echo "Fixing ownership for MyFiles only..." >> "$LOGFILE"
    chown -R chronos:chronos "$MYFILES_PATH" 2>> "$LOGFILE"
  fi
  
  rm -rf "$TEMP_DIR"
  
  RESTORED_COUNT=$(find "$MYFILES_PATH" -type f 2>/dev/null | wc -l)
  echo "Restore completed! Files in MyFiles: $RESTORED_COUNT" >> "$LOGFILE"
  echo "Restore completed! Your files are in MyFiles folder."
  exit 0
else
  echo "Usage: backup|restore --email EMAIL --key KEY --target PATH"
  exit 1
fi
)SCRIPT";

bool EnsureRestoreScriptExists() {
  base::FilePath script_path(kRestoreScriptPath);
  // Always write the script to ensure latest version is used
  // This overwrites any cached old version
  if (!base::WriteFile(script_path, kRestoreScript)) {
    LOG(ERROR) << "Failed to create restore script";
    return false;
  }
  chmod(kRestoreScriptPath, 0755);
  return true;
}

}  // namespace

// JemaOsHandler::JemaOsHandler(Profile* profile, PrefService* prefs) :
//   profile_(profile), prefs_(prefs) {}
JemaOsHandler::JemaOsHandler(Profile* profile, PrefService* prefs) :
  profile_(profile), prefs_(prefs) {
  DCHECK(ash::Shell::Get());
  ash::Shell::Get()->tablet_mode_controller()->AddObserver(this);
  // TODO(fangzhou) use real system_salt_ and encryptor
  // SystemSaltGetter::Get()->GetSystemSalt(base::BindOnce(
  //     &JemaOsHandler::OnSystemSaltObtained, weak_ptr_factory_.GetWeakPtr()));
  OnSystemSaltObtained("JEMAOS");
}

JemaOsHandler::~JemaOsHandler() {
  if (ash::Shell::Get()->tablet_mode_controller())
    ash::Shell::Get()->tablet_mode_controller()->RemoveObserver(this);
  if (select_file_dialog_.get())
    select_file_dialog_->ListenerDestroyed();
}

void JemaOsHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "getIsOfflineAutoSigninEnabled",
      base::BindRepeating(&JemaOsHandler::HandleGetIsOfflineAutoSigninEnabled,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
               "saveOfflineLoginPassword",
                base::BindRepeating(&JemaOsHandler::HandleSaveOfflineLoginPassword,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
               "cleanOfflineLoginPassword",
                base::BindRepeating(&JemaOsHandler::HandleCleanOfflineLoginPassword,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "fetchConnectApiUserId",
      base::BindRepeating(&JemaOsHandler::HandleFetchConnectApiUserId,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getConnectApiUserId",
      base::BindRepeating(&JemaOsHandler::HandleGetConnectApiUserId,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getBackupAnalysis",
      base::BindRepeating(&JemaOsHandler::HandleGetBackupAnalysis,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getShowRotateScreenButton",
      base::BindRepeating(&JemaOsHandler::HandleGetShowRotateScreenButton,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "setShowRotateScreenButton",
      base::BindRepeating(&JemaOsHandler::HandleSetShowRotateScreenButton,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getIsInTabletPhysicalState",
      base::BindRepeating(&JemaOsHandler::HandleGetIsInTabletPhysicalState,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getShowSwitchTabletLaptopButton",
      base::BindRepeating(&JemaOsHandler::HandleGetShowSwitchTabletLaptopButton,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "setShowSwitchTabletLaptopButton",
      base::BindRepeating(&JemaOsHandler::HandleSetShowSwitchTabletLaptopButton,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getIsForceTpmFallback",
      base::BindRepeating(&JemaOsHandler::HandleGetIsForceTpmFallback,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "setForceTpmFallback",
      base::BindRepeating(&JemaOsHandler::HandleSetForceTpmFallback,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getRebootRequiredForWidevine",
      base::BindRepeating(&JemaOsHandler::HandleGetRebootRequiredForWidevine,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "toggleRebootRequiredForWidevine",
      base::BindRepeating(
          &JemaOsHandler::HandleToggleRebootRequiredForWidevine,
          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getRebootRequiredForWidevine",
      base::BindRepeating(&JemaOsHandler::HandleGetRebootRequiredForWidevine,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "toggleRebootRequiredForWidevine",
      base::BindRepeating(&JemaOsHandler::HandleToggleRebootRequiredForWidevine,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "jemaosBackupSupported",
      base::BindRepeating(&JemaOsHandler::HandleJemaOSBackupSupported,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "jemaosBackupSelectFile",
      base::BindRepeating(&JemaOsHandler::HandleJemaOSBackupSelectFile,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "jemaosBackupStarted",
      base::BindRepeating(&JemaOsHandler::HandleJemaOSBackupStarted,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getJemaosBackupState",
      base::BindRepeating(&JemaOsHandler::HandleGetJemaOSBackupState,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "jemaosRestoreSelectFile",
      base::BindRepeating(&JemaOsHandler::HandleJemaOSRestoreSelectFile,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "jemaosRestoreStarted",
      base::BindRepeating(&JemaOsHandler::HandleJemaOSRestoreStarted,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "jemaosCloudBackupStarted",
      base::BindRepeating(&JemaOsHandler::HandleJemaOSCloudBackupStarted,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "jemaosCloudRestoreStarted",
      base::BindRepeating(&JemaOsHandler::HandleJemaOSCloudRestoreStarted,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "jemaosCloudListBackupFiles",
      base::BindRepeating(&JemaOsHandler::HandleJemaOSCloudListBackupFiles,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getArcMediaAutoScanState",
      base::BindRepeating(&JemaOsHandler::HandleGetArcMediaAutoScanState,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "setArcMediaAutoScanState",
      base::BindRepeating(&JemaOsHandler::HandleSetArcMediaAutoScanState,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "setArcMediaAutoScanStateForCurrentSession",
      base::BindRepeating(&JemaOsHandler::HandleSetArcMediaAutoScanStateForCurrentSession,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "setDevMode",
      base::BindRepeating(&JemaOsHandler::HandleSetDevMode,
                      base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getDevModeSwitchSupported",
      base::BindRepeating(&JemaOsHandler::HandleGetDevModeSwitchSupported,
                      base::Unretained(this)));
}

void JemaOsHandler::OnJavascriptAllowed() {
  pref_change_registrar_.Init(prefs_);

  local_state_pref_change_registrar_.Init(g_browser_process->local_state());
  local_state_pref_change_registrar_.Add(
      jemaos::prefs::kShowRotateScreenButton,
      base::BindRepeating(
          &JemaOsHandler::OnShowRotateScreenButtonChanged,
          base::Unretained(this)));
  local_state_pref_change_registrar_.Add(
      jemaos::prefs::kShowSwitchTabletLaptopButton,
      base::BindRepeating(
          &JemaOsHandler::OnShowSwitchTabletLaptopButtonChanged,
          base::Unretained(this)));
  local_state_pref_change_registrar_.Add(
      jemaos::prefs::kForceTpmFallback,
      base::BindRepeating(
          &JemaOsHandler::OnForceTpmFallbackChanged,
          base::Unretained(this)));
}

void JemaOsHandler::OnJavascriptDisallowed() {
  pref_change_registrar_.RemoveAll();
  local_state_pref_change_registrar_.RemoveAll();
}

void JemaOsHandler::OnShowRotateScreenButtonChanged() {
  PrefService* prefs = g_browser_process->local_state();
  bool showRotate = prefs->GetBoolean(jemaos::prefs::kShowRotateScreenButton);
  FireWebUIListener("show-rotate-screen-button-changed",
                    base::Value(showRotate));
}

void JemaOsHandler::OnSystemSaltObtained(const std::string& system_salt) {
  system_salt_ = system_salt;
  if (IsJavascriptAllowed()) {
    FireWebUIListener("offline-auto-signin-system-salt-obtained");
  }
}

void JemaOsHandler::ListAuthFactors(const AccountId& account_id, const std::string& callback_id) {
  auto client = UserDataAuthClient::Get();
  user_data_auth::ListAuthFactorsRequest request;
  *request.mutable_account_id() =
      cryptohome::CreateAccountIdentifierFromAccountId(account_id);
  client->ListAuthFactors(
      request, base::BindOnce(&JemaOsHandler::OnListAuthFactors,
                              weak_ptr_factory_.GetWeakPtr(),
                              callback_id));
}

void JemaOsHandler::OnListAuthFactors(const std::string& callback_id, std::optional<user_data_auth::ListAuthFactorsReply> reply) {
  auto error = user_data_auth::ReplyToCryptohomeError(reply);
  if (cryptohome::HasError(error)) {
    LOG(ERROR) << "Could not list auth factors " << error;
    return;
  }
  CHECK(reply.has_value());
  auth_factor_has_password_ = false;
  for (const auto& factor_with_status_proto :
       reply->configured_auth_factors_with_status()) {
    if (factor_with_status_proto.auth_factor().type() == user_data_auth::AUTH_FACTOR_TYPE_PASSWORD) {
      auth_factor_has_password_ = true;
      break;
    }
  }

  const user_manager::User* user =
      ProfileHelper::Get()->GetUserByProfile(profile_);
  PrefService* prefs = g_browser_process->local_state();
  const std::string& password =
    prefs->GetString(jemaos::prefs::kOfflineAutoSigninPassword);
  const std::string& account_id_key =
    prefs->GetString(jemaos::prefs::kOfflineAutoSigninAccountIdKey);
  base::Value::Dict response;
  response.Set("enabled", !account_id_key.empty() && !password.empty());
  if (!user->GetAccountId().HasAccountIdKey() || !user->IsFlintAccountUser()) {
    response.Set("is_current_user", false);
  }
  if (user->GetAccountId().GetAccountIdKey() == account_id_key) {
    response.Set("is_current_user", true);
  } else {
    response.Set("is_current_user", false);
  }
  response.Set("system_salt_obtained", !system_salt_.empty());
  response.Set("auth_factor_has_password", auth_factor_has_password_);
  ResolveJavascriptCallback(callback_id, response);
}

void JemaOsHandler::HandleGetIsOfflineAutoSigninEnabled(
    const base::Value::List& args) {
  AllowJavascript();
  CHECK(args.size());
  std::string callback_id = args[0].GetString();
  const user_manager::User* user =
      ProfileHelper::Get()->GetUserByProfile(profile_);
  ListAuthFactors(user->GetAccountId(), callback_id);
}

void JemaOsHandler::HandleSaveOfflineLoginPassword(
    const base::Value::List& args) {
  CHECK_EQ(2u, args.size());
  const base::Value& callback_id = args[0];
  if (!g_browser_process || system_salt_.empty()) {
    ResolveJavascriptCallback(callback_id, base::Value(false));
    return;
  }
  std::string password = args[1].GetString();
  if (password.empty()) {
    ResolveJavascriptCallback(callback_id, base::Value(false));
  }
  user_manager::User* user =
    ash::ProfileHelper::Get()->GetUserByProfile(profile_);
  const AccountId account_id = user->GetAccountId();
  if (!account_id.HasAccountIdKey()) {
    ResolveJavascriptCallback(callback_id, base::Value(false));
    return;
  }
  PrefService* prefs = g_browser_process->local_state();
  // ash::CryptohomeTokenEncryptor encryptor(system_salt_);
  // prefs->SetString(jemaos::prefs::kOfflineAutoSigninPassword,
  //    encryptor.EncryptWithSystemSalt(password));
  prefs->SetString(jemaos::prefs::kOfflineAutoSigninPassword, password);
  prefs->SetString(jemaos::prefs::kOfflineAutoSigninPasswordFormat,
      "plaintext");
  prefs->SetString(jemaos::prefs::kOfflineAutoSigninAccountIdKey,
      account_id.GetAccountIdKey());
  
  // Fetch user_id from Connect API after login
  if (account_id.GetUserEmail().length() > 0) {
    std::string email = account_id.GetUserEmail();
    LOG(INFO) << "Triggering Connect API user_id fetch for email: " << email;
    HandleFetchConnectApiUserId(base::Value::List().Append(email));
  } else {
    LOG(WARNING) << "No email available for Connect API user_id fetch";
  }
  
  ResolveJavascriptCallback(callback_id, base::Value(true));
}

void JemaOsHandler::HandleCleanOfflineLoginPassword(
    const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  const base::Value& callback_id = args[0];
  if (!g_browser_process) {
    ResolveJavascriptCallback(callback_id, base::Value(false));
    return;
  }
  PrefService* prefs = g_browser_process->local_state();
  prefs->SetString(jemaos::prefs::kOfflineAutoSigninPassword, std::string());
  prefs->SetString(jemaos::prefs::kOfflineAutoSigninAccountIdKey,
      std::string());
  ResolveJavascriptCallback(callback_id, base::Value(true));
}

void JemaOsHandler::HandleFetchConnectApiUserId(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  std::string email = args[0].GetString();
  
  if (email.empty()) {
    LOG(ERROR) << "Empty email provided for Connect API user ID fetch";
    return;
  }
  
  LOG(INFO) << "Fetching Connect API user ID for email: " << email;
  
  // API endpoint
  const char kConnectApiBaseUrl[] = "https://test-connect-api.jematech.fr";
  std::string api_url = std::string(kConnectApiBaseUrl) + "/v1/connect/user/by-email";
  
  // Prepare JSON body
  std::string json_body = base::StringPrintf(
      "{\"email\": \"%s\"}", email.c_str());
  
  LOG(INFO) << "=== Connect API Call Debug ===";
  LOG(INFO) << "Email: " << email;
  LOG(INFO) << "API URL: " << api_url;
  LOG(INFO) << "JSON body: " << json_body;
  
  // Write a shell script to execute curl - same pattern as cloud backup API
  std::string script_file = "/tmp/jemaos_connect_api.sh";
  std::string script_content = 
      "#!/bin/bash\n"
      "exec 2>/dev/null\n"  // Suppress stderr warnings about noexec mount
      "curl -s -L -X POST \"" + api_url + "\" \\\n"
      "  -H 'Content-Type: application/json' \\\n"
      "  -d '" + json_body + "'\n";
  
  LOG(INFO) << "Writing script to: " << script_file;
  LOG(INFO) << "Script content:\n" << script_content;
  
  base::WriteFile(base::FilePath(script_file), script_content);
  chmod(script_file.c_str(), 0755);
  
  // Redirect stderr when running the script to suppress noexec warnings
  std::string command = "/bin/bash " + script_file + " 2>/dev/null";
  LOG(INFO) << "Command: " << command;
  LOG(INFO) << "==============================";
  
  auto* shell_client = JemaOSShellClient::Get();
  if (!shell_client) {
    LOG(ERROR) << "Shell client not available for Connect API call";
    return;
  }
  
  shell_client->SyncExec(command,
      base::BindOnce(&JemaOsHandler::OnConnectApiUserIdReceived,
                     weak_ptr_factory_.GetWeakPtr(), email));
}

void JemaOsHandler::OnConnectApiUserIdReceived(const std::string& email, 
                                               std::optional<ShellState> state) {
  LOG(INFO) << "=== OnConnectApiUserIdReceived called ===";
  LOG(INFO) << "Email: " << email;
  
  if (!state) {
    LOG(ERROR) << "CRITICAL: No state returned from Connect API call - command may not have been executed";
    LOG(ERROR) << "This usually means the shell client failed to execute the command";
    return;
  }
  
  LOG(INFO) << "Exit code: " << state->code;
  LOG(INFO) << "Raw output length: " << state->result.length();
  LOG(INFO) << "Raw output (first 500 chars): " << state->result.substr(0, std::min<size_t>(500, state->result.length()));
  
  if (state->code != 0) {
    LOG(ERROR) << "Command failed with exit code: " << state->code;
    LOG(ERROR) << "Full output: " << state->result;
    LOG(ERROR) << "Possible causes: network issue, curl not found, or command syntax error";
    return;
  }
  
  LOG(INFO) << "Connect API raw response: [" << state->result << "]";
  LOG(INFO) << "Response length: " << state->result.length();
  
  // Check if response is empty
  if (state->result.empty()) {
    LOG(ERROR) << "Empty response from Connect API";
    return;
  }
  
  // Filter out warning lines from the response (noexec mount warnings)
  std::string cleaned_result = state->result;
  size_t json_start = cleaned_result.find("{");
  if (json_start != std::string::npos) {
    // Extract only the JSON part (starting from first '{')
    cleaned_result = cleaned_result.substr(json_start);
    LOG(INFO) << "Cleaned JSON response: [" << cleaned_result << "]";
  } else {
    LOG(ERROR) << "No JSON found in response";
    return;
  }
  
  // Parse JSON response using base::JSONReader
  std::optional<base::Value> json_value = base::JSONReader::Read(cleaned_result);
  if (!json_value || !json_value->is_dict()) {
    LOG(ERROR) << "Invalid JSON response from Connect API. First 200 chars: " 
               << cleaned_result.substr(0, std::min<size_t>(200, cleaned_result.length()));
    return;
  }
  
  const base::Value::Dict& root_dict = json_value->GetDict();
  const base::Value* success = root_dict.Find("success");
  if (!success || !success->GetBool()) {
    LOG(ERROR) << "API returned success=false";
    return;
  }
  
  const base::Value::Dict* data = root_dict.FindDict("data");
  if (!data) {
    LOG(ERROR) << "No data field in API response";
    return;
  }
  
  const std::string* user_id = data->FindString("user_id");
  if (!user_id || user_id->empty()) {
    LOG(ERROR) << "No user_id in API response data";
    return;
  }
  
  // Extract status field
  const std::string* status = data->FindString("status");
  std::string status_value = status ? *status : "unknown";
  
  LOG(INFO) << "Storing user_id: " << *user_id << " for email: " << email << " with status: " << status_value;
  
  // Store user_id and status in local state preferences
  PrefService* prefs = g_browser_process->local_state();
  if (prefs) {
    prefs->SetString(jemaos::prefs::kConnectApiUserId, *user_id);
    prefs->SetString(jemaos::prefs::kConnectApiUserStatus, status_value);
  }
  
  // Notify UI that user_id was fetched
  base::Value::Dict response_dict;
  response_dict.Set("user_id", *user_id);
  response_dict.Set("email", email);
  response_dict.Set("status", status_value);
  
  if (IsJavascriptAllowed()) {
    FireWebUIListener("connect-api-user-id-fetched", response_dict);
  }
}

void JemaOsHandler::HandleGetConnectApiUserId(const base::Value::List& args) {
  AllowJavascript();
  
  if (args.size() != 1u) {
    LOG(ERROR) << "Invalid arguments for getConnectApiUserId";
    return;
  }
  
  if (!args[0].is_string()) {
    LOG(ERROR) << "Invalid callback_id type for getConnectApiUserId";
    return;
  }
  
  std::string callback_id = args[0].GetString();
  
  if (callback_id.empty()) {
    LOG(ERROR) << "Empty callback_id for getConnectApiUserId";
    return;
  }
  
  if (!g_browser_process) {
    LOG(ERROR) << "Browser process not available for getConnectApiUserId";
    if (IsJavascriptAllowed()) {
      base::Value::Dict error_response;
      error_response.Set("user_id", "");
      error_response.Set("has_user_id", false);
      ResolveJavascriptCallback(callback_id, error_response);
    }
    return;
  }
  
  PrefService* prefs = g_browser_process->local_state();
  if (!prefs) {
    LOG(ERROR) << "Prefs not available for getConnectApiUserId";
    if (IsJavascriptAllowed()) {
      base::Value::Dict error_response;
      error_response.Set("user_id", "");
      error_response.Set("has_user_id", false);
      ResolveJavascriptCallback(callback_id, error_response);
    }
    return;
  }
  
  std::string user_id = prefs->GetString(jemaos::prefs::kConnectApiUserId);
  std::string status = prefs->GetString(jemaos::prefs::kConnectApiUserStatus);
  
  // Get current user email
  std::string email;
  const user_manager::User* user = 
      ash::ProfileHelper::Get()->GetUserByProfile(profile_);
  if (user && user->GetAccountId().GetUserEmail().length() > 0) {
    email = user->GetAccountId().GetUserEmail();
  }
  
  // If user_id is not available, try to fetch it from API using current user's email
  if (user_id.empty() && !email.empty()) {
    LOG(INFO) << "User ID not found in prefs, fetching from API for email: " << email;
    // Trigger API call in background - don't wait for result
    HandleFetchConnectApiUserId(base::Value::List().Append(email));
    // Return empty for now, will be updated via WebUI event when API responds
  }
  
  if (!IsJavascriptAllowed()) {
    LOG(WARNING) << "Javascript not allowed, cannot resolve callback for getConnectApiUserId";
    return;
  }
  
  base::Value::Dict response;
  response.Set("user_id", user_id);
  response.Set("email", email);
  response.Set("status", status);
  response.Set("has_user_id", !user_id.empty());
  
  LOG(INFO) << "Returning user info - user_id: " << user_id << ", status: " << status << ", email: " << email;
  
  ResolveJavascriptCallback(callback_id, response);
}

void JemaOsHandler::HandleGetBackupAnalysis(const base::Value::List& args) {
  AllowJavascript();
  
  if (args.size() != 1u) {
    LOG(ERROR) << "Invalid arguments for getBackupAnalysis";
    return;
  }
  
  if (!args[0].is_string()) {
    LOG(ERROR) << "Invalid callback_id type for getBackupAnalysis";
    return;
  }
  
  std::string callback_id = args[0].GetString();
  
  if (callback_id.empty()) {
    LOG(ERROR) << "Empty callback_id for getBackupAnalysis";
    return;
  }
  
  // Get user_id from preferences
  if (!g_browser_process) {
    LOG(ERROR) << "Browser process not available";
    if (IsJavascriptAllowed()) {
      base::Value::Dict error_response;
      error_response.Set("success", false);
      error_response.Set("error", "Browser process not available");
      ResolveJavascriptCallback(callback_id, error_response);
    }
    return;
  }
  
  PrefService* prefs = g_browser_process->local_state();
  if (!prefs) {
    LOG(ERROR) << "Local state prefs not available";
    if (IsJavascriptAllowed()) {
      base::Value::Dict error_response;
      error_response.Set("success", false);
      error_response.Set("error", "Preferences not available");
      ResolveJavascriptCallback(callback_id, error_response);
    }
    return;
  }
  
  std::string user_id = prefs->GetString(jemaos::prefs::kConnectApiUserId);
  
  if (user_id.empty()) {
    LOG(ERROR) << "No user_id available for backup analysis";
    if (IsJavascriptAllowed()) {
      base::Value::Dict error_response;
      error_response.Set("success", false);
      error_response.Set("error", "No user_id available");
      ResolveJavascriptCallback(callback_id, error_response);
    }
    return;
  }
  
  LOG(INFO) << "Fetching backup analysis for user_id: " << user_id;
  
  // API endpoint
  const char kConnectApiBaseUrl[] = "https://test-connect-api.jematech.fr";
  std::string api_url = base::StringPrintf("%s/v1/connect/backup/analysis/%s",
                                           kConnectApiBaseUrl, user_id.c_str());
  
  // Create curl script
  std::string script_file = "/tmp/jemaos_backup_analysis.sh";
  std::string script_content = 
      "#!/bin/bash\n"
      "curl -s -L \"" + api_url + "\"\n";
  
  base::WriteFile(base::FilePath(script_file), script_content);
  chmod(script_file.c_str(), 0755);
  
  // Suppress stderr to avoid noexec warnings
  std::string command = "/bin/bash " + script_file + " 2>/dev/null";
  
  auto* shell_client = JemaOSShellClient::Get();
  if (!shell_client) {
    LOG(ERROR) << "Shell client not available for backup analysis API call";
    if (IsJavascriptAllowed()) {
      base::Value::Dict error_response;
      error_response.Set("success", false);
      error_response.Set("error", "System service not available");
      ResolveJavascriptCallback(callback_id, error_response);
    }
    return;
  }
  
  shell_client->SyncExec(command,
      base::BindOnce(&JemaOsHandler::OnBackupAnalysisReceived,
                     weak_ptr_factory_.GetWeakPtr(), callback_id));
}

void JemaOsHandler::OnBackupAnalysisReceived(const std::string& callback_id,
                                             std::optional<ShellState> state) {
  // Safety check - ensure JavaScript is allowed before resolving callbacks
  if (!IsJavascriptAllowed()) {
    LOG(ERROR) << "JavaScript not allowed, skipping backup analysis callback";
    return;
  }
  
  if (!state || state->code != 0) {
    LOG(ERROR) << "Failed to fetch backup analysis. Exit code: " 
               << (state ? state->code : -1);
    if (!callback_id.empty() && IsJavascriptAllowed()) {
      base::Value::Dict error_response;
      error_response.Set("success", false);
      error_response.Set("error", "API call failed");
      ResolveJavascriptCallback(callback_id, error_response);
    }
    return;
  }
  
  LOG(INFO) << "Backup analysis API raw response: " << state->result;
  
  // Filter out warning lines from the response (noexec mount warnings)
  std::string cleaned_result = state->result;
  size_t json_start = cleaned_result.find("{");
  if (json_start != std::string::npos) {
    cleaned_result = cleaned_result.substr(json_start);
    LOG(INFO) << "Cleaned backup analysis JSON: " << cleaned_result;
  } else {
    LOG(ERROR) << "No JSON found in backup analysis response";
    if (!callback_id.empty() && IsJavascriptAllowed()) {
      base::Value::Dict error_response;
      error_response.Set("success", false);
      error_response.Set("error", "No JSON in response");
      ResolveJavascriptCallback(callback_id, error_response);
    }
    return;
  }
  
  // Parse JSON response using base::JSONReader
  std::optional<base::Value> json_value = base::JSONReader::Read(cleaned_result);
  if (!json_value || !json_value->is_dict()) {
    LOG(ERROR) << "Invalid JSON response from backup analysis API";
    if (!callback_id.empty() && IsJavascriptAllowed()) {
      base::Value::Dict error_response;
      error_response.Set("success", false);
      error_response.Set("error", "Invalid JSON response");
      ResolveJavascriptCallback(callback_id, error_response);
    }
    return;
  }
  
  const base::Value::Dict& root_dict = json_value->GetDict();
  const base::Value* success = root_dict.Find("success");
  if (!success || !success->GetBool()) {
    LOG(ERROR) << "API returned success=false";
    if (!callback_id.empty() && IsJavascriptAllowed()) {
      base::Value::Dict error_response;
      error_response.Set("success", false);
      const std::string* message = root_dict.FindString("message");
      error_response.Set("error", message ? *message : "API call failed");
      ResolveJavascriptCallback(callback_id, error_response);
    }
    return;
  }
  
  // Extract data section
  const base::Value::Dict* data = root_dict.FindDict("data");
  if (!data) {
    LOG(ERROR) << "No data field in backup analysis API response";
    if (!callback_id.empty() && IsJavascriptAllowed()) {
      base::Value::Dict error_response;
      error_response.Set("success", false);
      error_response.Set("error", "No data in response");
      ResolveJavascriptCallback(callback_id, error_response);
    }
    return;
  }
  
  // Build response with parsed data
  base::Value::Dict response;
  response.Set("success", true);
  
  // Extract storage info
  const base::Value::Dict* storage = data->FindDict("storage");
  if (storage) {
    base::Value::Dict storage_dict;
    const base::Value* limit = storage->Find("limit");
    const base::Value* used = storage->Find("used");
    if (limit) {
      storage_dict.Set("limit", limit->GetIfInt().value_or(0));
    }
    if (used) {
      storage_dict.Set("used", used->GetIfInt().value_or(0));
    }
    response.Set("storage", std::move(storage_dict));
  }
  
  // Extract hardware devices
  const base::Value::List* hardware_devices = data->FindList("hardware_devices");
  if (hardware_devices) {
    base::Value::List devices_list;
    for (const auto& device_value : *hardware_devices) {
      if (device_value.is_dict()) {
        const base::Value::Dict& device = device_value.GetDict();
        base::Value::Dict device_dict;
        const std::string* hardware_id = device.FindString("hardware_id");
        const std::string* status = device.FindString("status");
        const std::string* used_storage = device.FindString("used_storage");
        if (hardware_id) {
          device_dict.Set("hardware_id", *hardware_id);
        }
        if (status) {
          device_dict.Set("status", *status);
        }
        if (used_storage) {
          device_dict.Set("used_storage", *used_storage);
        }
        devices_list.Append(std::move(device_dict));
      }
    }
    response.Set("hardware_devices", std::move(devices_list));
  }
  
  // Extract last_updated
  const std::string* last_updated = data->FindString("last_updated");
  if (last_updated) {
    response.Set("last_updated", *last_updated);
  }
  
  // Safety check before resolving callback
  if (!callback_id.empty() && IsJavascriptAllowed()) {
    ResolveJavascriptCallback(callback_id, response);
  } else {
    LOG(WARNING) << "Cannot resolve callback - JavaScript not allowed or empty callback_id";
  }
}

void JemaOsHandler::HandleSetShowRotateScreenButton(
    const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool showRotate = args[0].GetBool();
  PrefService* prefs = g_browser_process->local_state();
  prefs->SetBoolean(jemaos::prefs::kShowRotateScreenButton, showRotate);
}

void JemaOsHandler::HandleGetShowRotateScreenButton(
    const base::Value::List& args) {
  AllowJavascript();

  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  PrefService* prefs = g_browser_process->local_state();
  bool showRotate = prefs->GetBoolean(jemaos::prefs::kShowRotateScreenButton);
  ResolveJavascriptCallback(base::Value(callback_id),
                            base::Value(showRotate));
}

void JemaOsHandler::OnTabletPhysicalStateChanged() {
  if (!IsJavascriptAllowed()) {
    return;
  }
  bool is_in_tablet_physical_state =
    ash::Shell::Get()->tablet_mode_controller()->is_in_tablet_physical_state();
  FireWebUIListener("is-in-tablet-physical-state-changed",
                    base::Value(is_in_tablet_physical_state));
}

void JemaOsHandler::HandleGetIsInTabletPhysicalState(
    const base::Value::List& args) {
  AllowJavascript();

  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  bool is_in_tablet_physical_state =
    ash::Shell::Get()->tablet_mode_controller()->is_in_tablet_physical_state();
  ResolveJavascriptCallback(base::Value(callback_id),
                            base::Value(is_in_tablet_physical_state));
}

void JemaOsHandler::OnShowSwitchTabletLaptopButtonChanged() {
  PrefService* prefs = g_browser_process->local_state();
  bool showButton = prefs->GetBoolean(jemaos::prefs::kShowSwitchTabletLaptopButton);
  FireWebUIListener("show-switch-tablet-laptop-button-changed",
                    base::Value(showButton));
}

void JemaOsHandler::HandleSetShowSwitchTabletLaptopButton(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool showButton = args[0].GetBool();
  PrefService* prefs = g_browser_process->local_state();
  prefs->SetBoolean(jemaos::prefs::kShowSwitchTabletLaptopButton, showButton);
}

void JemaOsHandler::HandleGetShowSwitchTabletLaptopButton(const base::Value::List& args) {
  AllowJavascript();

  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  PrefService* prefs = g_browser_process->local_state();
  bool showButton = prefs->GetBoolean(jemaos::prefs::kShowSwitchTabletLaptopButton);
  ResolveJavascriptCallback(base::Value(callback_id),
                            base::Value(showButton));
}

void JemaOsHandler::HandleGetIsForceTpmFallback(const base::Value::List& args) {
  AllowJavascript();

  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  PrefService* prefs = g_browser_process->local_state();
  base::Value::Dict response;
  bool tpm_fallback = prefs->GetBoolean(jemaos::prefs::kForceTpmFallback);
  bool current_tpm_fallback =
    prefs->GetBoolean(jemaos::prefs::kCurrentForceTpmFallback);
  response.Set("current", current_tpm_fallback);
  response.Set("pref", tpm_fallback);
  ResolveJavascriptCallback(base::Value(callback_id), response);
}

void JemaOsHandler::HandleSetForceTpmFallback(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool tpm_fallback = args[0].GetBool();
  if (!user_manager::UserManager::Get()->IsCurrentUserOwner()) {
    VLOG(2) << "non owner user tried to set "
      << jemaos::prefs::kForceTpmFallback << ", ignored";
    return;
  }
  PrefService* prefs = g_browser_process->local_state();
  const bool necessary =
    prefs->GetBoolean(jemaos::prefs::kForceTpmFallbackNecessary);
  if (!necessary) {
    VLOG(2) << "tried to set " << jemaos::prefs::kForceTpmFallback
      << ", which is not necessary, ignored";
    return;
  }
  prefs->SetBoolean(jemaos::prefs::kForceTpmFallback, tpm_fallback);
}

void JemaOsHandler::OnForceTpmFallbackChanged() {
  PrefService* prefs = g_browser_process->local_state();
  base::Value::Dict response;
  bool tpm_fallback = prefs->GetBoolean(jemaos::prefs::kForceTpmFallback);
  bool current_tpm_fallback =
    prefs->GetBoolean(jemaos::prefs::kCurrentForceTpmFallback);
  response.Set("current", current_tpm_fallback);
  response.Set("pref", tpm_fallback);
  FireWebUIListener("force-tpm-fallback-changed", response);
}

void JemaOsHandler::HandleGetRebootRequiredForWidevine(
    const base::Value::List& args) {
  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  PrefService* prefs = g_browser_process->local_state();
  bool rebootRequired = prefs->GetBoolean(
      jemaos::prefs::kRebootRequiredForWidevine);
  ResolveJavascriptCallback(callback_id, base::Value(rebootRequired));
}

void JemaOsHandler::HandleToggleRebootRequiredForWidevine(
    const base::Value::List& args) {
  if (args.size() < 2 || !args[0].is_string() || !args[1].is_bool()) {
    VLOG(2) << "Invalid arguments for toggleRebootRequiredForWidevine";
    return;
  }
  std::string callback_id = args[0].GetString();
  bool force = args[1].GetBool();
  PrefService* prefs = g_browser_process->local_state();
  bool current = prefs->GetBoolean(jemaos::prefs::kRebootRequiredForWidevine);
  bool rebootRequired;
  if (force) {
    nextToggleRebootRequiredForWidevine_ = current;
    rebootRequired = true;
    lastToggleRebootRequiredForce_ = true;
  } else {
    if (lastToggleRebootRequiredForce_) {
      rebootRequired = nextToggleRebootRequiredForWidevine_;
    } else {
      rebootRequired = !current;
    }
    lastToggleRebootRequiredForce_ = false;
  }
  prefs->SetBoolean(jemaos::prefs::kRebootRequiredForWidevine, rebootRequired);
  ResolveJavascriptCallback(callback_id, base::Value(rebootRequired));
}

void JemaOsHandler::FileSelected(const ui::SelectedFileInfo& file,
                                 int /*index*/) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  select_file_dialog_ = nullptr;
  switch (file_dialog_type_) {
    case FileDialogType::kBackup:
      OnBackupFileSelected(file.path());
      break;
    case FileDialogType::kRestore:
      OnRestoreFileSelected(file.path());
      break;
    case FileDialogType::kUnspecified:
      NOTREACHED();
  }
}

void JemaOsHandler::FileSelectionCanceled() {
  select_file_dialog_ = nullptr;
  switch (file_dialog_type_) {
    case FileDialogType::kBackup:
      OnBackupFileSelectionCanceled();
      break;
    case FileDialogType::kRestore:
      OnRestoreFileSelectionCanceled();
      break;
    case FileDialogType::kUnspecified:
      NOTREACHED();
  }
}

void JemaOsHandler::HandleJemaOSBackupSupported(
    const base::Value::List& args) {
  AllowJavascript();
  CHECK_EQ(1u, args.size());
  const std::string& callback_id = args[0].GetString();
  
  // For development: always enable backup feature
  // In production, this should check for backup script existence
  // TODO: Add proper script detection once deployment is configured
  OnJemaOSBackupScriptChecked(callback_id, true);
}

void JemaOsHandler::OnJemaOSBackupScriptChecked(const std::string& callback_id,
                                                bool is_backup_supported) {
  ResolveJavascriptCallback(
      base::Value(callback_id), base::Value(is_backup_supported));
}

void JemaOsHandler::HandleJemaOSBackupSelectFile(
    const base::Value::List& args) {
  DCHECK(args.size());
  std::string default_filename = args[0].GetString();
  select_file_dialog_ = ui::SelectFileDialog::Create(
      this,
      std::make_unique<ChromeSelectFilePolicy>(web_ui()->GetWebContents()));

  ui::SelectFileDialog::FileTypeInfo file_type_info;
  file_type_info.allowed_paths =
    ui::SelectFileDialog::FileTypeInfo::NATIVE_PATH;
  file_type_info.extensions.resize(1);
  file_type_info.extensions[0].push_back(FILE_PATH_LITERAL("bak"));

  Browser* browser =
      chrome::FindBrowserWithTab(web_ui()->GetWebContents());

  auto* volume_manager = file_manager::VolumeManager::Get(profile_);
  file_manager::Volume* volume = nullptr;
  for (auto& v : volume_manager->GetVolumeList()) {
    if (SuitableForBackupVolume(v.get())) {
      volume = v.get();
      break;
    }
  }
  base::FilePath default_path = volume
    ? volume->mount_path()
    : file_manager::util::GetMyFilesFolderForProfile(profile_);
  const base::FilePath default_filepath = default_path.Append(default_filename);
  file_dialog_type_ = FileDialogType::kBackup;
  select_file_dialog_->SelectFile(
      ui::SelectFileDialog::SELECT_SAVEAS_FILE,
      l10n_util::GetStringUTF16(
        IDS_OS_SETTINGS_JEMAOS_BACKUP_SAVE_FILE_DIALOG_TITLE),
      default_filepath, &file_type_info, 0, base::FilePath::StringType(),
      browser->window()->GetNativeWindow(), nullptr);
}

void JemaOsHandler::OnBackupFileSelected(const base::FilePath& path) {
  BackupTaskManager::GetInstance()->SetBackupFile(path);
  const bool canceled = false;
  FireWebUIListener("jemaos-backup-file-selected", base::Value(canceled));
}

void JemaOsHandler::OnBackupFileSelectionCanceled() {
  const bool canceled = true;
  FireWebUIListener("jemaos-backup-file-selected", base::Value(canceled));
}

void JemaOsHandler::HandleJemaOSBackupStarted(const base::Value::List& args) {
  DCHECK_EQ(args.size(), 2u);
  std::string email = args[0].GetString();
  std::string password = args[1].GetString();

  BackupTaskManager::GetInstance()->SetCallback(
      base::BindRepeating(&JemaOsHandler::OnBackupTaskFinished,
                          weak_ptr_factory_.GetWeakPtr()));
  BackupTaskManager::GetInstance()->StartTask(profile_, email, password);
}

void JemaOsHandler::OnBackupTaskFinished(BackupTaskManager::TaskState state) {
  if (state == BackupTaskManager::TaskState::kRunning
      || state == BackupTaskManager::TaskState::kIdle) {
    return;
  }
  FireWebUIListener("jemaos-backup-task-finished",
      base::Value(state == BackupTaskManager::TaskState::kFinished));
}

void JemaOsHandler::HandleGetJemaOSBackupState(const base::Value::List& args) {
  AllowJavascript();
  CHECK_EQ(1u, args.size());
  const base::Value& callback_id = args[0];
  BackupTaskManager::TaskState state =
    BackupTaskManager::GetInstance()->GetTaskState();
  std::string state_str;
  switch (state) {
    case BackupTaskManager::TaskState::kIdle:
    case BackupTaskManager::TaskState::kFinished:
    case BackupTaskManager::TaskState::kFailed:
      state_str = "not_running";
      break;
    case BackupTaskManager::TaskState::kRunning:
      state_str = "running";
      break;
    default:
      NOTREACHED_IN_MIGRATION();
      break;
  }
  ResolveJavascriptCallback(callback_id, base::Value(state_str));
}

void JemaOsHandler::HandleGetArcMediaAutoScanState(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  const std::string& callback_id = args[0].GetString();
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&ArcMediaAutoScanIndicatorFileExists),
      base::BindOnce(&JemaOsHandler::OnArcMediaAutoScanIndicatorFileExistenceChecked,
                    weak_ptr_factory_.GetWeakPtr(), callback_id));

}

void JemaOsHandler::OnArcMediaAutoScanIndicatorFileExistenceChecked(const std::string& callback_id, bool result) {
  const PrefService::Preference* pref = prefs_->FindPreference(jemaos::prefs::kJemaOSArcMediaAutoScanEnabled);
  int saved = 0;
  if (!pref || pref->IsDefaultValue()) {
    saved = -1;
  } else {
    const bool n = pref->GetValue()->GetBool();
    saved = n ? 1 : 0;
  }
  base::Value::Dict response;
  response.Set("enabled", result);
  response.Set("saved", saved);
  if (callback_id.empty()) {
    FireWebUIListener("jemaos-arc-media-auto-scan-changed", response);
  } else {
    ResolveJavascriptCallback(callback_id, response);
  }
}

void JemaOsHandler::HandleSetArcMediaAutoScanStateForCurrentSession(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool enabled = args[0].GetBool();
  prefs_->SetBoolean(jemaos::prefs::kJemaOSArcMediaAutoScanEnabled, enabled);
}

void JemaOsHandler::HandleSetArcMediaAutoScanState(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool enable = args[0].GetBool();
  if (!enable) {
    base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&DeleteArcMediaAutoScanIndicatorFile),
      base::BindOnce(&JemaOsHandler::OnEnableArcMediaAutoScan,
                     weak_ptr_factory_.GetWeakPtr()));
  } else {
    const std::string empty = std::string();
    base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&CreateArcMediaAutoScanIndicatorFile),
      base::BindOnce(&JemaOsHandler::OnDisableArcMediaAutoScan,
                     weak_ptr_factory_.GetWeakPtr()));
  }
}

void JemaOsHandler::OnEnableArcMediaAutoScan(bool result) {
  RefreshArcMediaAutoScanState();
}

void JemaOsHandler::OnDisableArcMediaAutoScan(bool result) {
  RefreshArcMediaAutoScanState();
}

void JemaOsHandler::RefreshArcMediaAutoScanState() {
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&ArcMediaAutoScanIndicatorFileExists),
      base::BindOnce(&JemaOsHandler::OnArcMediaAutoScanIndicatorFileExistenceChecked,
                 weak_ptr_factory_.GetWeakPtr(), ""));
}

void JemaOsHandler::OnSetDevMode(const std::string& callback_id, bool result) {
  ResolveJavascriptCallback(base::Value(callback_id), base::Value(result));
}

void JemaOsHandler::HandleSetDevMode(const base::Value::List& args) {
  CHECK_EQ(2u, args.size());
  const std::string& callback_id = args[0].GetString();
  bool enable = args[1].GetBool();
  jemaos::misc::SetDevMode(enable, base::BindOnce(&JemaOsHandler::OnSetDevMode,
                                                  weak_ptr_factory_.GetWeakPtr(),
                                                  callback_id));
}

void JemaOsHandler::HandleGetDevModeSwitchSupported(const base::Value::List& args) {
  AllowJavascript();
  CHECK_EQ(1u, args.size());
  const std::string& callback_id = args[0].GetString();
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&jemaos::misc::IsDevModeSwitchSupported),
      base::BindOnce(&JemaOsHandler::OnDevModeSwitchSupportedChecked,
                     weak_ptr_factory_.GetWeakPtr(), callback_id));
}

void JemaOsHandler::OnDevModeSwitchSupportedChecked(const std::string& callback_id, bool result) {
  ResolveJavascriptCallback(base::Value(callback_id), base::Value(result));
}

void JemaOsHandler::HandleJemaOSRestoreSelectFile(
    const base::Value::List& args) {
  DCHECK(args.size() == 0);
  select_file_dialog_ = ui::SelectFileDialog::Create(
      this,
      std::make_unique<ChromeSelectFilePolicy>(web_ui()->GetWebContents()));

  ui::SelectFileDialog::FileTypeInfo file_type_info;
  file_type_info.allowed_paths =
    ui::SelectFileDialog::FileTypeInfo::NATIVE_PATH;
  file_type_info.extensions.resize(1);
  file_type_info.extensions[0].push_back(FILE_PATH_LITERAL("bak"));
  // Also allow all files in case extension is different
  file_type_info.include_all_files = true;

  Browser* browser =
      chrome::FindBrowserWithTab(web_ui()->GetWebContents());

  auto* volume_manager = file_manager::VolumeManager::Get(profile_);
  file_manager::Volume* volume = nullptr;
  for (auto& v : volume_manager->GetVolumeList()) {
    if (SuitableForBackupVolume(v.get())) {
      volume = v.get();
      break;
    }
  }
  base::FilePath default_path = volume
    ? volume->mount_path()
    : file_manager::util::GetMyFilesFolderForProfile(profile_);
  
  file_dialog_type_ = FileDialogType::kRestore;
  select_file_dialog_->SelectFile(
      ui::SelectFileDialog::SELECT_OPEN_FILE,
      l10n_util::GetStringUTF16(
        IDS_OS_SETTINGS_JEMAOS_RESTORE_SELECT_FILE_DIALOG_TITLE),
      default_path, &file_type_info, 0, base::FilePath::StringType(),
      browser->window()->GetNativeWindow(), nullptr);
}

void JemaOsHandler::OnRestoreFileSelected(const base::FilePath& path) {
  const bool canceled = false;
  FireWebUIListener("jemaos-restore-file-selected", 
      base::Value(canceled), base::Value(path.value()));
}

void JemaOsHandler::OnRestoreFileSelectionCanceled() {
  const bool canceled = true;
  FireWebUIListener("jemaos-restore-file-selected", 
      base::Value(canceled), base::Value(std::string()));
}

void JemaOsHandler::HandleJemaOSRestoreStarted(const base::Value::List& args) {
  DCHECK_EQ(args.size(), 3u);
  std::string email = args[0].GetString();
  std::string password = args[1].GetString();
  std::string filePath = args[2].GetString();

  LOG(INFO) << "Restore started for: " << email << " from file: " << filePath;
  
  // Ensure restore script exists (creates from embedded script if missing)
  if (!EnsureRestoreScriptExists()) {
    LOG(ERROR) << "Failed to ensure restore script exists";
    FireWebUIListener("jemaos-restore-task-finished",
        base::Value(false), base::Value("Restore failed: Could not create restore script"));
    return;
  }
  
  // Generate key from email and password (same as backup)
  std::string key = email + ":" + password;
  std::string hex_encoded_hash = base::HexEncode(
      base::SHA1Hash(base::as_byte_span(key)));
  hex_encoded_hash.resize(16);
  std::string restore_key = base::ToLowerASCII(hex_encoded_hash);
  
  // Build restore command using the embedded script
  std::string command = base::StringPrintf(
      "/bin/bash %s restore --email %s --key %s --target %s",
      kRestoreScriptPath, email.c_str(), restore_key.c_str(), filePath.c_str());
  
  // Execute restore using shell client
  auto* shell_client = JemaOSShellClient::Get();
  if (!shell_client) {
    LOG(ERROR) << "Shell client not available for restore";
    FireWebUIListener("jemaos-restore-task-finished",
        base::Value(false), base::Value("Restore failed: System service not available"));
    return;
  }
  
  shell_client->SyncExec(command,
      base::BindOnce(&JemaOsHandler::OnRestoreCompleted,
                     weak_ptr_factory_.GetWeakPtr()));
}

void JemaOsHandler::OnRestoreCompleted(std::optional<ShellState> state) {
  bool success = false;
  std::string message;
  
  if (!state) {
    message = "Restore failed: No response from system";
  } else if (state->code != 0) {
    message = "Restore failed: " + state->result;
  } else {
    success = true;
    message = "Restore completed successfully! Please restart your device to apply changes.";
  }
  
  LOG(INFO) << "Restore completed: success=" << success << ", message=" << message;
  FireWebUIListener("jemaos-restore-task-finished",
      base::Value(success), base::Value(message));
}

// Cloud Backup API constants
const char kCloudBackupApiBaseUrl[] = "https://test-connect-api.jematech.fr";
const char kCloudBackupApiPath[] = "/v1/connect/upload/signed-url-private";

// Cloud Backup implementation
void JemaOsHandler::HandleJemaOSCloudBackupStarted(const base::Value::List& args) {
  DCHECK_EQ(args.size(), 2u);
  std::string email = args[0].GetString();
  std::string password = args[1].GetString();

  LOG(INFO) << "Cloud backup started for: " << email;
  
  // Ensure backup script exists
  if (!EnsureRestoreScriptExists()) {
    LOG(ERROR) << "Failed to ensure backup script exists";
    FireWebUIListener("jemaos-cloud-backup-task-finished",
        base::Value(false), base::Value("Cloud backup failed: Could not create backup script"));
    return;
  }
  
  // Generate key from email and password
  std::string key = email + ":" + password;
  std::string hex_encoded_hash = base::HexEncode(
      base::SHA1Hash(base::as_byte_span(key)));
  hex_encoded_hash.resize(16);
  std::string backup_key = base::ToLowerASCII(hex_encoded_hash);
  
  // Generate temp file path for cloud backup with timestamp
  // Format: jemaos_backup_YYYYMMDD_HHMMSS.bak
  base::Time now = base::Time::Now();
  base::Time::Exploded exploded;
  now.LocalExplode(&exploded);
  std::string timestamp = base::StringPrintf(
      "%04d%02d%02d_%02d%02d%02d",
      exploded.year, exploded.month, exploded.day_of_month,
      exploded.hour, exploded.minute, exploded.second);
  std::string filename = "jemaos_backup_" + timestamp + ".bak";
  std::string temp_file = "/tmp/" + filename;
  
  LOG(INFO) << "Cloud backup filename: " << filename;
  
  // Store cloud backup context for later use
  cloud_backup_email_ = email;
  cloud_backup_filename_ = filename;
  cloud_backup_temp_file_ = temp_file;
  
  // First create local backup to temp file
  std::string command = base::StringPrintf(
      "/bin/bash %s backup --email %s --key %s --target %s",
      kRestoreScriptPath, email.c_str(), backup_key.c_str(), temp_file.c_str());
  
  auto* shell_client = JemaOSShellClient::Get();
  if (!shell_client) {
    LOG(ERROR) << "Shell client not available for cloud backup";
    FireWebUIListener("jemaos-cloud-backup-task-finished",
        base::Value(false), base::Value("Cloud backup failed: System service not available"));
    return;
  }
  
  shell_client->SyncExec(command,
      base::BindOnce(&JemaOsHandler::OnCloudBackupLocalCompleted,
                     weak_ptr_factory_.GetWeakPtr()));
}

void JemaOsHandler::OnCloudBackupLocalCompleted(std::optional<ShellState> state) {
  if (!state || state->code != 0) {
    std::string error_msg = state ? state->result : "No response";
    LOG(ERROR) << "Cloud backup local step failed: " << error_msg;
    FireWebUIListener("jemaos-cloud-backup-task-finished",
        base::Value(false), base::Value("Cloud backup failed: " + error_msg));
    return;
  }
  
  LOG(INFO) << "Local backup created, getting pre-signed URL...";
  
  // Call API to get pre-signed URL
  // fileName: just the filename (API will handle the path)
  std::string api_url = std::string(kCloudBackupApiBaseUrl) + kCloudBackupApiPath;
  std::string json_body = base::StringPrintf(
      "{\"fileName\": \"%s\", \"email_id\": \"%s\"}",
      cloud_backup_filename_.c_str(), cloud_backup_email_.c_str());
  
  LOG(INFO) << "Requesting upload URL for: " << cloud_backup_filename_;
  LOG(INFO) << "API URL: " << api_url;
  LOG(INFO) << "JSON body: " << json_body;
  
  // Write a shell script to execute curl - avoids all escaping issues
  std::string script_file = "/tmp/jemaos_cloud_curl.sh";
  std::string script_content = 
      "#!/bin/bash\n"
      "curl -s -L -X POST \"" + api_url + "\" \\\n"
      "  -H 'Content-Type: application/json' \\\n"
      "  -d '" + json_body + "'\n";
  base::WriteFile(base::FilePath(script_file), script_content);
  chmod(script_file.c_str(), 0755);
  
  std::string command = "/bin/bash " + script_file;
  
  LOG(INFO) << "Executing curl via script: " << script_file;
  
  auto* shell_client = JemaOSShellClient::Get();
  if (!shell_client) {
    LOG(ERROR) << "Shell client not available for API call";
    FireWebUIListener("jemaos-cloud-backup-task-finished",
        base::Value(false), base::Value("Cloud backup failed: System service not available"));
    return;
  }
  
  shell_client->SyncExec(command,
      base::BindOnce(&JemaOsHandler::OnCloudBackupPresignedUrlReceived,
                     weak_ptr_factory_.GetWeakPtr()));
}

void JemaOsHandler::OnCloudBackupPresignedUrlReceived(std::optional<ShellState> state) {
  
  if (!state) {
    LOG(ERROR) << "Failed to get pre-signed URL: No response";
    base::DeleteFile(base::FilePath(cloud_backup_temp_file_));
    FireWebUIListener("jemaos-cloud-backup-task-finished",
        base::Value(false), base::Value("Cloud backup failed: Could not get upload URL (no response)"));
    return;
  }
  
  LOG(INFO) << "Curl exit code: " << state->code;
  LOG(INFO) << "Curl output: " << state->result;
  
  if (state->code != 0) {
    std::string error_msg = state->result;
    LOG(ERROR) << "Failed to get pre-signed URL, curl exit code: " << state->code << ", output: " << error_msg;
    base::DeleteFile(base::FilePath(cloud_backup_temp_file_));
    FireWebUIListener("jemaos-cloud-backup-task-finished",
        base::Value(false), base::Value("Cloud backup failed: Could not get upload URL (curl error " + std::to_string(state->code) + ")"));
    return;
  }
  
  // Parse the response to extract the upload URL
  // Response format: {"success":true,"data":{"uploadUrl":"https://...","fileKey":"...","expiresIn":900},"message":"..."}
  // Note: response may have HTTP status code appended at the end due to -w flag
  std::string response = state->result;
  LOG(INFO) << "API Response: " << response;
  
  // Check for success
  if (response.find("\"success\":true") == std::string::npos &&
      response.find("\"success\": true") == std::string::npos) {
    LOG(ERROR) << "API returned error response";
    base::DeleteFile(base::FilePath(cloud_backup_temp_file_));
    FireWebUIListener("jemaos-cloud-backup-task-finished",
        base::Value(false), base::Value("Cloud backup failed: API error"));
    return;
  }
  
  // Extract uploadUrl from data object
  std::string upload_url;
  size_t url_pos = response.find("\"uploadUrl\"");
  
  if (url_pos != std::string::npos) {
    size_t colon_pos = response.find(':', url_pos);
    size_t quote_start = response.find('"', colon_pos);
    size_t quote_end = response.find('"', quote_start + 1);
    if (quote_start != std::string::npos && quote_end != std::string::npos) {
      upload_url = response.substr(quote_start + 1, quote_end - quote_start - 1);
    }
  }
  
  if (upload_url.empty()) {
    LOG(ERROR) << "Could not parse uploadUrl from response";
    base::DeleteFile(base::FilePath(cloud_backup_temp_file_));
    FireWebUIListener("jemaos-cloud-backup-task-finished",
        base::Value(false), base::Value("Cloud backup failed: Invalid API response"));
    return;
  }
  
  LOG(INFO) << "Got pre-signed URL, uploading backup...";
  LOG(INFO) << "Upload URL length: " << upload_url.length();
  
  // Write a shell script to execute curl upload - avoids all escaping issues
  std::string script_file = "/tmp/jemaos_cloud_upload.sh";
  std::string script_content = 
      "#!/bin/bash\n"
      "curl -s -X PUT \\\n"
      "  -H 'Content-Type: application/octet-stream' \\\n"
      "  -T \"" + cloud_backup_temp_file_ + "\" \\\n"
      "  \"" + upload_url + "\"\n";
  base::WriteFile(base::FilePath(script_file), script_content);
  chmod(script_file.c_str(), 0755);
  
  std::string command = "/bin/bash " + script_file;
  
  LOG(INFO) << "Executing upload via script: " << script_file;
  
  auto* shell_client = JemaOSShellClient::Get();
  shell_client->SyncExec(command,
      base::BindOnce(&JemaOsHandler::OnCloudBackupUploadCompleted,
                     weak_ptr_factory_.GetWeakPtr()));
}

void JemaOsHandler::OnCloudBackupUploadCompleted(std::optional<ShellState> state) {
  // Clean up temp files regardless of result
  base::DeleteFile(base::FilePath(cloud_backup_temp_file_));
  base::DeleteFile(base::FilePath("/tmp/jemaos_cloud_upload.sh"));
  
  if (!state) {
    LOG(ERROR) << "Upload failed: No response";
    FireWebUIListener("jemaos-cloud-backup-task-finished",
        base::Value(false), base::Value("Cloud backup failed: Upload error"));
    return;
  }
  
  LOG(INFO) << "Upload curl exit code: " << state->code;
  LOG(INFO) << "Upload curl output: " << state->result;
  
  // curl returns 0 on success
  if (state->code != 0) {
    LOG(ERROR) << "Upload failed with code: " << state->code;
    FireWebUIListener("jemaos-cloud-backup-task-finished",
        base::Value(false), base::Value("Cloud backup failed: Upload error (code " + std::to_string(state->code) + ")"));
    return;
  }
  
  LOG(INFO) << "Cloud backup uploaded successfully: " << cloud_backup_filename_;
  FireWebUIListener("jemaos-cloud-backup-task-finished",
      base::Value(true), 
      base::Value("Backup uploaded successfully: " + cloud_backup_filename_));
}

// Cloud Restore API paths
const char kCloudRestoreApiPath[] = "/v1/connect/download/signed-url-private";
const char kCloudListFilesApiPath[] = "/v1/connect/backup/files/list";

// List backup files implementation
void JemaOsHandler::HandleJemaOSCloudListBackupFiles(const base::Value::List& args) {
  DCHECK_EQ(args.size(), 1u);
  std::string email = args[0].GetString();

  LOG(INFO) << "Listing cloud backup files for: " << email;
  
  // Store email for later use
  cloud_backup_email_ = email;
  
  // Call API to list backup files
  std::string api_url = std::string(kCloudBackupApiBaseUrl) + kCloudListFilesApiPath;
  std::string json_body = base::StringPrintf(
      "{\"email_id\": \"%s\", \"filter\": \"all\"}",
      email.c_str());
  
  LOG(INFO) << "List files API URL: " << api_url;
  LOG(INFO) << "List files JSON body: " << json_body;
  
  // Write a shell script to execute curl
  std::string script_file = "/tmp/jemaos_cloud_list_files.sh";
  std::string script_content = 
      "#!/bin/bash\n"
      "curl -s -L -X POST \"" + api_url + "\" \\\n"
      "  -H 'Content-Type: application/json' \\\n"
      "  -d '" + json_body + "'\n";
  base::WriteFile(base::FilePath(script_file), script_content);
  chmod(script_file.c_str(), 0755);
  
  std::string command = "/bin/bash " + script_file;
  
  LOG(INFO) << "Executing list files curl via script";
  
  auto* shell_client = JemaOSShellClient::Get();
  if (!shell_client) {
    LOG(ERROR) << "Shell client not available for listing files";
    FireWebUIListener("jemaos-cloud-list-files-result",
        base::Value(false), base::Value::List());
    return;
  }
  
  shell_client->SyncExec(command,
      base::BindOnce(&JemaOsHandler::OnCloudListFilesCompleted,
                     weak_ptr_factory_.GetWeakPtr()));
}

void JemaOsHandler::OnCloudListFilesCompleted(std::optional<ShellState> state) {
  // Clean up script file
  base::DeleteFile(base::FilePath("/tmp/jemaos_cloud_list_files.sh"));
  
  if (!state || state->code != 0) {
    std::string error_msg = state ? state->result : "No response";
    LOG(ERROR) << "Failed to list backup files: " << error_msg;
    FireWebUIListener("jemaos-cloud-list-files-result",
        base::Value(false), base::Value::List());
    return;
  }
  
  LOG(INFO) << "List files API response: " << state->result;
  
  // Parse the JSON response to extract files array
  std::string response = state->result;
  
  // Check for success
  if (response.find("\"success\":true") == std::string::npos &&
      response.find("\"success\": true") == std::string::npos) {
    LOG(ERROR) << "API returned error - failed to list files";
    FireWebUIListener("jemaos-cloud-list-files-result",
        base::Value(false), base::Value::List());
    return;
  }
  
  // Parse files array - simple parsing for the expected format
  base::Value::List files_list;
  
  // Find "files": [ array
  size_t files_start = response.find("\"files\"");
  if (files_start != std::string::npos) {
    size_t array_start = response.find('[', files_start);
    size_t array_end = response.find(']', array_start);
    
    if (array_start != std::string::npos && array_end != std::string::npos) {
      std::string files_array = response.substr(array_start, array_end - array_start + 1);
      
      // Parse each file object
      size_t pos = 0;
      while ((pos = files_array.find('{', pos)) != std::string::npos) {
        size_t obj_end = files_array.find('}', pos);
        if (obj_end == std::string::npos) break;
        
        std::string file_obj = files_array.substr(pos, obj_end - pos + 1);
        
        // Extract fileKey
        std::string file_key;
        size_t key_pos = file_obj.find("\"fileKey\"");
        if (key_pos != std::string::npos) {
          size_t colon = file_obj.find(':', key_pos);
          size_t quote1 = file_obj.find('"', colon);
          size_t quote2 = file_obj.find('"', quote1 + 1);
          if (quote1 != std::string::npos && quote2 != std::string::npos) {
            file_key = file_obj.substr(quote1 + 1, quote2 - quote1 - 1);
          }
        }
        
        // Extract fileName
        std::string file_name;
        size_t name_pos = file_obj.find("\"fileName\"");
        if (name_pos != std::string::npos) {
          size_t colon = file_obj.find(':', name_pos);
          size_t quote1 = file_obj.find('"', colon);
          size_t quote2 = file_obj.find('"', quote1 + 1);
          if (quote1 != std::string::npos && quote2 != std::string::npos) {
            file_name = file_obj.substr(quote1 + 1, quote2 - quote1 - 1);
          }
        }
        
        // Extract size
        int64_t size = 0;
        size_t size_pos = file_obj.find("\"size\"");
        if (size_pos != std::string::npos) {
          size_t colon = file_obj.find(':', size_pos);
          size_t num_start = colon + 1;
          while (num_start < file_obj.size() && (file_obj[num_start] == ' ' || file_obj[num_start] == ':')) {
            num_start++;
          }
          size_t num_end = num_start;
          while (num_end < file_obj.size() && std::isdigit(file_obj[num_end])) {
            num_end++;
          }
          if (num_end > num_start) {
            size = std::stoll(file_obj.substr(num_start, num_end - num_start));
          }
        }
        
        // Extract lastModified
        std::string last_modified;
        size_t mod_pos = file_obj.find("\"lastModified\"");
        if (mod_pos != std::string::npos) {
          size_t colon = file_obj.find(':', mod_pos);
          size_t quote1 = file_obj.find('"', colon);
          size_t quote2 = file_obj.find('"', quote1 + 1);
          if (quote1 != std::string::npos && quote2 != std::string::npos) {
            last_modified = file_obj.substr(quote1 + 1, quote2 - quote1 - 1);
          }
        }
        
        if (!file_key.empty() && !file_name.empty()) {
          base::Value::Dict file_dict;
          file_dict.Set("fileKey", file_key);
          file_dict.Set("fileName", file_name);
          file_dict.Set("size", static_cast<double>(size));
          file_dict.Set("lastModified", last_modified);
          files_list.Append(std::move(file_dict));
        }
        
        pos = obj_end + 1;
      }
    }
  }
  
  // Sort files by lastModified date (latest first)
  std::sort(files_list.begin(), files_list.end(),
      [](const base::Value& a, const base::Value& b) {
        const std::string* date_a = a.GetDict().FindString("lastModified");
        const std::string* date_b = b.GetDict().FindString("lastModified");
        if (!date_a || !date_b) return false;
        // ISO date strings can be compared lexicographically
        return *date_a > *date_b;  // Descending order (latest first)
      });
  
  LOG(INFO) << "Parsed " << files_list.size() << " backup files (sorted by date, latest first)";
  FireWebUIListener("jemaos-cloud-list-files-result",
      base::Value(true), std::move(files_list));
}

// Cloud Restore implementation
void JemaOsHandler::HandleJemaOSCloudRestoreStarted(const base::Value::List& args) {
  // Accept 3 arguments: email, password, fileKey
  DCHECK_GE(args.size(), 2u);
  std::string email = args[0].GetString();
  std::string password = args[1].GetString();
  
  // Get fileKey from third argument (full path from the file list API)
  std::string file_key;
  if (args.size() >= 3 && args[2].is_string() && !args[2].GetString().empty()) {
    file_key = args[2].GetString();
  } else if (!cloud_backup_filename_.empty()) {
    // Use filename from last backup in this session - construct the full path
    file_key = "common/" + email + "/" + cloud_backup_filename_;
  } else {
    // Default fallback
    file_key = "common/" + email + "/jemaos_backup.bak";
  }

  LOG(INFO) << "Cloud restore started for: " << email;
  LOG(INFO) << "Restore fileKey: " << file_key;
  
  // Store restore context
  cloud_backup_email_ = email;
  cloud_restore_password_ = password;
  cloud_restore_temp_file_ = "/tmp/jemaos_cloud_restore.bak";
  
  // Call API to get download pre-signed URL
  std::string api_url = std::string(kCloudBackupApiBaseUrl) + kCloudRestoreApiPath;
  std::string json_body = base::StringPrintf(
      "{\"fileKey\": \"%s\"}",
      file_key.c_str());
  
  LOG(INFO) << "Requesting download URL for: " << file_key;
  LOG(INFO) << "API URL: " << api_url;
  LOG(INFO) << "JSON body: " << json_body;
  
  // Write a shell script to execute curl - avoids all escaping issues
  std::string script_file = "/tmp/jemaos_cloud_curl.sh";
  std::string script_content = 
      "#!/bin/bash\n"
      "curl -s -L -X POST \"" + api_url + "\" \\\n"
      "  -H 'Content-Type: application/json' \\\n"
      "  -d '" + json_body + "'\n";
  base::WriteFile(base::FilePath(script_file), script_content);
  chmod(script_file.c_str(), 0755);
  
  std::string command = "/bin/bash " + script_file;
  
  LOG(INFO) << "Executing curl via script: " << script_file;
  
  auto* shell_client = JemaOSShellClient::Get();
  if (!shell_client) {
    LOG(ERROR) << "Shell client not available for cloud restore";
    FireWebUIListener("jemaos-cloud-restore-task-finished",
        base::Value(false), base::Value("Cloud restore failed: System service not available"));
    return;
  }
  
  shell_client->SyncExec(command,
      base::BindOnce(&JemaOsHandler::OnCloudRestorePresignedUrlReceived,
                     weak_ptr_factory_.GetWeakPtr()));
}

void JemaOsHandler::OnCloudRestorePresignedUrlReceived(std::optional<ShellState> state) {
  
  if (!state) {
    LOG(ERROR) << "Failed to get download URL: No response";
    FireWebUIListener("jemaos-cloud-restore-task-finished",
        base::Value(false), base::Value("Cloud restore failed: Could not get download URL (no response)"));
    return;
  }
  
  LOG(INFO) << "Curl exit code: " << state->code;
  LOG(INFO) << "Curl output: " << state->result;
  
  if (state->code != 0) {
    std::string error_msg = state->result;
    LOG(ERROR) << "Failed to get download URL, curl exit code: " << state->code << ", output: " << error_msg;
    FireWebUIListener("jemaos-cloud-restore-task-finished",
        base::Value(false), base::Value("Cloud restore failed: Could not get download URL (curl error " + std::to_string(state->code) + ")"));
    return;
  }
  
  // Parse response to extract download URL
  // Response format: {"success":true,"data":{"downloadUrl":"https://...","expiresIn":900},"message":"..."}
  // Note: response may have HTTP status code appended at the end due to -w flag
  std::string response = state->result;
  LOG(INFO) << "Download API Response: " << response;
  
  // Check for success
  if (response.find("\"success\":true") == std::string::npos &&
      response.find("\"success\": true") == std::string::npos) {
    LOG(ERROR) << "API returned error - no backup found in cloud";
    FireWebUIListener("jemaos-cloud-restore-task-finished",
        base::Value(false), base::Value("Cloud restore failed: No backup found in cloud for this account"));
    return;
  }
  
  // Extract downloadUrl from data object
  std::string download_url;
  size_t url_pos = response.find("\"downloadUrl\"");
  
  if (url_pos != std::string::npos) {
    size_t colon_pos = response.find(':', url_pos);
    size_t quote_start = response.find('"', colon_pos);
    size_t quote_end = response.find('"', quote_start + 1);
    if (quote_start != std::string::npos && quote_end != std::string::npos) {
      download_url = response.substr(quote_start + 1, quote_end - quote_start - 1);
    }
  }
  
  if (download_url.empty()) {
    LOG(ERROR) << "Could not parse downloadUrl from response";
    FireWebUIListener("jemaos-cloud-restore-task-finished",
        base::Value(false), base::Value("Cloud restore failed: No backup found in cloud"));
    return;
  }
  
  LOG(INFO) << "Got download URL, downloading backup...";
  LOG(INFO) << "Download URL length: " << download_url.length();
  
  // Write a shell script to execute curl download - avoids all escaping issues
  std::string script_file = "/tmp/jemaos_cloud_download.sh";
  std::string script_content = 
      "#!/bin/bash\n"
      "curl -s -o \"" + cloud_restore_temp_file_ + "\" \\\n"
      "  \"" + download_url + "\"\n";
  base::WriteFile(base::FilePath(script_file), script_content);
  chmod(script_file.c_str(), 0755);
  
  std::string command = "/bin/bash " + script_file;
  
  LOG(INFO) << "Executing download via script: " << script_file;
  
  auto* shell_client = JemaOSShellClient::Get();
  shell_client->SyncExec(command,
      base::BindOnce(&JemaOsHandler::OnCloudRestoreDownloadCompleted,
                     weak_ptr_factory_.GetWeakPtr()));
}

void JemaOsHandler::OnCloudRestoreDownloadCompleted(std::optional<ShellState> state) {
  // Clean up script temp file
  base::DeleteFile(base::FilePath("/tmp/jemaos_cloud_download.sh"));
  
  if (!state) {
    LOG(ERROR) << "Download failed: No response";
    FireWebUIListener("jemaos-cloud-restore-task-finished",
        base::Value(false), base::Value("Cloud restore failed: Download error (no response)"));
    return;
  }
  
  LOG(INFO) << "Download curl exit code: " << state->code;
  LOG(INFO) << "Download curl output: " << state->result;
  
  if (state->code != 0) {
    LOG(ERROR) << "Download failed with code: " << state->code;
    FireWebUIListener("jemaos-cloud-restore-task-finished",
        base::Value(false), base::Value("Cloud restore failed: Download error (code " + std::to_string(state->code) + ")"));
    return;
  }
  
  // Check if file was downloaded
  if (!base::PathExists(base::FilePath(cloud_restore_temp_file_))) {
    LOG(ERROR) << "Downloaded file not found";
    FireWebUIListener("jemaos-cloud-restore-task-finished",
        base::Value(false), base::Value("Cloud restore failed: Downloaded file not found"));
    return;
  }
  
  LOG(INFO) << "Backup downloaded, starting restore...";
  
  // Ensure restore script exists
  if (!EnsureRestoreScriptExists()) {
    base::DeleteFile(base::FilePath(cloud_restore_temp_file_));
    FireWebUIListener("jemaos-cloud-restore-task-finished",
        base::Value(false), base::Value("Cloud restore failed: Could not create restore script"));
    return;
  }
  
  // Generate key from email and password
  std::string key = cloud_backup_email_ + ":" + cloud_restore_password_;
  std::string hex_encoded_hash = base::HexEncode(
      base::SHA1Hash(base::as_byte_span(key)));
  hex_encoded_hash.resize(16);
  std::string restore_key = base::ToLowerASCII(hex_encoded_hash);
  
  // Run restore
  std::string command = base::StringPrintf(
      "/bin/bash %s restore --email %s --key %s --target %s",
      kRestoreScriptPath, cloud_backup_email_.c_str(), restore_key.c_str(), 
      cloud_restore_temp_file_.c_str());
  
  auto* shell_client = JemaOSShellClient::Get();
  shell_client->SyncExec(command,
      base::BindOnce(&JemaOsHandler::OnCloudRestoreCompleted,
                     weak_ptr_factory_.GetWeakPtr()));
}

void JemaOsHandler::OnCloudRestoreCompleted(std::optional<ShellState> state) {
  // Clean up temp file
  base::DeleteFile(base::FilePath(cloud_restore_temp_file_));
  
  bool success = false;
  std::string message;
  
  if (!state) {
    message = "Cloud restore failed: No response from system";
  } else if (state->code != 0) {
    message = "Cloud restore failed: " + state->result;
  } else {
    success = true;
    message = "Cloud restore completed successfully! Please restart your device.";
  }
  
  LOG(INFO) << "Cloud restore completed: success=" << success;
  FireWebUIListener("jemaos-cloud-restore-task-finished",
      base::Value(success), base::Value(message));
}

}  // namespace ash::settings
