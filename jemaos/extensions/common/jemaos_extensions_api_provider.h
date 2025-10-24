// Copyright 2018 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_EXTENSIONS_API_PROVIDER_H_
#define JEMAOS_EXTENSIONS_API_PROVIDER_H_

#include "jemaos/extensions/common/jemaos_api_permissions.h"
#include "extensions/common/extensions_api_provider.h"

namespace extensions {

class JemaOSExtensionsAPIProvider : public ExtensionsAPIProvider {
 public:
  JemaOSExtensionsAPIProvider();
  ~JemaOSExtensionsAPIProvider() override;

  // ExtensionsAPIProvider:
  void AddAPIFeatures(FeatureProvider* provider) override;
  void AddManifestFeatures(FeatureProvider* provider) override;
  void AddPermissionFeatures(FeatureProvider* provider) override;
  void AddBehaviorFeatures(FeatureProvider* provider) override;
  void AddAPIJSONSources(JSONFeatureProviderSource* json_source) override;
  bool IsAPISchemaGenerated(const std::string& name) override;
  std::string_view GetAPISchema(const std::string& name) override;
  void RegisterPermissions(PermissionsInfo* permissions_info) override;
  void RegisterManifestHandlers() override;
};

}  // namespace extensions

#endif  // JEMAOS_EXTENSIONS_API_PROVIDER_H_
