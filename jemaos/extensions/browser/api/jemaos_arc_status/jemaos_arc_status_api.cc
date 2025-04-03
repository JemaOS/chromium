// Copyright 2025 Jema Technology. All rights reserved.

#include "jemaos/extensions/browser/api/jemaos_arc_status/jemaos_arc_status_api.h"
#include "base/files/file_util.h"

namespace extensions {

namespace {

// Base path for ARC image files
const char kArcImageBasePath[] = "/opt/google/containers/android";
// Names of ARC system and vendor image files
const char kArcSystemImageName[] = "system.raw.img";
const char kArcVendorImageName[] = "vendor.raw.img";

// Process names for ARC status checks
const char kArcRunningProcessName[] = "org.chromium.arc.home";
const char kArcInstallableProcessName[] = "/system/bin/arc_jemaos_service";

// Keys for ARC status dictionary
const char kArcSupportedKey[] = "supported";
const char kArcHeroKey[] = "archero";
const char kArcRunningKey[] = "running";
const char kArcInstallableKey[] = "installable";

// Prefix for ARC Hero process names
const char kArcHeroProcessSuffix[] = "com.jemaos.archero.";

// Checks if ARC is supported by verifying the existence of required image files
bool GetArcSupported() {
  bool system_img_exists = base::PathExists(
      base::FilePath(kArcImageBasePath).Append(kArcSystemImageName));
  bool vendor_img_exists = base::PathExists(
      base::FilePath(kArcImageBasePath).Append(kArcVendorImageName));
  return system_img_exists && vendor_img_exists;
}

// Retrieves the ARC status
void ArcStatusHelper::GetStatus(StatusCallback callback) {
  callback_ = std::move(callback);

  status_.archero = false;
  status_.installable = false;
  status_.running = false;
  status_.supported = GetArcSupported();

  GetSystemProcessList();
}

// Retrieves the list of system processes
void ArcStatusHelper::GetSystemProcessList() {
  arc::ArcProcessService* arc_process_service = arc::ArcProcessService::Get();
  if (!arc_process_service) {
    Final();
    return;
  }
  arc_process_service->RequestSystemProcessList(
      base::BindOnce(&ArcStatusHelper::OnGetSystemProcessList, weak_factory_.GetWeakPtr()));
}

// Processes the list of system processes
void ArcStatusHelper::OnGetSystemProcessList(
    OptionalArcProcessList processes) {
  if (!processes) {
    Final();
    return;
  }
  for (auto& entry : *processes) {
    if (entry.process_name() == kArcRunningProcessName) {
      status_.running = true;
    } else if (entry.process_name() == kArcInstallableProcessName) {
      status_.installable = true;
    } else if (base::StartsWith(entry.process_name(), kArcHeroProcessSuffix)) {
      // Special case for ARC Hero
      status_.archero = true;
      status_.running = true;
      status_.supported = true;
      status_.installable = true;
      break;
    }
    if (status_.installable && status_.running) {
      break;
    }
  }

  GetAppProcessList();
}

// Retrieves the list of app processes
void ArcStatusHelper::GetAppProcessList() {
  arc::ArcProcessService* arc_process_service = arc::ArcProcessService::Get();
  if (!arc_process_service) {
    Final();
    return;
  }
  arc_process_service->RequestAppProcessList(
      base::BindOnce(&ArcStatusHelper::OnGetAppProcessList, weak_factory_.GetWeakPtr()));
}

// Processes the list of app processes
void ArcStatusHelper::OnGetAppProcessList(
    OptionalArcProcessList processes) {
  if (!processes) {
    Final();
    return;
  }
  for (auto& entry : *processes) {
    // Check only for the ARC running process
    if (entry.process_name() == kArcRunningProcessName) {
      status_.running = true;
      break;
    }
  }
  Final();
}

// Finalizes the ARC status retrieval and invokes the callback
void ArcStatusHelper::Final() {
  if (callback_) {
    std::move(callback_).Run(status_);
  }
}

// Global helper instance
ArcStatusHelper *helper = nullptr;

}  // namespace

// Handles the "Get" function for ARC status
ExtensionFunction::ResponseAction JemaosArcStatusGetFunction::Run() {
  if (!helper) {
    helper = new ArcStatusHelper();
  }
  helper->GetStatus(base::BindOnce(
      &JemaosArcStatusGetFunction::OnGetStatus, this));

  return RespondLater();
}

// Callback for when ARC status is retrieved
void JemaosArcStatusGetFunction::OnGetStatus(ArcStatus status) {
  base::Value result(base::Value::Type::DICT);
  result.SetBoolKey(kArcSupportedKey, status.supported);
  result.SetBoolKey(kArcHeroKey, status.archero);
  result.SetBoolKey(kArcRunningKey, status.running);
  result.SetBoolKey(kArcInstallableKey, status.installable);
  Respond(WithArguments(std::move(result)));
}

}  // namespace extensions