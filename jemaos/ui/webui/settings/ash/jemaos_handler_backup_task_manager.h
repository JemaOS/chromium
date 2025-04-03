// Copyright (c) 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _JEMAOS_UI_WEBUI_SETTINGS_ASH_JEMAOS_HANDLER_BACKUP_TASK_MANAGER_H_
#define _JEMAOS_UI_WEBUI_SETTINGS_ASH_JEMAOS_HANDLER_BACKUP_TASK_MANAGER_H_

#include <string>
#include <utility>
#include <memory>
#include "base/memory/weak_ptr.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "base/functional/callback.h"
#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/shell_state.h"
#include "base/files/file_path.h"

namespace base {
class OneShotTimer;
class FilePath;
}

namespace message_center {
class Notification;
}

class Profile;

namespace jemaos::ash {
class JemaOSShellClient;
}

namespace ash::settings {

using jemaos::ash::JemaOSShellClient;
using jemaos::ash::ShellState;

class BackupTaskManager {
 public:
  // Enum representing the state of the backup task
  // NOTE FOR DEVELOPERS: Add new states here if additional task states are introduced.
  enum class TaskState {
    kIdle,     // Task is idle and not running
    kRunning,  // Task is currently running
    kFinished, // Task has completed successfully
    kFailed,   // Task has failed
  };

  using BackupTaskCallback = base::OnceCallback<void(TaskState)>;

  // Constructor and destructor
  // NOTE FOR DEVELOPERS: Ensure proper cleanup of resources in the destructor.
  BackupTaskManager();
  ~BackupTaskManager();

  // Singleton instance management
  // NOTE FOR DEVELOPERS: Use these methods to manage the global instance of the BackupTaskManager.
  static BackupTaskManager* GetInstance();
  static void DestroyInstance();

  // Starts a backup task
  // NOTE FOR DEVELOPERS: Ensure the profile, email, and password are valid before calling this method.
  void StartTask(Profile* profile,
                 const std::string& email,
                 const std::string& password);

  // Callback for when the backup task starts
  void OnTaskStarted(absl::optional<ShellState> state);

  // Retrieves the output and state of the backup task
  void GetTaskOutputAndState();

  // Callback for when the task output and state are retrieved
  void OnGetTaskOutputAndState(absl::optional<ShellState> state);

  // Schedules periodic retrieval of task output and state
  void ScheduleGetTaskOutputAndState();

  // Sets the backup file path
  void SetBackupFile(const base::FilePath& path) { backup_path_ = path; }

  // Retrieves the current state of the backup task
  TaskState GetTaskState() const { return task_state_; }

  // Sets a callback to be invoked when the task state changes
  void SetCallback(BackupTaskCallback callback) {
    callback_ = std::move(callback);
  }

 private:
  // Displays a notification for the backup task
  // NOTE FOR DEVELOPERS: Customize the notification appearance and behavior as needed.
  void DisplayNotification(
      std::unique_ptr<message_center::Notification> notification);

  // Handles the completion of the backup task
  void OnBackupFinished();

  // Checks if the backup file exists
  void OnCheckBackupFile(const bool exists);

  // Displays a notification when the backup is finished
  void ShowBackupFinishedNotification(
      const base::FilePath& file_path, const bool exists);

  // Retrieves the state of the shell client task
  void GetShellClientTaskState();

  // Callback for when the shell client task state is retrieved
  void OnGetShellClientTaskState(absl::optional<ShellState> state);

  // Handles backup errors
  void OnBackupError();

  // Checks if the temporary log file exists
  void OnCheckTmpLogFile(const bool exists);

  // Processes the temporary log file using the shell client
  void ProcessTmpLogFileByShellClient(
      const base::FilePath& tmp, ReadTmpLogCallback callback);

  // Callback for when the temporary log file is read
  void OnTmpLogFileRead(
      ReadTmpLogCallback callback, absl::optional<ShellState> state);

  // Callback for when the log file is copied
  void OnLogFileCopied(ReadTmpLogCallback callback,
                       const base::FilePath& log, const bool exists);

  // Displays a notification when a backup error occurs
  void ShowBackupErrorNotification(
      const base::FilePath& log, const bool exists);

  // Member variables
  JemaOSShellClient* shell_client_ = nullptr;  // Shell client instance
  int32_t task_id_ = 0;                        // ID of the current task
  TaskState task_state_ = TaskState::kIdle;    // Current state of the task
  Profile* profile_ = nullptr;                 // Profile associated with the task
  base::FilePath backup_path_;                 // Path to the backup file
  base::FilePath tmp_log_path_;                // Path to the temporary log file
  BackupTaskCallback callback_;                // Callback for task state changes

  scoped_refptr<base::SequencedTaskRunner> task_runner_;  // Task runner for background tasks
  base::WeakPtrFactory<BackupTaskManager> weak_ptr_factory_{this};  // Weak pointer factory
};

}  // namespace ash::settings

#endif  // _JEMAOS_UI_WEBUI_SETTINGS_ASH_JEMAOS_HANDLER_BACKUP_TASK_MANAGER_H_