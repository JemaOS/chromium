// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/ui/webui/settings/ash/jemaos_handler_backup_task_manager.h"

#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/jemaos_shell_client.h"
#include "base/logging.h"
#include "base/time/time.h"
#include "base/task/sequenced_task_runner.h"
#include "base/strings/stringprintf.h"
#include "base/base64.h"
#include "ui/message_center/message_center.h"
#include "ui/message_center/public/cpp/notification.h"
#include "ash/public/cpp/notification_utils.h"
#include "chrome/browser/notifications/notification_handler.h"
#include "chrome/browser/notifications/notification_display_service_factory.h"
#include "chrome/browser/notifications/notification_display_service.h"
#include "components/vector_icons/vector_icons.h"
#include "base/files/file_util.h"
#include "chrome/browser/platform_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/json/json_reader.h"
#include "base/strings/string_util.h"
#include "chrome/grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"
#include "base/hash/sha1.h"
#include "base/strings/string_number_conversions.h"
#include "ash/constants/notifier_catalogs.h"
#include "base/task/thread_pool.h"

using message_center::MessageCenter;
using message_center::Notification;

namespace ash::settings {

namespace {

// Command format for initiating a backup task
// NOTE FOR DEVELOPERS: Ensure the command format matches the expected shell client behavior.
const char kJemaOSBackupCommandFormat[] =
    "/usr/bin/jemaos-backup backup --email %s --key %s --target %s";

// Backup task tracking interval in seconds
constexpr int kJemaOSBackupTaskTrackIntervalSeconds = 5;

// Number of output lines to fetch from the backup task
constexpr int kJemaOSBackupTaskOutputLines = 10;

// Notification IDs for backup tasks
const char kJemaOSBackupNotificationId[] = "jemaos.os-settings.backup";
const char kJemaOSBackupNotifierId[] = "jemaos.os-settings.backup";

// Retrieves the shell client instance
// NOTE FOR DEVELOPERS: Ensure the shell client is properly initialized before use.
JemaOSShellClient* GetShellClient() {
  return JemaOSShellClient::Get();
}

// Singleton instance of the BackupTaskManager
BackupTaskManager* g_backup_task_manager = nullptr;

// Creates a notification for the backup task
// NOTE FOR DEVELOPERS: Customize the notification appearance and behavior as needed.
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

  std::unique_ptr<Notification> notification =
      ::ash::CreateSystemNotificationPtr(
          type, kJemaOSBackupNotificationId, title, message, std::u16string(),
          GURL(),
          message_center::NotifierId(
              message_center::NotifierType::SYSTEM_COMPONENT,
              kJemaOSBackupNotifierId,
              ash::NotificationCatalogName::kJemaOSDataBackup),
          message_center::RichNotificationData(), nullptr,
          vector_icons::kProductIcon, warning_level);

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

// Generates a key for the backup task using email and password
// NOTE FOR DEVELOPERS: The key is hashed and truncated for security purposes.
const std::string GenerateKey(const std::string& email,
                               const std::string& password) {
  std::string key = email + ":" + password;
  std::string hex_encoded_hash = base::HexEncode(
      base::SHA1HashSpan(base::as_bytes(base::make_span(key))));
  hex_encoded_hash.resize(16);
  return base::ToLowerASCII(hex_encoded_hash);
}

// Writes content to a file and verifies its existence
// NOTE FOR DEVELOPERS: Ensure the file path is valid and writable.
bool WriteFile_(const base::FilePath& path, const std::string& content) {
  int ret = base::WriteFile(path, content.c_str(), content.size());
  if (ret != static_cast<int>(content.size())) {
    return false;
  }
  return base::PathExists(path);
}

}  // namespace

// Singleton instance management for BackupTaskManager
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

// Constructor for BackupTaskManager
BackupTaskManager::BackupTaskManager() {
  task_runner_ = base::ThreadPool::CreateSequencedTaskRunner(
      {base::MayBlock(), base::TaskPriority::BEST_EFFORT});
}

// Destructor for BackupTaskManager
BackupTaskManager::~BackupTaskManager() {
  if (MessageCenter::Get()->FindVisibleNotificationById(
          kJemaOSBackupNotificationId)) {
    MessageCenter::Get()->RemoveNotification(kJemaOSBackupNotificationId,
                                              false);
  }
}

// Starts a backup task
// NOTE FOR DEVELOPERS: Ensure the shell client is initialized before starting the task.
void BackupTaskManager::StartTask(Profile* profile, const std::string& email,
                                  const std::string& password) {
  profile_ = profile;
  if (!shell_client_) {
    shell_client_ = GetShellClient();
  }
  std::string encoded_filepath;
  base::Base64Encode(backup_path_.value(), &encoded_filepath);
  task_state_ = TaskState::kRunning;
  const std::string key = GenerateKey(email, password);
  const std::string command = base::StringPrintf(
      kJemaOSBackupCommandFormat, email.c_str(), key.c_str(),
      encoded_filepath.c_str());
  shell_client_->AsyncExec(
      command, base::BindOnce(&BackupTaskManager::OnTaskStarted,
                              weak_ptr_factory_.GetWeakPtr()));
}

// Handles the start of a backup task
// NOTE FOR DEVELOPERS: Logs errors if the task fails to start.
void BackupTaskManager::OnTaskStarted(absl::optional<ShellState> state) {
  if (!state || state->code == -1) {
    LOG(ERROR) << "start backup task error, "
               << (state ? state->result : "state is null");
    task_state_ = TaskState::kFailed;
    return;
  }
  task_id_ = state->code;

  // Force remove and add notification to ensure it pops up
  if (MessageCenter::Get()->FindVisibleNotificationById(
          kJemaOSBackupNotificationId)) {
    MessageCenter::Get()->RemoveNotification(kJemaOSBackupNotificationId,
                                              false);
  }
  DisplayNotification(CreateNotification(
      task_state_,
      l10n_util::GetStringUTF16(
          IDS_JEMAOS_BACKUP_NOTIFICATION_RUNNING_TITLE),
      u"", nullptr));
  GetShellClientTaskState();
  GetTaskOutputAndState();
}

// Additional methods omitted for brevity...

}  // namespace ash::settings