// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_EXTENSIONS_API_PROVIDER_H_
#define JEMAOS_EXTENSIONS_API_PROVIDER_H_

#include "jemaos/extensions/common/jemaos_api_permissions.h"
#include "extensions/common/extensions_api_provider.h"

namespace extensions {

// Provides API features, permissions, and schemas for JemaOS extensions.
class JemaOSExtensionsAPIProvider : public ExtensionsAPIProvider {
 public:
  // Constructor
  JemaOSExtensionsAPIProvider();

  // Destructor
  ~JemaOSExtensionsAPIProvider() override;

  // Adds API features to the feature provider
  void AddAPIFeatures(FeatureProvider* provider) override;

  // Adds manifest features to the feature provider
  void AddManifestFeatures(FeatureProvider* provider) override;

  // Adds permission features to the feature provider
  void AddPermissionFeatures(FeatureProvider* provider) override;

  // Adds behavior features to the feature provider
  void AddBehaviorFeatures(FeatureProvider* provider) override;

  // Adds API JSON sources to the JSON feature provider
  void AddAPIJSONSources(JSONFeatureProviderSource* json_source) override;

  // Checks if an API schema is generated
  bool IsAPISchemaGenerated(const std::string& name) override;

  // Retrieves the API schema for a given name
  base::StringPiece GetAPISchema(const std::string& name) override;

  // Registers permissions with the PermissionsInfo global
  void RegisterPermissions(PermissionsInfo* permissions_info) override;

  // Registers manifest handlers
  void RegisterManifestHandlers() override;
};

}  // namespace extensions

#endif  // JEMAOS_EXTENSIONS_API_PROVIDER_H_