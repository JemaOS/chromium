// Copyright 2020 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_LICENSE_ENFORCEMENT_H_
#define JEMAOS_LICENSE_ENFORCEMENT_H_

#include <string>
#include <memory>
#include "chromeos/ash/components/dbus/update_engine/update_engine_client.h"

class Profile;

namespace base {
  class OneShotTimer;
  class RepeatingTimer;
}
namespace content {
  class WebContents;
}

namespace message_center{
  class Notification;
}

namespace jemaos {
namespace license {

enum EnforcementMode {
  EnforcementModeLevel0 = 0,
  EnforcementModeLevel1 = 1,
  EnforcementModeLevel2 = 2,
  // EnforcementModeLevel3 = 3,
};

const EnforcementMode EnforcementModeDefault = EnforcementModeLevel2;

class LicenseEnforcement {
  public:
    explicit LicenseEnforcement();
    ~LicenseEnforcement();

    void StartEnforcement(Profile* profile,
                          const std::string& licenseID,
                          const std::string& serialNumber,
                          EnforcementMode mode,
                          int logOutInterval);
    void StopEnforcement();
    // Freemium information: at most ONE discreet notification per session.
    // No popup window, no timer, no logout. Removed by StopEnforcement()
    // when the subscription becomes active.
    void ShowFreemiumNotice(Profile* profile);

  private:
    void StartEnforcementInternal();
    void Enforce();
    void PopupLicenseWindow(bool from_user_interaction);
    void CloseLicenseWindow();
    void ForceQuitCurrentUser();

    void ForceQuitNotification();

    void OnForceQuitNotificationClicked();
    void RemoveForceQuitNotification();

    void OnGetEolInfo(::ash::UpdateEngineClient::EolInfo info);

    std::unique_ptr<base::RepeatingTimer> enforce_timer_;
    std::unique_ptr<message_center::Notification> notification_;
    Profile* profile_;
    std::string id_;
    std::string serial_number_;
    EnforcementMode mode_ = EnforcementModeLevel0;
    int log_out_interval_ = 0;
    bool is_eol_;
    // One popup and one notification per enforcement session. The old code
    // re-displayed the notification every 10 s and logged the user out on a
    // timer, which produced the sign-out loop with expired licenses.
    bool session_popup_shown_ = false;
    bool session_notified_ = false;
}; // class LicenseEnforcement

} // namespace license

} // namespace jemaos
#endif
