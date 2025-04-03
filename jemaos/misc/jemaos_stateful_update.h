// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_STATEFUL_UPDATE_H
#define JEMAOS_STATEFUL_UPDATE_H

#include <string>
#include <memory>

#include "third_party/abseil-cpp/absl/types/optional.h"
#include "chromeos/chromeos_export.h"
#include "chromeos/ash/components/login/login_state/login_state.h"
#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/shell_state.h"

class Profile;

namespace base {
class OneShotTimer;
class RepeatingTimer;
}

namespace message_center {
class Notification;
}

namespace jemaos {

namespace misc {

// Manages the stateful update process for JemaOS
class StatefulUpdater {
 public:
    // Constructor
    StatefulUpdater();

    // Destructor
    ~StatefulUpdater();

    // Starts the stateful update process
    void Start();

    // Stops the stateful update process
    void Stop();

 private:
    // Delays the update check
    void CheckUpdateDelay();

    // Checks for available updates
    void CheckUpdate();

    // Callback for when the update check is complete
    void OnCheckUpdateDone(absl::optional<jemaos::ash::ShellState> state);

    // Generates a notification for available updates
    void GenerateUpdateNotification();

    // Closes the update notification
    void CloseUpdateNotification();

    // Performs the update
    void DoUpdate();

    // Callback for when the update starts
    void OnDoUpdateStarted(absl::optional<jemaos::ash::ShellState> state);

    // Generates a progress notification for the update
    void GenerateProgressNotification();

    // Closes the progress notification
    void CloseProgressNotification();

    // Generates a reboot notification
    void GenerateRebootNotification();

    // Updates the progress notification
    void TrytoUpdateProgressNotification(std::string result);

    // Closes the reboot notification
    void CloseRebootNotification();

    // Checks the update status
    void CheckUpdateStatus();

    // Callback for when the update status check is complete
    void OnCheckUpdateStatusDone(absl::optional<jemaos::ash::ShellState> state);

    // Reboots the system
    void Reboot();

    // Handles clicks on the update notification
    void HandleUpdateNotificationClick(absl::optional<int> button_index);

    // Handles clicks on the reboot notification
    void HandleRebootNotificationClick(absl::optional<int> button_index);

    Profile* profile_ = nullptr;
    bool started_ = false;
    std::unique_ptr<base::OneShotTimer> timer_;
    std::unique_ptr<base::RepeatingTimer> status_timer_;
    std::unique_ptr<message_center::Notification> progress_notification_;
    int check_update_count_;
    int check_status_count_;

    base::WeakPtrFactory<StatefulUpdater> weak_ptr_factory_{this};
};

}  // namespace misc

}  // namespace jemaos

#endif  // JEMAOS_STATEFUL_UPDATE_H