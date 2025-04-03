#ifndef JEMAOS_DBUS_SHELL_CLIENT_H_
#define JEMAOS_DBUS_SHELL_CLIENT_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "base/functional/callback.h"
#include "base/observer_list.h"
#include "chromeos/chromeos_export.h"
#include "chromeos/dbus/common/dbus_client.h"
#include "chromeos/dbus/common/dbus_method_call_status.h"
#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/shell_state.h"

namespace jemaos {
namespace ash {

// JemaOSShellClient provides an interface for interacting with the JemaOS Shell service
class COMPONENT_EXPORT(ASH_DBUS_JEMAOS_SHELL_CLIENT) JemaOSShellClient
    : public chromeos::DBusClient {
 public:
    // Observer class for receiving notifications
    class Observer {
     public:
       virtual ~Observer() {}
       // Called when a system notification is received
       virtual void OnSystemNotificationReceived(int32_t level,
            const std::string& msg) {}
       // Called when a command notification is received
       virtual void OnCommandNotificationReceived(int32_t handler, int32_t state,
            const std::string& msg) {}
       // Called when a custom notification is received
       virtual void OnCustomNotificationReceived(int32_t data, int32_t exdata,
            const std::string& extra) {}
    };

    // Adds an observer
    virtual void AddObserver(Observer* observer) = 0;

    // Removes an observer
    virtual void RemoveObserver(Observer* observer) = 0;

    // Checks if an observer exists
    virtual bool HasObserver(const Observer* observer) const = 0;

    // Executes a synchronous shell command
    virtual void SyncExec(const std::string& cmd, chromeos::DBusMethodCallback<ShellState> callback) = 0;

    // Executes an asynchronous shell command
    virtual void AsyncExec(const std::string& cmd, chromeos::DBusMethodCallback<ShellState> callback) = 0;

    // Retrieves the output of a task
    virtual void GetTaskOutput(int32_t key, int32_t lines, chromeos::DBusMethodCallback<ShellState> callback) = 0;

    // Retrieves the state of a task
    virtual void GetTaskState(int32_t key, chromeos::DBusMethodCallback<ShellState> callback) = 0;

    // Forces the closure of a task
    virtual void ForceCloseTask(int32_t key, chromeos::DBusMethodCallback<ShellState> callback) = 0;

    // Retrieves the state of the daemon
    virtual void GetDaemonState(chromeos::DBusMethodCallback<ShellState> callback) = 0;

    // Creates and initializes the global instance. |bus| must not be null.
    static void Initialize(dbus::Bus* bus);

    // Creates and initializes a fake global instance.
    static void InitializeFake();

    // Destroys the global instance if it has been initialized.
    static void Shutdown();

    // Returns the global instance if initialized. May return null.
    static JemaOSShellClient* Get();

    JemaOSShellClient(const JemaOSShellClient&) = delete;
    JemaOSShellClient& operator=(const JemaOSShellClient&) = delete;

 protected:
    // Constructor
    JemaOSShellClient();

    // Destructor
    ~JemaOSShellClient() override;
};

}  // namespace ash
}  // namespace jemaos

#endif  // JEMAOS_DBUS_SHELL_CLIENT_H_