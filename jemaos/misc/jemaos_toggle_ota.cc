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
  JemaOSShellClient* GetShellClient() {
    return JemaOSShellClient::Get();
  }

  const char kJemaOSOTAIndicatorFile[] = "/mnt/stateful_partition/unencrypted/preserve/.disable_jemaos_ota";

  std::string EnableOTACommand() {
    return std::string("rm ") + kJemaOSOTAIndicatorFile;
  }
  std::string DisableOTACommand() {
    return std::string("touch ") + kJemaOSOTAIndicatorFile;
  }

  void OnSetCommandFinished(const bool enabled, base::OnceCallback<void()> callback, absl::optional<ShellState> state) {
    if (!state || state->code != 0) {
      LOG(ERROR) << "Set JemaOS OTA " << enabled << " failed";
    }
    std::move(callback).Run();
  }
}

void EnableJemaOTA(const bool enabled, base::OnceCallback<void()> callback) {
  JemaOSShellClient* shellClient = GetShellClient();
  if (!shellClient) return;
  const std::string command = enabled ? EnableOTACommand() : DisableOTACommand();
  shellClient->SyncExec(command,
      base::BindOnce(&OnSetCommandFinished, enabled, std::move(callback)));
}

bool GetEnabledJemaOTA() {
  const bool enabled = !base::PathExists(base::FilePath(kJemaOSOTAIndicatorFile));
  return enabled;
}

} // misc
} // jemaos
