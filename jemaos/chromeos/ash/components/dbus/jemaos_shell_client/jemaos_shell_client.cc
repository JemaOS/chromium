// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/jemaos_shell_client.h"

#include <stdint.h>
#include <utility>

#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/format_macros.h"
#include "base/strings/stringprintf.h"
#include "dbus/bus.h"
#include "dbus/message.h"
#include "dbus/object_path.h"
#include "dbus/object_proxy.h"

namespace jemaos {
namespace ash {

namespace {

// Constants for JemaOS Shell service
const char kJemaOSShellServiceName[] = "io.jemaos.ShellDaemon";
const char kJemaOSShellServicePath[] = "/io/jemaos/ShellDaemon";
const char kJemaOSShellServiceInterface[] = "io.jemaos.ShellInterface";
const char kShellSyncExec[] = "SyncExec";
const char kShellAsyncExec[] = "AsyncExec";
const char kShellGetTaskOutput[] = "GetAsyncTaskOutput";
const char kShellGetTaskState[] = "GetTaskState";
const char kShellForceCloseTask[] = "ForceCloseTask";
const char kShellGetDaemonState[] = "GetDaemonState";
const char kShellSystemNotify[] = "ShellNotifying";

// Notification types
enum NotificationType { SYSTEM, COMMAND };

JemaOSShellClient* g_instance = nullptr;

// Implementation of JemaOSShellClient
class JemaOSShellClientImpl : public JemaOSShellClient {
 public:
  // Constructor
  JemaOSShellClientImpl() : shell_proxy_(nullptr) {}

  // Deleted copy assignment operator
  JemaOSShellClientImpl& operator=(const JemaOSShellClientImpl&) = delete;

  // Destructor
  ~JemaOSShellClientImpl() override = default;

