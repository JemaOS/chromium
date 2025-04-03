// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_STATISTICS_COLLECTOR_H
#define JEMAOS_STATISTICS_COLLECTOR_H

#include <memory>
#include <string>
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "chromeos/chromeos_export.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "base/functional/callback.h"
#include "chromeos/ash/components/login/login_state/login_state.h"
#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/shell_state.h"

class Profile;

namespace chromeos {
namespace system {
class StatisticsProvider;
}
}

namespace network {
class SimpleURLLoader;
}

namespace jemaos {
namespace misc {

// Collects and uploads system and user statistics for JemaOS
class StatisticsCollector {
 public:
    // Constructor
    StatisticsCollector();

    // Destructor
    ~StatisticsCollector();

    // Starts the statistics collection process
    void Start();

    // Stops the statistics collection process
    void Stop();

 private:
    // Structure to hold collected statistics
    struct Statistics {
      std::string license_id;
      std::string license_type;
      std::string major_version;
      std::string board_name;
      std::string hardware_model_name;
      std::string region;
      std::string ethernet_mac_address;
      std::string dock_mac_address;
      std::string os_version;
      std::string kernel_version;
      base::Time os_release_time;
      std::string channel_name;
      std::string browser_version;
      std::string browser_milestone;
      std::string browser_language;
      bool is_multi_boot;

      std::string profile_account_id;
      std::string profile_account_type;
      base::Time profile_creation_time;
      base::Time profile_start_time;
      bool is_new_profile;
      bool is_jema_profile;
    };

    // Enumeration of collection steps
    enum CollectStep {
      INITIALIZE = 0,
      GET_LICENSE_ID,
      GET_LICENSE_TYPE,
      GET_INSTALL_TYPE,
      SYNC_COLLECT,
    };

    // Internal methods for managing the collection process
    void StartInternal();
    void OnMachineStatisticsLoaded();
    void ProceedToNextStep();

    // Methods for collecting specific statistics
    void GetLicenseId();
    void OnGetLicenseId(absl::optional<jemaos::ash::ShellState> state);

    void GetLicenseType();
    void OnGetLicenseType(absl::optional<jemaos::ash::ShellState> state);
    void GetLicenseTypeFromInfo(const std::string& license);

    void GetInstallType();
    void OnGetInstallType(absl::optional<jemaos::ash::ShellState> state);

    void SyncCollect();

    // Methods for collecting user and device information
    void CollectUserInfo();
    void CollectDeviceInfo();

    // Methods for uploading collected statistics
    bool UploadStatistics();
    bool GetUploadData(std::string* output);
    bool EncryptData(const std::string& text, std::string* encrypted);
    void OnUploaded(std::unique_ptr<network::SimpleURLLoader> url_loader,
                    std::unique_ptr<std::string> response_body);

    // Member variables
    bool started_ = false;
    CollectStep step_;
    Statistics statistics_ = {};
    Profile* profile_;

    base::WeakPtrFactory<StatisticsCollector> weak_factory_{this};
};

}  // namespace misc
}  // namespace jemaos

#endif  // JEMAOS_STATISTICS_COLLECTOR_H