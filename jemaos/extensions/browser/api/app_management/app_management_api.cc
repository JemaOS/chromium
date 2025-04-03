// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/extensions/browser/api/app_management/app_management_api.h"

#include <stddef.h>
#include <algorithm>
#include <limits>
#include <utility>
#include <vector>

#include "base/values.h"
#include "chrome/browser/ash/app_list/arc/arc_app_list_prefs.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/extension_set.h"
#include "jemaos/extensions/common/api/app_management.h"
#include "chrome/browser/ash/arc/arc_util.h"
#include "chrome/browser/policy/chrome_policy_conversions_client.h"
#include "components/policy/core/browser/policy_conversions.h"

namespace extensions {
const char kAppIdName[] = "appId";
const char kAppTypeName[] = "appType";
// const char kPackageName[] = "package";
const char kWebApp[] = "web_app";
const char kArcApp[] = "arc_app";

namespace {

// Retrieves the list of ARC app IDs
std::vector<std::string> GetArcAppIdList(content::BrowserContext* context) {
  return ArcAppListPrefs::Get(context)->GetAppIds();
}

// Retrieves the set of extension IDs
ExtensionIdSet GetExtensionIdSet(content::BrowserContext* context) {
  return extensions::ExtensionRegistry::Get(context)
      ->enabled_extensions()
      .GetIDs();
}

// Creates a dictionary representing a web app
base::Value CreateWebAppInfo(std::string app_id) {
  base::Value result(base::Value::Type::DICT);
  result.SetStringKey(kAppIdName, app_id);
  result.SetStringKey(kAppTypeName, kWebApp);
  return result;
}

// Creates a dictionary representing an ARC app
base::Value CreateArcAppInfo(std::string app_id) {
  base::Value result(base::Value::Type::DICT);
  result.SetStringKey(kAppIdName, app_id);
  result.SetStringKey(kAppTypeName, kArcApp);
  return result;
}

}  // namespace

// Handles the "GetAppList" function
ExtensionFunction::ResponseAction AppManagementGetAppListFunction::Run() {
  Profile* profile = Profile::FromBrowserContext(browser_context());
  base::Value::List app_list;
  for (std::string app_id : GetExtensionIdSet(browser_context()))
    app_list.Append(CreateWebAppInfo(app_id));
  if (arc::IsArcAllowedForProfile(profile))
    for (std::string app_id : GetArcAppIdList(browser_context()))
      app_list.Append(CreateArcAppInfo(app_id));
  return RespondNow(WithArguments(std::move(app_list)));
}

// Handles the "InstallWebApp" function
ExtensionFunction::ResponseAction AppManagementInstallWebAppFunction::Run() {
  return RespondNow(NoArguments());
}

// Handles the "InstallArcApp" function
ExtensionFunction::ResponseAction AppManagementInstallArcAppFunction::Run() {
  return RespondNow(NoArguments());
}

// Handles the "GetArcPolicy" function
ExtensionFunction::ResponseAction AppManagementGetArcPolicyFunction::Run() {
  auto client = std::make_unique<policy::ChromePolicyConversionsClient>(
      browser_context());
  base::Value::Dict dict =
      policy::DictionaryPolicyConversions(std::move(client)).ToValueDict();
  const base::Value* arc_policy =
      dict.FindByDottedPath("chromePolicies.ArcPolicy.value");
  std::string policy_value = "";
  if (arc_policy) {
    policy_value = arc_policy->GetString();
  }
  return RespondNow(WithArguments(base::Value(policy_value)));
}

}  // namespace extensions