  // Executes a synchronous shell command
  void SyncExec(const std::string& cmd, chromeos::DBusMethodCallback<ShellState> callback) override {
    VLOG(1) << "SyncExec received command:" << cmd;
    dbus::MethodCall method_call(kJemaOSShellServiceInterface, kShellSyncExec);
    dbus::MessageWriter writer(&method_call);
    writer.AppendString(cmd);
    shell_proxy_->CallMethod(&method_call,
        dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
        base::BindOnce(&JemaOSShellClientImpl::OnShellDaemonReturn,
          weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
  }

  // Executes an asynchronous shell command
  void AsyncExec(const std::string& cmd, chromeos::DBusMethodCallback<ShellState> callback) override {
    VLOG(1) << "AsyncExec received command:" << cmd;
    dbus::MethodCall method_call(kJemaOSShellServiceInterface, kShellAsyncExec);
    dbus::MessageWriter writer(&method_call);
    writer.AppendString(cmd);
    shell_proxy_->CallMethod(&method_call,
        dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
        base::BindOnce(&JemaOSShellClientImpl::OnShellDaemonReturn,
          weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
  }

  // Retrieves the output of a task
  void GetTaskOutput(int32_t key, int32_t lines, chromeos::DBusMethodCallback<ShellState> callback) override {
    dbus::MethodCall method_call(kJemaOSShellServiceInterface, kShellGetTaskOutput);
    dbus::MessageWriter writer(&method_call);
    writer.AppendInt32(key);
    writer.AppendInt32(lines);
    shell_proxy_->CallMethod(&method_call,
        dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
        base::BindOnce(&JemaOSShellClientImpl::OnShellDaemonReturn,
          weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
  }

  // Retrieves the state of a task
  void GetTaskState(int32_t key, chromeos::DBusMethodCallback<ShellState> callback) override {
    dbus::MethodCall method_call(kJemaOSShellServiceInterface, kShellGetTaskState);
    dbus::MessageWriter writer(&method_call);
    writer.AppendInt32(key);
    shell_proxy_->CallMethod(&method_call,
        dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
        base::BindOnce(&JemaOSShellClientImpl::OnShellDaemonReturn,
          weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
  }

  // Forces the closure of a task
  void ForceCloseTask(int32_t key, chromeos::DBusMethodCallback<ShellState> callback) override {
    dbus::MethodCall method_call(kJemaOSShellServiceInterface, kShellForceCloseTask);
    dbus::MessageWriter writer(&method_call);
    writer.AppendInt32(key);
    shell_proxy_->CallMethod(&method_call,
        dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
        base::BindOnce(&JemaOSShellClientImpl::OnShellDaemonReturn,
          weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
  }

  // Retrieves the state of the daemon
  void GetDaemonState(chromeos::DBusMethodCallback<ShellState> callback) override {
    dbus::MethodCall method_call(kJemaOSShellServiceInterface, kShellGetDaemonState);
    shell_proxy_->CallMethod(&method_call,
        dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
        base::BindOnce(&JemaOSShellClientImpl::OnShellDaemonReturn,
          weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
  }

  // Adds an observer
  void AddObserver(Observer* observer) override {
    observers_.AddObserver(observer);
  }

  // Removes an observer
  void RemoveObserver(Observer* observer) override {
    observers_.RemoveObserver(observer);
  }

  // Checks if an observer exists
  bool HasObserver(const Observer* observer) const override {
    return observers_.HasObserver(observer);
  }

  // Initializes the client with a D-Bus connection
  void Init(dbus::Bus* bus) override {
    shell_proxy_ = bus->GetObjectProxy(kJemaOSShellServiceName,
        dbus::ObjectPath(kJemaOSShellServicePath));
    shell_proxy_->ConnectToSignal(
      kJemaOSShellServiceInterface, kShellSystemNotify,
      base::BindRepeating(&JemaOSShellClientImpl::OnShellSignalReceived,
        weak_ptr_factory_.GetWeakPtr()),
      base::BindOnce(&JemaOSShellClientImpl::SignalConnected,
        weak_ptr_factory_.GetWeakPtr())
    );
    VLOG(1) << "Init shell_proxy";
  }

 private:
  // Handles signal connection success or failure
  void SignalConnected(const std::string& interface_name,
      const std::string& signal_name, bool success) {
    if (!success)
      LOG(ERROR) << "Failed to connect to signal " << signal_name << ".";
    else
      VLOG(1) << "Successfully connected to signal " << signal_name;
  }

  // Handles received signals
  void OnShellSignalReceived(dbus::Signal* signal) {
    int32_t type, handler, state;
    std::string msg;
    dbus::MessageReader reader(signal);
    if (!reader.PopInt32(&type) || !reader.PopInt32(&handler) ||
        !reader.PopInt32(&state) || !reader.PopString(&msg)) {
      LOG(ERROR) << "Error reading signal: " << signal->ToString();
      return;
    }
    VLOG(1) << "Received notification type:" << type << " handler:" << handler
            << " state:" << state << " msg:" << msg;
    switch (type) {
      case NotificationType::SYSTEM:
        for (auto& observer : observers_)
          observer.OnSystemNotificationReceived(state, std::move(msg));
        break;
      case NotificationType::COMMAND:
        for (auto& observer : observers_)
          observer.OnCommandNotificationReceived(handler, state, std::move(msg));
        break;
      default:
        for (auto& observer : observers_)
          observer.OnCustomNotificationReceived(handler, state, std::move(msg));
        break;
    }
  }

  // Handles the return from the shell daemon
  void OnShellDaemonReturn(chromeos::DBusMethodCallback<ShellState> callback,
      dbus::Response* response) {
    if (!response) {
      LOG(ERROR) << "Error calling JemaOS shell daemon";
      std::move(callback).Run(absl::nullopt);
      return;
    }
    ShellState shell_state;
    dbus::MessageReader reader(response);
    dbus::MessageReader sub_reader(nullptr);
    if (!reader.PopStruct(&sub_reader) || !sub_reader.PopInt32(&shell_state.code) ||
        !sub_reader.PopString(&shell_state.result)) {
      LOG(ERROR) << "Error reading response from JemaOS shell daemon: "
                 << response->ToString();
      std::move(callback).Run(absl::nullopt);
      return;
    }
    VLOG(1) << "Return code:" << shell_state.code;
    std::move(callback).Run(std::move(shell_state));
  }

  dbus::ObjectProxy* shell_proxy_;
  base::ObserverList<Observer>::Unchecked observers_;
  base::WeakPtrFactory<JemaOSShellClientImpl> weak_ptr_factory_{this};
};

}  // namespace

// static
JemaOSShellClient* JemaOSShellClient::Get() {
  return g_instance;
}

// static
void JemaOSShellClient::Initialize(dbus::Bus* bus) {
  CHECK(bus);
  (new JemaOSShellClientImpl())->Init(bus);
}

// static
void JemaOSShellClient::Shutdown() {
  CHECK(g_instance);
  delete g_instance;
}

// Constructor
JemaOSShellClient::JemaOSShellClient() {
  CHECK(!g_instance);
  g_instance = this;
}

// Destructor
JemaOSShellClient::~JemaOSShellClient() {
  CHECK_EQ(g_instance, this);
  g_instance = nullptr;
}

}  // namespace ash
}  // namespace jemaos