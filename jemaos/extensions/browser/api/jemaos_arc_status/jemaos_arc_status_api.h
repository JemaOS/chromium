// Copyright 2025 Jema Technology. All rights reserved.

#ifndef _JEMAOS_EXTENSIONS_JEMAOS_ARC_STATUS_JEMAOS_ARC_STATUS_API_H_
#define _JEMAOS_EXTENSIONS_JEMAOS_ARC_STATUS_JEMAOS_ARC_STATUS_API_H_

#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"
#include "chrome/browser/ash/arc/process/arc_process_service.h"

namespace extensions {

namespace {

  // Alias for the optional list of ARC processes
  using OptionalArcProcessList = arc::ArcProcessService::OptionalArcProcessList;

  // Represents the status of ARC
  struct ArcStatus {
    bool supported = false;   // Whether ARC is supported
    bool archero = false;     // Whether ARC Hero is enabled
    bool running = false;     // Whether ARC is currently running
    bool installable = false; // Whether ARC is installable
  };

  // Callback type for ARC status retrieval
  using StatusCallback = base::OnceCallback<void(ArcStatus)>;

  // Helper class for retrieving ARC status
  class ArcStatusHelper {
    public:
    ArcStatusHelper() = default;
    ~ArcStatusHelper() = default;

    // Retrieves the ARC status
    void GetStatus(StatusCallback callback);

    private:
    // Retrieves the list of system processes
    void GetSystemProcessList();

    // Processes the list of system processes
    void OnGetSystemProcessList(OptionalArcProcessList process_list);

    // Retrieves the list of app processes
    void GetAppProcessList();

    // Processes the list of app processes
    void OnGetAppProcessList(OptionalArcProcessList process_list);

    // Finalizes the ARC status retrieval and invokes the callback
    void Final();

    StatusCallback callback_; // Callback to invoke with the ARC status
    ArcStatus status_;        // Current ARC status

    base::WeakPtrFactory<ArcStatusHelper> weak_factory_{this}; // Weak pointer factory
  };

}  // namespace

  // Handles the "Get" function for ARC status
  class JemaosArcStatusGetFunction: public ExtensionFunction {
    ~JemaosArcStatusGetFunction() override {}
    ResponseAction Run() override;

    // Callback for when ARC status is retrieved
    void OnGetStatus(ArcStatus status);

    DECLARE_EXTENSION_FUNCTION("jemaosArcStatus.get", JEMAOS_ARC_STATUS_GET)
  };

}  // namespace extensions

#endif  // _JEMAOS_EXTENSIONS_JEMAOS_ARC_STATUS_JEMAOS_ARC_STATUS_API_H_