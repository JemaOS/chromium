#ifndef JEMAOS_EXTENSIONS_API_SHELL_CLIENT_API_H_
#define JEMAOS_EXTENSIONS_API_SHELL_CLIENT_API_H_

#include <string>
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "extensions/browser/browser_context_keyed_api_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"
#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/jemaos_shell_client.h"
#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/shell_state.h"

using ShellState = jemaos::ash::ShellState;
using JemaOSShellClient = jemaos::ash::JemaOSShellClient;

namespace extensions {

// Event router for handling ShellClient events
class ShellClientEventRouter : public JemaOSShellClient::Observer {
 public:
  explicit ShellClientEventRouter(Profile* profile);
  ~ShellClientEventRouter() override;

  // Adds an extension ID to the event router
  void AddExtensionId(const std::string& extension_id, const std::string& event_name);

  // Removes an extension ID from the event router
  void RemoveExtensionId(const std::string& extension_id, const std::string& event_name);

 private:
  // Handles system notifications
  void OnSystemNotificationReceived(int32_t level, const std::string& msg) override;

  // Handles command notifications
  void OnCommandNotificationReceived(int32_t handler, int32_t state,
                                      const std::string& msg) override;

  // Handles custom notifications
  void OnCustomNotificationReceived(int32_t data, int32_t exdata,
                                     const std::string& extra) override;

  base::ScopedObservation<JemaOSShellClient, JemaOSShellClient::Observer>
      jemaos_client_observer_;
  Profile* profile_;
  EventRouter* event_router_;
  std::set<std::string> system_extension_ids_;
  std::set<std::string> command_extension_ids_;
  std::set<std::string> custom_extension_ids_;
  base::WeakPtrFactory<ShellClientEventRouter> weak_factory_;
};

// API for managing ShellClient functionality
class ShellClientAPI : public BrowserContextKeyedAPI,
                       public EventRouter::Observer {
 public:
  static BrowserContextKeyedAPIFactory<ShellClientAPI>* GetFactoryInstance();
  static ShellClientAPI* Get(content::BrowserContext* context);
  explicit ShellClientAPI(content::BrowserContext* context);
  ~ShellClientAPI() override;

  // Shuts down the API
  void Shutdown() override;

  // EventRouter::Observer implementation
  void OnListenerAdded(const EventListenerInfo& details) override;
  void OnListenerRemoved(const EventListenerInfo& details) override;

 private:
  friend class BrowserContextKeyedAPIFactory<ShellClientAPI>;

  // BrowserContextKeyedAPI implementation
  static const char* service_name() { return "ShellClientAPI"; }
  static const bool kServiceRedirectedInIncognito = true;
  static const bool kServiceIsNULLWhileTesting = true;

  // Registers notifications for the API
  void RegisterNotifications();

  Profile* profile_;
  std::unique_ptr<ShellClientEventRouter> shell_client_event_router_;
  base::WeakPtrFactory<ShellClientAPI> weak_factory_;
};

// Function for executing synchronous shell commands
class ShellClientExecSyncFunction : public ExtensionFunction {
  ~ShellClientExecSyncFunction() override {}
  ResponseAction Run() override;
  void StateCallback(absl::optional<ShellState> state);
  DECLARE_EXTENSION_FUNCTION("shellClient.execSync", SHELL_SYNC_EXEC)
};

// Function for executing asynchronous shell commands
class ShellClientExecAsyncFunction : public ExtensionFunction {
  ~ShellClientExecAsyncFunction() override {}
  ResponseAction Run() override;
  void StateCallback(absl::optional<ShellState> state);
  DECLARE_EXTENSION_FUNCTION("shellClient.execAsync", SHELL_ASYNC_EXEC)
};

// Function for retrieving the state of a task
class ShellClientGetTaskStateFunction : public ExtensionFunction {
  ~ShellClientGetTaskStateFunction() override {}
  ResponseAction Run() override;
  void StateCallback(absl::optional<ShellState> state);
  DECLARE_EXTENSION_FUNCTION("shellClient.getTaskState", SHELL_GET_TASK_STATE)
};

// Function for forcing the closure of a task
class ShellClientForceCloseTaskFunction : public ExtensionFunction {
  ~ShellClientForceCloseTaskFunction() override {}
  ResponseAction Run() override;
  void StateCallback(absl::optional<ShellState> state);
  DECLARE_EXTENSION_FUNCTION("shellClient.forceCloseTask", SHELL_FORCE_CLOSE_TASK)
};

// Function for retrieving the output of a task
class ShellClientGetTaskOutputFunction : public ExtensionFunction {
  ~ShellClientGetTaskOutputFunction() override {}
  ResponseAction Run() override;
  void StateCallback(absl::optional<ShellState> state);
  DECLARE_EXTENSION_FUNCTION("shellClient.getTaskOutput", SHELL_GET_TASK_OUTPUT)
};

// Function for retrieving the state of the daemon
class ShellClientGetDaemonStateFunction : public ExtensionFunction {
  ~ShellClientGetDaemonStateFunction() override {}
  ResponseAction Run() override;
  void StateCallback(absl::optional<ShellState> state);
  DECLARE_EXTENSION_FUNCTION("shellClient.getDaemonState", SHELL_GET_DAEMON_STATE)
};

}  // namespace extensions

#endif  // JEMAOS_EXTENSIONS_API_SHELL_CLIENT_API_H_