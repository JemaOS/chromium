// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/misc/jemaos_toggle_ota.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "base/logging.h"
#include "base/functional/bind.h"
#include "base/files/file_util.h"
#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/jemaos_shell_client.h"

using jemaos::ash::JemaOSShellClient;
using jemaos::ash::ShellState;

namespace jemaos {
namespace misc {

namespace {

  // Retrieves the JemaOS Shell client instance
  JemaOSShellClient* GetShellClient() {
    return JemaOSShellClient::Get();
  }

  // Path to the OTA indicator file
  const char kJemaOSOTAIndicatorFile[] = "/mnt/stateful_partition/unencrypted/preserve/.disable_jemaos_ota";

  // Command to enable OTA updates
  std::string EnableOTACommand() {
    return std::string("rm ") + kJemaOSOTAIndicatorFile;
  }

  // Command to disable OTA updates
  std::string DisableOTACommand() {
    return std::string("touch ") + kJemaOSOTAIndicatorFile;
  }

  // Callback for when the OTA toggle command finishes
  void OnSetCommandFinished(const bool enabled, base::OnceCallback<void()> callback, absl::optional<ShellState> state) {
    if (!state || state->code != 0) {
      LOG(ERROR) << "Set JemaOS OTA " << (enabled ? "enabled" : "disabled") << " failed";
    }
    std::move(callback).Run();
  }

}  // namespace

// Enables or disables JemaOS OTA updates
void EnableJemaOTA(const bool enabled, base::OnceCallback<void()> callback) {
  JemaOSShellClient* shellClient = GetShellClient();
  if (!shellClient) return;
  const std::string command = enabled ? EnableOTACommand() : DisableOTACommand();
  shellClient->SyncExec(command,
      base::BindOnce(&OnSetCommandFinished, enabled, std::move(callback)));
}

// Checks if JemaOS OTA updates are enabled
bool GetEnabledJemaOTA() {
  const bool enabled = !base::PathExists(base::FilePath(kJemaOSOTAIndicatorFile));
  return enabled;
}

}  // namespace misc
}  // namespace jemaos