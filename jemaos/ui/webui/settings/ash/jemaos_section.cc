// Copyright (c) 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/ui/webui/settings/ash/jemaos_section.h"
#include "base/no_destructor.h"
#include "chrome/grit/generated_resources.h"
#include "chrome/grit/chromium_strings.h"
#include "ui/base/l10n/l10n_util.h"
#include "content/public/browser/web_ui.h"
#include "chrome/browser/ui/webui/webui_util.h"
#include "ui/base/webui/web_ui_util.h"
#include "content/public/browser/web_ui_data_source.h"
#include "chrome/browser/profiles/profile.h"
#include "components/prefs/pref_service.h"
#include "base/system/sys_info.h"
#include "chrome/browser/browser_process.h"
#include "jemaos/switches/misc/misc_switches.h"
#include "jemaos/switches/urls/urls_constants.h"

#include "base/strings/utf_string_conversions.h"

#include "jemaos/ui/webui/settings/ash/jemaos_handler.h"
#include "jemaos/prefs/jemaos_pref_names.h"

namespace ash::settings {

namespace mojom {
using ::chromeos::settings::mojom::kJemaOsSectionPath;
using ::chromeos::settings::mojom::kJemaOsSubpagePath;
using ::chromeos::settings::mojom::Section;
using ::chromeos::settings::mojom::Subpage;
using ::chromeos::settings::mojom::Setting;
}

namespace {
  const std::vector<SearchConcept>& GetJemaOsSearchConcepts() {
    static const base::NoDestructor<std::vector<SearchConcept>> tags({
      {IDS_OS_SETTINGS_JEMAOS_SETTINGS,
       mojom::kJemaOsSubpagePath,
       mojom::SearchResultIcon::kChrome,
       mojom::SearchResultDefaultRank::kMedium,
       mojom::SearchResultType::kSubpage,
       {.subpage = mojom::Subpage::kJemaOsMain}},
    });
    return *tags;
  }
}

JemaOsSection::JemaOsSection(Profile* profile,
                           SearchTagRegistry* search_tag_registry,
                           PrefService* pref_service)
  : OsSettingsSection(profile, search_tag_registry),
    pref_service_(pref_service) {
  SearchTagRegistry::ScopedTagUpdater updater = registry()->StartUpdate();
  updater.AddSearchTags(GetJemaOsSearchConcepts());
}

JemaOsSection::~JemaOsSection() = default;

void JemaOsSection::AddLoadTimeData(content::WebUIDataSource* html_source) {
  static constexpr webui::LocalizedString kLocalizedStrings[] = {
    {"jemaosSettingsAccountTitle", IDS_OS_SETTINGS_JEMAOS_ACCOUNT_TITLE},
    {"jemaosSettingsWifiDriverTitle", IDS_OS_SETTINGS_JEMAOS_WIFI_DRIVER_TITLE},
    {"jemaosSettingsWifiDriverLabel",
      IDS_JEMAOS_ADVANCED_SETTING_WIFI_DRIVER_TITLE},
    {"jemaosSettingsWifiDriverDesc",
      IDS_JEMAOS_ADVANCED_SETTING_WIFI_DRIVER_HINT},
    {"jemaosSettingsWifiDriverNote",
      IDS_JEMAOS_ADVANCED_SETTING_WIFI_DRIVER_HINT2},
    {"jemaosSettingsTouchpadModeTitle",
      IDS_OS_SETTINGS_JEMAOS_TOUCHPAD_MODE_TITLE},
    {"jemaosSettingsTouchpadModeLabel",
      IDS_JEMAOS_ADVANCED_SETTING_TOUCHPAD_DRIVER_TITLE},
    {"jemaosSettingsTouchpadModeDesc",
      IDS_JEMAOS_ADVANCED_SETTING_TOUCHPAD_DRIVER_HINT},
    {"jemaosSettingsTouchpadModeNote",
      IDS_JEMAOS_ADVANCED_SETTING_TOUCHPAD_DRIVER_HINT1},
    {"jemaosSettingsRemoteAssistanceTitle",
      IDS_OS_SETTINGS_JEMAOS_REMOTE_ASSISTANCE_TITLE},
    {"jemaosSettingsRemoteHelperServiceTitle",
      IDS_OS_SETTINGS_JEMAOS_REMOTE_HELPER_SERVICE_TITLE},
    {"jemaosSettingsRemoteHelperDesc",
      IDS_OS_SETTINGS_JEMAOS_REMOTE_HELPER_SERVICE_DESC},
    {"jemaosSettingsRemoteHelperEnabledMessage",
      IDS_OS_SETTINGS_JEMAOS_REMOTE_HELPER_SERVICE_ENABLED_MESSAGE},
    {"jemaosSettingsRemoteHelperRequireRestartMessage",
      IDS_OS_SETTINGS_JEMAOS_REMOTE_HELPER_SERVICE_REQUIRE_RESTART_MESSAGE},
    {"jemaosSettingsRemoteHelperStartingMessage",
      IDS_OS_SETTINGS_JEMAOS_REMOTE_HELPER_SERVICE_STARGING_MESSAGE},
    {"jemaosSettingsMoreInfoTitle", IDS_OS_SETTINGS_JEMAOS_MORE_INFO_TITLE},

    {"jemaosSettingsOtherTweaksTitle",
      IDS_OS_SETTINGS_JEMAOS_OTHER_TWEAKS_TITLE},
    {"rebootButtonInTrayLabel",
      IDS_OS_SETTINGS_JEMAOS_REBOOT_BUTTON_IN_TRAY_LABEL},
    {"displayJemaOsRebootButtonInTray",
      IDS_OS_SETTINGS_JEMAOS_DISPLAY_REBOOT_BUTTON_IN_TRAY},
    {"rotateScreenButtonInTrayLabel",
      IDS_OS_SETTINGS_JEMAOS_ROTATE_SCREEN_BUTTON_IN_TRAY_LABEL},
    {"notTabletPhysicalStateDisableJemaOsRotateScreen",
      IDS_OS_SETTINGS_JEMAOS_NOT_TABLET_STATE_DISABLE_ROTATE_SCREEN},
    {"displayJemaOsRotateScreenButton",
      IDS_OS_SETTINGS_JEMAOS_DISPLAY_ROTATE_SCREEN_BUTTON},

    {"switchTabletLaptopModeButtonInTrayLabel",
      IDS_OS_SETTINGS_JEMAOS_SWITCH_TABLET_LAPTOP_MODE_BUTTON_IN_TRAY_LABEL},
    {"displaySwitchTabletLaptopModeButton",
      IDS_OS_SETTINGS_JEMAOS_DISPLAY_SWITCH_TABLET_LAPTOP_MODE_BUTTON},

    {"enableLibwidevineLabel", IDS_OS_SETTINGS_JEMAOS_ENABLE_LIBWIDEVINE_LABEL},
    {"failedEnableWidevineTitle",
      IDS_OS_SETTINGS_JEMAOS_FAILED_ENABLE_WIDEVINE_TITLE},
    {"failedEnableWidevineMessage",
      IDS_OS_SETTINGS_JEMAOS_FAILED_ENABLE_WIDEVINE_MESSAGE},

    {"jemaosExperimentalFeatures",
      IDS_OS_SETTINGS_JEMAOS_EXPERIMENTAL_FEATURES_TITLE},
    {"jemaosBypassTpmChecksTitle",
      IDS_OS_SETTINGS_JEMAOS_BYPASS_TPM_CHECKS_TITLE},
    {"jemaosBypassTpmChecksDesc",
      IDS_OS_SETTINGS_JEMAOS_BYPASS_TPM_CHECKS_DESC},

    {"autoSigninForJemaLocalAccountTitle",
      IDS_OS_SETTINGS_JEMAOS_AUTO_SIGNIN_FOR_LOCAL_ACCOUNT_TITLE},
    {"enableAutoSigninForJemaLocalAccountHelpMessage",
      IDS_OS_SETTINGS_JEMAOS_ENABLE_AUTO_SIGNIN_FOR_LOCAL_ACCOUNT_MESSAGE},
    {"autoSigninForJemaLocalAccountOtherUserAlreadyEnabled",
      IDS_OS_SETTINGS_JEMAOS_AUTO_SIGNIN_FOR_LOCAL_ACCOUNT_ALREADY_ENABLED_BY_OTHER},
    {"unableToSetAutoSigninForJemaLocalAccount",
      IDS_OS_SETTINGS_JEMAOS_UNABLE_TO_SET_AUTO_SIGNIN_FOR_LOCAL_ACCOUNT},
    {"unableToSetAutoSigninForJemaNonLocalAccount",
      IDS_OS_SETTINGS_JEMAOS_UNABLE_TO_SET_AUTO_SIGNIN_FOR_NON_LOCAL_ACCOUNT},

    {"jemaosSettingsBackupButtonLabel",
      IDS_OS_SETTINGS_JEMAOS_BACKUP_BUTTON_LABEL},
    {"jemaosSettingsBackupIntroTitle",
      IDS_OS_SETTINGS_JEMAOS_BACKUP_INTRO_TITLE},
    {"jemaosSettingsBackupPasswordPromptTitle",
      IDS_OS_SETTINGS_JEMAOS_BACKUP_PASSWORD_PROMPT_TITLE},
    {"jemaosSettingsBackupPasswordPromptText",
      IDS_OS_SETTINGS_JEMAOS_BACKUP_PASSWORD_PROMPT_TEXT},
  };

  html_source->AddLocalizedStrings(kLocalizedStrings);
  html_source->AddString("jemaosSettingsPageTitle",
      l10n_util::GetStringFUTF16(IDS_OS_SETTINGS_JEMAOS_SETTINGS,
        l10n_util::GetStringUTF16(IDS_PRODUCT_OS_NAME)));

  html_source->AddString(
      "jemaosSettingsBackupLabel",
      l10n_util::GetStringFUTF16(IDS_OS_SETTINGS_JEMAOS_BACKUP_LABEL,
          base::ASCIIToUTF16(
            jemaos::constants::kJemaOSBackupRestoreLearnMoreURL)));
  html_source->AddString(
      "jemaosSettingsBackupIntroText",
      l10n_util::GetStringFUTF16(
          IDS_OS_SETTINGS_JEMAOS_BACKUP_INTRO_TEXT,
          base::ASCIIToUTF16(
            jemaos::constants::kJemaOSBackupRestoreLearnMoreURL)));

  html_source->AddString(
      "toggleWidevineHelpMessage",
      l10n_util::GetStringFUTF16(
          IDS_OS_SETTINGS_JEMAOS_TOGGLE_LIBWIDEVINE_HELP_MESSAGE,
            base::ASCIIToUTF16(
              jemaos::constants::kJemaOSEnableWidevineLearnMoreURL)));

  const std::string board = base::SysInfo::GetLsbReleaseBoard();
  html_source->AddBoolean("showToggleRebootButtonInTray", true);
  html_source->AddBoolean("showToggleRotateScreenButton",
      jemaos::switches::IsNonForYouBoard(board));
  html_source->AddBoolean("showToggleSwitchTabletLaptopButton", true);

  html_source->AddString("jemaOSRdpUrl",
      jemaos::constants::kJemaOSRemoteDesktopURL);
  html_source->AddString("jemaosAccountBaseUrl",
      jemaos::constants::kJemaOSAccountBaseUrl);

  html_source->AddBoolean("isTpmFallbackNecessary",
      g_browser_process->local_state()->GetBoolean(
        jemaos::prefs::kForceTpmFallbackNecessary));
  html_source->AddString("jemaExperimentTpmfallbackUrl",
      jemaos::constants::kJemaExperimentTpmFallbackUrl);
}

int JemaOsSection::GetSectionNameMessageId() const {
  return IDS_OS_SETTINGS_JEMAOS_SETTINGS;
}

mojom::Section JemaOsSection::GetSection() const {
  return mojom::Section::kJemaOs;
}

mojom::SearchResultIcon JemaOsSection::GetSectionIcon() const {
  NOTIMPLEMENTED();
  return mojom::SearchResultIcon::kChrome;
}

std::string JemaOsSection::GetSectionPath() const {
  return mojom::kJemaOsSectionPath;
}

void JemaOsSection::AddHandlers(content::WebUI* web_ui) {
  web_ui->AddMessageHandler(
      std::make_unique<JemaOsHandler>(profile(), pref_service_));
}

bool JemaOsSection::LogMetric(
    mojom::Setting setting, base::Value& value) const {
  // Unimplemented.
  return false;
}

void JemaOsSection::RegisterHierarchy(HierarchyGenerator* generator) const {
  // jemaos top level
  generator->RegisterTopLevelSubpage(
      IDS_OS_SETTINGS_JEMAOS_SETTINGS, mojom::Subpage::kJemaOsMain,
      mojom::SearchResultIcon::kChrome, mojom::SearchResultDefaultRank::kMedium,
      mojom::kJemaOsSubpagePath);
}

}  // namespace ash::settings
