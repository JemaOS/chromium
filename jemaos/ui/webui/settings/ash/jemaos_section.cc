// Copyright (c) 2021 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/ui/webui/settings/ash/jemaos_section.h"
#include "base/command_line.h"
#include "base/no_destructor.h"
#include "chrome/grit/generated_resources.h"
#include "chrome/grit/branded_strings.h"
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
#include "jemaos/build/config/buildflags.h"

#if BUILDFLAG(USE_JEMAOS_LICENSE)
#include "jemaos/switches/license/license_switches.h"
#include "jemaos/license/jemaos_license_user_util.h"
#endif

#include "chromeos/dbus/constants/dbus_switches.h"

namespace ash::settings {

namespace mojom {
using ::chromeos::settings::mojom::kJemaOsSectionPath;
using ::chromeos::settings::mojom::kJemaOsSubpagePath;
#if BUILDFLAG(USE_JEMAOS_LICENSE)
using ::chromeos::settings::mojom::kJemaOsLicenseInfoSubpagePath;
#endif
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
#if BUILDFLAG(USE_JEMAOS_LICENSE)
      {IDS_OS_SETTINGS_JEMAOS_SETTINGS_JEMAOS_LICENSE_INFO_TITLE,
       mojom::kJemaOsLicenseInfoSubpagePath,
       mojom::SearchResultIcon::kChrome,
       mojom::SearchResultDefaultRank::kMedium,
       mojom::SearchResultType::kSubpage,
       {.subpage = mojom::Subpage::kJemaOsLicenseInfo}},
#endif
    });
    return *tags;
  }

#if BUILDFLAG(USE_JEMAOS_LICENSE)
  const char kJemaOSLicenseLookupPath[] = "/web/license.html";
