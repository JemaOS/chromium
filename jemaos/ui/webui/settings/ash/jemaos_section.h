// Copyright (c) 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_JEMAOS_SECTION_H_
#define CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_JEMAOS_SECTION_H_

#include <string>
#include "base/values.h"
#include "chrome/browser/ui/webui/settings/ash/os_settings_section.h"
#include "chrome/browser/ui/webui/settings/ash/search/search_tag_registry.h"

class PrefService;
class Profile;

namespace ash::settings {

// Represents the JemaOS section in the settings UI
// NOTE FOR DEVELOPERS: This class handles the registration of search tags,
// localized strings, and WebUI message handlers for the JemaOS section.
class JemaOsSection : public OsSettingsSection {
 public:
  // Constructor
  // NOTE FOR DEVELOPERS: Initializes the section with the given profile,
  // search tag registry, and preference service.
  JemaOsSection(Profile* profile, SearchTagRegistry* search_tag_registry,
                PrefService* pref_service);

  // Destructor
  // NOTE FOR DEVELOPERS: Cleans up resources if necessary.
  ~JemaOsSection() override;

 private:
  // Adds localized strings and data to the WebUI data source
  // NOTE FOR DEVELOPERS: Add new localized strings here as needed.
  void AddLoadTimeData(content::WebUIDataSource* html_source) override;

  // Adds message handlers to the WebUI
  // NOTE FOR DEVELOPERS: Register new WebUI message handlers here.
  void AddHandlers(content::WebUI* web_ui) override;

  // Retrieves the section name message ID
  // NOTE FOR DEVELOPERS: Update this method if the section name changes.
  int GetSectionNameMessageId() const override;

  // Retrieves the section type
  // NOTE FOR DEVELOPERS: Update this method if the section type changes.
  chromeos::settings::mojom::Section GetSection() const override;

  // Retrieves the section icon
  // NOTE FOR DEVELOPERS: Update this method if the section icon changes.
  ash::settings::mojom::SearchResultIcon GetSectionIcon() const override;

  // Retrieves the section path
  // NOTE FOR DEVELOPERS: Update this method if the section path changes.
  std::string GetSectionPath() const override;

  // Logs metrics for settings changes
  // NOTE FOR DEVELOPERS: Implement this method to log metrics for specific settings.
  bool LogMetric(chromeos::settings::mojom::Setting setting,
                 base::Value& value) const override;

  // Registers the hierarchy for the JemaOS section
  // NOTE FOR DEVELOPERS: Update this method if the hierarchy changes.
  void RegisterHierarchy(HierarchyGenerator* generator) const override;

  // Member variables
  PrefService* pref_service_;  // Preference service for managing settings
};

}  // namespace ash::settings

#endif  // CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_JEMAOS_SECTION_H_