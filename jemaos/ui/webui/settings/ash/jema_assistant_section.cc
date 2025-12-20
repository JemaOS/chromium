// Copyright 2024 The Jema Technology Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "jemaos/ui/webui/settings/ash/jema_assistant_section.h"

#include "ash/constants/ash_features.h"
#include "base/no_destructor.h"
#include "chrome/browser/profiles/profile.h"
#include "components/prefs/pref_service.h"
#include "chrome/grit/generated_resources.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "jemaos/switches/misc/misc_switches.h"

namespace ash::settings {

namespace mojom {
using ::chromeos::settings::mojom::kJemaAssistantSectionPath;
using ::chromeos::settings::mojom::Section;
using ::chromeos::settings::mojom::Subpage;
using ::chromeos::settings::mojom::Setting;
} // namespace mojom

namespace {
  const std::vector<SearchConcept>& GetJemaAssistantSearchConcepts() {
    static const base::NoDestructor<std::vector<SearchConcept>> tags({
      {IDS_OS_SETTINGS_JEMA_ASSISTANT,
       mojom::kJemaAssistantSectionPath,
       mojom::SearchResultIcon::kAssistant,
       mojom::SearchResultDefaultRank::kMedium,
       mojom::SearchResultType::kSetting,
       {.setting = mojom::Setting::kJemaAssistantSettings}},
    });
    return *tags;
  }
}

JemaAssistantSection::JemaAssistantSection(Profile* profile,
                                           SearchTagRegistry* search_tag_registry,
                                           PrefService* pref_service)
  : OsSettingsSection(profile, search_tag_registry),
    pref_service_(pref_service) {
  SearchTagRegistry::ScopedTagUpdater updater = registry()->StartUpdate();
  updater.AddSearchTags(GetJemaAssistantSearchConcepts());
}

JemaAssistantSection::~JemaAssistantSection() = default;

void JemaAssistantSection::AddLoadTimeData(content::WebUIDataSource* html_source) {
  static constexpr webui::LocalizedString kLocalizedStrings[] = {
    {"jemaAssistantPageTitle", IDS_OS_SETTINGS_JEMA_ASSISTANT},
    {"jemaAssistantMenuItemDescription", IDS_OS_SETTINGS_JEMA_ASSISTANT_MENU_ITEM_DESCRIPTION},
    {"jemaAssistantToggleLabel", IDS_OS_SETTINGS_JEMA_ASSISTANT_TOGGLE_LABEL},
    {"jemaAssistantToggleSublabel", IDS_OS_SETTINGS_JEMA_ASSISTANT_TOGGLE_SUBLABEL},
    {"jemaAssistantBubbleShortcutToggleLabel", IDS_OS_SETTINGS_JEMA_ASSISTANT_BUBBLE_SHORTCUT_TOGGLE_LABEL},
    {"jemaAssistantBubbleShortcutToggleSublabel", IDS_OS_SETTINGS_JEMA_ASSISTANT_BUBBLE_SHORTCUT_TOGGLE_SUBLABEL},
    {"jemaAssistantSettingsInAppLabel", IDS_OS_SETTINGS_JEMA_ASSISTANT_SETTINGS_IN_APP_LABEL},
    {"jemaAssistantSettingsInAppSublabel", IDS_OS_SETTINGS_JEMA_ASSISTANT_SETTINGS_IN_APP_SUBLABEL},
  };
  html_source->AddLocalizedStrings(kLocalizedStrings);
  html_source->AddBoolean("jemaAssistantFeatureEnabled",ash::features::IsJemaAssistantEnabled());
  html_source->AddBoolean("isJemaAiHidden", jemaos::switches::IsJemaAiHidden());
}

void JemaAssistantSection::AddHandlers(content::WebUI* web_ui) {

}

int JemaAssistantSection::GetSectionNameMessageId() const {
  return IDS_OS_SETTINGS_JEMA_ASSISTANT;
}
chromeos::settings::mojom::Section JemaAssistantSection::GetSection() const {
  return mojom::Section::kJemaAssistant;
}

ash::settings::mojom::SearchResultIcon JemaAssistantSection::GetSectionIcon() const {
  return mojom::SearchResultIcon::kAssistant;
}

const char* JemaAssistantSection::GetSectionPath() const {
  return mojom::kJemaAssistantSectionPath;
}

bool JemaAssistantSection::LogMetric(chromeos::settings::mojom::Setting setting,
               base::Value& value) const {
  return false;
}

void JemaAssistantSection::RegisterHierarchy(HierarchyGenerator* generator) const {
  generator->RegisterTopLevelSetting(mojom::Setting::kJemaAssistantSettings);
}

} // namespace ash::settings