#endif
}  // namespace

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
    {"jemaosCpuTurboLabel", IDS_OS_SETTINGS_JEMAOS_CPU_TURBO_LABEL},
    {"jemaosCpuTurboHelpMessage", IDS_OS_SETTINGS_JEMAOS_CPU_TURBO_HELP_MESSAGE},
    {"jemaosCpuTurboFailedTitle", IDS_OS_SETTINGS_JEMAOS_FAILED_CPU_TURBO_TITLE},
    {"jemaosCpuTurboFailedMessage", IDS_OS_SETTINGS_JEMAOS_FAILED_CPU_TURBO_MESSAGE},
    {"jemaosSettingsMenuItemDescription",
      IDS_OS_SETTINGS_JEMAOS_MENU_ITEM_DESCRIPTION},
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
    {"unableToSetAutoSigninWithoutPasswordAuthFactor",
      IDS_OS_SETTINGS_JEMAOS_UNABLE_TO_SET_AUTO_SIGNIN_WITHOUT_PASSWORD_AUTH_FACTOR},

    {"jemaosSettingsBackupSaveFileDialogTitle",
      IDS_OS_SETTINGS_JEMAOS_BACKUP_SAVE_FILE_DIALOG_TITLE},
    {"jemaosSettingsBackupButtonLabel",
      IDS_OS_SETTINGS_JEMAOS_BACKUP_BUTTON_LABEL},
    {"jemaosSettingsBackupIntroTitle",
      IDS_OS_SETTINGS_JEMAOS_BACKUP_INTRO_TITLE},
    {"jemaosSettingsBackupPasswordPromptTitle",
      IDS_OS_SETTINGS_JEMAOS_BACKUP_PASSWORD_PROMPT_TITLE},
    {"jemaosSettingsBackupPasswordPromptText",
      IDS_OS_SETTINGS_JEMAOS_BACKUP_PASSWORD_PROMPT_TEXT},

    {"jemaosSettingsRestoreSelectFileDialogTitle",
      IDS_OS_SETTINGS_JEMAOS_RESTORE_SELECT_FILE_DIALOG_TITLE},
    {"jemaosSettingsRestoreLabel",
      IDS_OS_SETTINGS_JEMAOS_RESTORE_LABEL},
    {"jemaosSettingsRestoreButtonLabel",
      IDS_OS_SETTINGS_JEMAOS_RESTORE_BUTTON_LABEL},
    {"jemaosSettingsRestorePasswordDialogTitle",
      IDS_OS_SETTINGS_JEMAOS_RESTORE_PASSWORD_DIALOG_TITLE},
    {"jemaosSettingsRestorePasswordDialogMessage",
      IDS_OS_SETTINGS_JEMAOS_RESTORE_PASSWORD_DIALOG_MESSAGE},
    {"jemaosSettingsRestorePasswordLabel",
      IDS_OS_SETTINGS_JEMAOS_RESTORE_PASSWORD_LABEL},
    {"jemaosSettingsRestorePasswordError",
      IDS_OS_SETTINGS_JEMAOS_RESTORE_PASSWORD_ERROR},
    {"jemaosSettingsRestoreSuccessTitle",
      IDS_OS_SETTINGS_JEMAOS_RESTORE_SUCCESS_TITLE},
    {"jemaosSettingsRestoreErrorTitle",
      IDS_OS_SETTINGS_JEMAOS_RESTORE_ERROR_TITLE},

    // Cloud backup/restore strings
    {"jemaosSettingsBackupLocalDesc",
      IDS_OS_SETTINGS_JEMAOS_BACKUP_LOCAL_DESC},
    {"jemaosSettingsCloudBackupLabel",
      IDS_OS_SETTINGS_JEMAOS_CLOUD_BACKUP_LABEL},
    {"jemaosSettingsCloudBackupDesc",
      IDS_OS_SETTINGS_JEMAOS_CLOUD_BACKUP_DESC},
    {"jemaosSettingsCloudBackupButtonLabel",
      IDS_OS_SETTINGS_JEMAOS_CLOUD_BACKUP_BUTTON_LABEL},
    {"jemaosSettingsRestoreLocalDesc",
      IDS_OS_SETTINGS_JEMAOS_RESTORE_LOCAL_DESC},
    {"jemaosSettingsCloudRestoreLabel",
      IDS_OS_SETTINGS_JEMAOS_CLOUD_RESTORE_LABEL},
    {"jemaosSettingsCloudRestoreDesc",
      IDS_OS_SETTINGS_JEMAOS_CLOUD_RESTORE_DESC},
    {"jemaosSettingsCloudRestoreButtonLabel",
      IDS_OS_SETTINGS_JEMAOS_CLOUD_RESTORE_BUTTON_LABEL},

    // Backup API and Storage strings
    {"jemaosSettingsConnectApiUserInfoTitle",
      IDS_OS_SETTINGS_JEMAOS_CONNECT_API_USER_INFO_TITLE},
    {"jemaosSettingsLoading",
      IDS_OS_SETTINGS_JEMAOS_LOADING},
    {"jemaosSettingsUserIdLabel",
      IDS_OS_SETTINGS_JEMAOS_USER_ID_LABEL},
    {"jemaosSettingsStatusLabel",
      IDS_OS_SETTINGS_JEMAOS_STATUS_LABEL},
    {"jemaosSettingsBackupStorageAnalysisTitle",
      IDS_OS_SETTINGS_JEMAOS_BACKUP_STORAGE_ANALYSIS_TITLE},
    {"jemaosSettingsUserIdRequired",
      IDS_OS_SETTINGS_JEMAOS_USER_ID_REQUIRED},
    {"jemaosSettingsStorageLabel",
      IDS_OS_SETTINGS_JEMAOS_STORAGE_LABEL},
    {"jemaosSettingsLastUpdatedLabel",
      IDS_OS_SETTINGS_JEMAOS_LAST_UPDATED_LABEL},
    {"jemaosSettingsHardwareDevicesLabel",
      IDS_OS_SETTINGS_JEMAOS_HARDWARE_DEVICES_LABEL},
    {"jemaosSettingsNoBackupData",
      IDS_OS_SETTINGS_JEMAOS_NO_BACKUP_DATA},

    // Restore dialog strings
    {"jemaosSettingsClose",
      IDS_OS_SETTINGS_JEMAOS_CLOSE},
    {"jemaosSettingsLoadingBackupFiles",
      IDS_OS_SETTINGS_JEMAOS_LOADING_BACKUP_FILES},
    {"jemaosSettingsSelectBackupFileLabel",
      IDS_OS_SETTINGS_JEMAOS_SELECT_BACKUP_FILE_LABEL},
    {"jemaosSettingsSelectBackupFilePlaceholder",
      IDS_OS_SETTINGS_JEMAOS_SELECT_BACKUP_FILE_PLACEHOLDER},
    {"jemaosSettingsNoBackupFilesFound",
      IDS_OS_SETTINGS_JEMAOS_NO_BACKUP_FILES_FOUND},

    // JemaOS AI Assistant strings
    {"jemaosComingSoon",
      IDS_OS_SETTINGS_JEMAOS_COMING_SOON},

#if BUILDFLAG(USE_JEMAOS_LICENSE)
    {"jemaosSettingsLicenseStateLoading",
      IDS_OS_SETTINGS_JEMAOS_SETTINGS_JEMAOS_LICENSE_INFO_STATE_LOADING},
    {"jemaosSettingsLicenseRetryButtonText",
      IDS_OS_SETTINGS_JEMAOS_SETTINGS_JEMAOS_LICENSE_INFO_RETRY_BUTTON},
    {"jemaosSettingsLicenseLabel",
      IDS_OS_SETTINGS_JEMAOS_SETTINGS_JEMAOS_LICENSE_INFO_MENU},
    {"jemaosSettingsLicenseTitle",
      IDS_OS_SETTINGS_JEMAOS_SETTINGS_JEMAOS_LICENSE_INFO_TITLE},
    {"jemaosSettingsLicenseErrorReadMachineId",
      IDS_OS_SETTINGS_JEMAOS_SETTINGS_JEMAOS_LICENSE_INFO_ERROR_MACHINE_ID},
    {"jemaosSettingsLicenseIdLabel",
      IDS_OS_SETTINGS_JEMAOS_SETTINGS_JEMAOS_LICENSE_INFO_ID_LABEL},
    {"jemaosSettingsLicenseExpireDateLabel",
      IDS_OS_SETTINGS_JEMAOS_SETTINGS_JEMAOS_LICENSE_INFO_EXPIRE_DATE_LABEL},
    {"jemaosSettingsLicenseRetryWebviewButtonText",
      IDS_OS_SETTINGS_JEMAOS_SETTINGS_JEMAOS_LICENSE_INFO_RETRY_WEBVIEW_BUTTON},
    {"jemaosSettingsLicenseOfflineHintMessage",
      IDS_OS_SETTINGS_JEMAOS_SETTINGS_JEMAOS_LICENSE_INFO_OFFLINE_HINT_MESSAGE},
#endif
    {"jemaosSettingsSecuritySectionTitle",
      IDS_OS_SETTINGS_JEMAOS_SECURITY_SECTION_TITLE},
    {"jemaosSettingsEnableDevModeButtonText",
      IDS_OS_SETTINGS_JEMAOS_SETTINGS_ENABLE_DEV_MODE_BUTTON_TEXT},
    {"jemaosSettingsInDevModeTooltip",
      IDS_OS_SETTINGS_JEMAOS_SETTINGS_IN_DEV_MODE_TOOLTIP},
    {"jemaosSettingsEnableDevModeConfirmTitle",
      IDS_OS_SETTINGS_JEMAOS_SETTINGS_ENABLE_DEV_MODE_CONFIRM_TITLE},
    {"jemaosSettingsEnableDevModeConfirmMessage",
      IDS_OS_SETTINGS_JEMAOS_SETTINGS_ENABLE_DEV_MODE_CONFIRM_MESSAGE},
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

  html_source->AddString(
      "toggleArcMediaAutoScanLabel",
      l10n_util::GetStringFUTF16(
          IDS_OS_SETTINGS_JEMAOS_TOGGLE_ARC_MEDIA_AUTO_SCAN_LABEL,
            base::ASCIIToUTF16(
              jemaos::constants::kJemaOSToggleArcMediaAutoScanLearnMoreURL)));

  html_source->AddString(
      "jemaosSettingsDevModeTransitionLabel",
      l10n_util::GetStringFUTF16(IDS_OS_SETTINGS_JEMAOS_DEV_MODE_TRANSITION_LABEL,
                                 l10n_util::GetStringUTF16(IDS_PRODUCT_OS_NAME),
                                 base::ASCIIToUTF16(
                                 jemaos::constants::kJemaOSDevModeTransitionLearnMoreURL)));

  const std::string board = base::SysInfo::GetLsbReleaseBoard();
  html_source->AddBoolean("showToggleRebootButtonInTray", false);
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

#if BUILDFLAG(USE_JEMAOS_LICENSE)
  html_source->AddBoolean("showJemaOsLicense", g_browser_process->local_state()->GetBoolean(jemaos::prefs::kJemaLicenseShouldShowInSettings));

  GURL url(jemaos::switches::GetJemaOSLicenseWebUrl() + kJemaOSLicenseLookupPath);
  html_source->AddString("jemaosSettingsLicenseUrl",
      jemaos::license::AppendAccountIdQueryParameter(url).spec());
  html_source->AddString("jemaosBoardName", board);
#endif

  html_source->AddBoolean("devMode",
                          base::CommandLine::ForCurrentProcess()->HasSwitch(
                          chromeos::switches::kSystemDevMode));
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

const char* JemaOsSection::GetSectionPath() const {
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

#if BUILDFLAG(USE_JEMAOS_LICENSE)
  // license info.
  generator->RegisterNestedSubpage(
      IDS_OS_SETTINGS_JEMAOS_SETTINGS_JEMAOS_LICENSE_INFO_TITLE,
      mojom::Subpage::kJemaOsLicenseInfo, mojom::Subpage::kJemaOsMain,
      mojom::SearchResultIcon::kChrome, mojom::SearchResultDefaultRank::kMedium,
      mojom::kJemaOsLicenseInfoSubpagePath);
#endif
}

}  // namespace ash::settings
