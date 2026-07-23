// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/web_applications/preinstalled_web_apps/preinstalled_web_apps.h"

#include <optional>
#include <string>
#include <utility>

#include "base/check_op.h"
#include "base/command_line.h"
#include "base/feature_list.h"
#include "base/metrics/field_trial_params.h"
#include "base/strings/string_util.h"
#include "build/branding_buildflags.h"
#include "build/build_config.h"
#include "build/buildflag.h"
#include "chrome/browser/web_applications/preinstalled_app_install_features.h"
#include "chrome/common/chrome_switches.h"
#if BUILDFLAG(IS_CHROMEOS)
#include "components/prefs/pref_service.h"
#include "jemaos/prefs/jemaos_pref_names.h"
#endif  // BUILDFLAG(IS_CHROMEOS)

#if BUILDFLAG(GOOGLE_CHROME_BRANDING)
#include "chrome/browser/web_applications/preinstalled_web_apps/gmail.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/google_chat.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/google_docs.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/google_drive.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/google_sheets.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/google_slides.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/youtube.h"

#if BUILDFLAG(IS_CHROMEOS)
#include "ash/constants/web_app_id_constants.h"
#include "base/feature_list.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/calculator.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/gemini.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/google_calendar.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/google_meet.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/messages_dogfood.h"
#include "chrome/common/extensions/extension_constants.h"
#include "extensions/common/constants.h"
#include "google_apis/gaia/gaia_auth_util.h"
#endif  // BUILDFLAG(IS_CHROMEOS)

#endif  // BUILDFLAG(GOOGLE_CHROME_BRANDING)

#include "chrome/browser/web_applications/preinstalled_web_apps/community.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/remote_desktop.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/notes.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/mistral.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/qwant.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/jema_calculator.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/bentopdf.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/screennow.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/proton_mail.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/proton_drive.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/galerie.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/osivibe.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/nephtys.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/jemachess.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/setsound.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/anima.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/gmail.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/office365.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/jemapdf.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/quicktext.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/toffeeshare.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/excalidraw.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/vscode.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/photopea.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/telegram.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/whatsapp.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/teams.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/google_docs.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/google_sheets.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/google_meet.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/jema_youtube.h"

#if BUILDFLAG(IS_CHROMEOS)
#include "chrome/browser/ash/profiles/profile_helper.h"
#include "chrome/browser/profiles/profile.h"
#include "components/account_id/account_id.h"
#include "components/user_manager/user.h"
#endif  // BUILDFLAG(IS_CHROMEOS)

namespace web_app {
namespace {

std::vector<ExternalInstallOptions>* g_preinstalled_app_data_for_testing =
    nullptr;

#if BUILDFLAG(IS_CHROMEOS)
// JemaOS: returns true for Jema local (Flint) accounts. Local accounts have no
// Jema subscription/license: they must not receive the preinstalled premium
// Jema PWAs (OEM folder) that online Jema accounts get at first login.
bool IsJemaLocalAccount(Profile& profile) {
  // Primary check: local Jema accounts always use the @jemaos.local domain.
  // This is available as soon as the profile exists and does not depend on
  // the ProfileHelper user association timing during profile startup.
  if (base::EndsWith(profile.GetProfileUserName(), "@jemaos.local",
                     base::CompareCase::INSENSITIVE_ASCII)) {
    return true;
  }
  // A known, non-local email means an online (Jema or Google) account: older
  // builds mislabeled online Jema accounts with the local "ft_id_"/"flint_id_"
  // prefixes and the kFlintAccount type; those must NOT be treated as local.
  if (!profile.GetProfileUserName().empty()) {
    return false;
  }
  // Email unknown yet (early profile startup): fall back to type/prefix.
  ash::ProfileHelper* profile_helper = ash::ProfileHelper::Get();
  if (!profile_helper) {
    return false;
  }
  const user_manager::User* user = profile_helper->GetUserByProfile(&profile);
  if (!user) {
    return false;
  }
  if (user->IsFlintAccountUser()) {
    return true;
  }
  // Flint accounts are stored as GOOGLE accounts with the "ft_id_" gaia id
  // prefix (cryptohome compatibility); detect them by prefix as well.
  const AccountId& account_id = user->GetAccountId();
  return account_id.GetAccountType() == AccountType::GOOGLE &&
         (account_id.GetGaiaId().starts_with("ft_id_") ||
          account_id.GetGaiaId().starts_with("flint_id_"));
}

// JemaOS: returns true for Jema online accounts (the subscription-based
// tiers: Freemium, Pro, Pro+). Google accounts and other account types
// return false.
bool IsJemaOnlineAccount(Profile& profile) {
  if (IsJemaLocalAccount(profile)) {
    return false;
  }
  ash::ProfileHelper* profile_helper = ash::ProfileHelper::Get();
  if (!profile_helper) {
    return false;
  }
  const user_manager::User* user = profile_helper->GetUserByProfile(&profile);
  if (!user) {
    return false;
  }
  if (user->IsJemaAccountUser()) {
    return true;
  }
  // Jema online accounts may be stored as GOOGLE accounts with the
  // "jema_id_" gaia id prefix (cryptohome compatibility). Older builds
  // mislabeled online Jema accounts with the local "ft_id_"/"flint_id_"
  // prefixes; count those as online too (their email is not @jemaos.local,
  // so IsJemaLocalAccount above already returned false for them).
  const AccountId& account_id = user->GetAccountId();
  return account_id.GetAccountType() == AccountType::GOOGLE &&
         (account_id.GetGaiaId().starts_with("jema_id_") ||
          account_id.GetGaiaId().starts_with("ft_id_") ||
          account_id.GetGaiaId().starts_with("flint_id_"));
}

// JemaOS: returns true when the premium OEM PWAs (Jema apps bundle) may be
// installed for this profile. Only Jema online accounts with an active
// Pro/Pro+ subscription qualify; Freemium accounts get the root bundle plus
// QuickText and Galerie only. The subscription state is cached in a profile
// pref by UserSessionManager (fail-closed default: false).
bool MayInstallPremiumOemApps(Profile& profile) {
  if (!IsJemaOnlineAccount(profile)) {
    return true;
  }
  return profile.GetPrefs()->GetBoolean(jemaos::prefs::kJemaSubscriptionActive);
}
#endif  // BUILDFLAG(IS_CHROMEOS)

#if BUILDFLAG(GOOGLE_CHROME_BRANDING)

#if !BUILDFLAG(IS_CHROMEOS)
BASE_FEATURE(kChatPreinstalledWebApp,
             "ChatPreinstalledWebApp",
             base::FEATURE_ENABLED_BY_DEFAULT);

BASE_FEATURE_PARAM(bool,
                   kOnlyForNewUsers,
                   &kChatPreinstalledWebApp,
                   "only_for_new_users",
                   false);
#endif

#if BUILDFLAG(IS_CHROMEOS)
bool IsGoogleInternalAccount() {
  Profile* profile = ProfileManager::GetActiveUserProfile();
  if (!profile)
    return false;
  return gaia::IsGoogleInternalAccountEmail(profile->GetProfileUserName());
}
#endif  // BUILDFLAG(IS_CHROMEOS)


std::vector<ExternalInstallOptions> GetChromeBrandedApps(
    Profile& profile,
    const std::optional<DeviceInfo>& device_info,
    bool include_premium_oem) {
  // JemaOS: We replace Google's default apps with our own selection.
  // This avoids conflicts and ensures our configuration is used.
  std::vector<ExternalInstallOptions> apps;
  if (include_premium_oem) {
    // Jema Apps (OEM Folder) - requires an active Pro/Pro+ subscription.
    apps.push_back(GetConfigForJemaNotes());
    apps.push_back(GetConfigForAnima());
    apps.push_back(GetConfigForOsivibe());
    apps.push_back(GetConfigForSetSound());
    apps.push_back(GetConfigForJemaChess());
    apps.push_back(GetConfigForNephtys());
    apps.push_back(GetConfigForJemaPDF());
  }
  apps.insert(apps.end(), {
      // QuickText remains available for every tier (including Freemium).
      GetConfigForQuickText(),

      // Root Apps
      GetConfigForJemaCommunity(),
      GetConfigForJemaRemoteDesktop(),
      GetConfigForMistral(),
      GetConfigForQwant(),
      GetConfigForBentoPDF(),
      GetConfigForScreenNow(),
      GetConfigForProtonMail(),
      GetConfigForProtonDrive(),
      GetConfigForGalerie(),
      GetConfigForGmail(),
      GetConfigForJemaCalculator(),
      GetConfigForOffice365(),
      GetConfigForToffeeShare(),
      GetConfigForExcalidraw(),
      GetConfigForVSCode(),
      GetConfigForPhotopea(),
      GetConfigForTelegram(),
      GetConfigForWhatsApp(),
      GetConfigForTeams(),
      GetConfigForGoogleDocs(),
      GetConfigForGoogleSheets(),
      GetConfigForGoogleMeet(),
  });
  return apps;
}
#endif  // BUILDFLAG(GOOGLE_CHROME_BRANDING)

}  // namespace

DeviceInfo::DeviceInfo() = default;

DeviceInfo::DeviceInfo(const DeviceInfo&) = default;

DeviceInfo::DeviceInfo(DeviceInfo&&) = default;

DeviceInfo& DeviceInfo::operator=(const DeviceInfo&) = default;

DeviceInfo& DeviceInfo::operator=(DeviceInfo&&) = default;

DeviceInfo::~DeviceInfo() = default;

bool PreinstalledWebAppsDisabled() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(
      ::switches::kDisableDefaultApps);
}

std::vector<ExternalInstallOptions> GetPreinstalledWebApps(
    Profile& profile,
    const std::optional<DeviceInfo>& device_info) {
  if (g_preinstalled_app_data_for_testing)
    return *g_preinstalled_app_data_for_testing;

  if (PreinstalledWebAppsDisabled())
    return {};

#if BUILDFLAG(IS_CHROMEOS)
  // JemaOS: local (Flint) accounts have no Jema subscription: never install
  // the premium Jema PWAs (OEM folder) nor the first-login app bundle that
  // online Jema accounts receive.
  if (IsJemaLocalAccount(profile))
    return {};

  // JemaOS: Freemium Jema online accounts (no active Pro/Pro+ subscription)
  // receive the root bundle plus QuickText/Galerie, but not the premium OEM
  // Jema PWAs.
  const bool include_premium_oem = MayInstallPremiumOemApps(profile);
#else
  const bool include_premium_oem = true;
#endif  // BUILDFLAG(IS_CHROMEOS)

#if BUILDFLAG(GOOGLE_CHROME_BRANDING)
#if BUILDFLAG(IS_CHROMEOS)
  // TODO(crbug.com/40854011): replace with config in admin console.
  if (IsGoogleInternalAccount()) {
    std::vector<ExternalInstallOptions> apps =
        GetChromeBrandedApps(profile, device_info, include_premium_oem);
    apps.push_back(GetConfigForMessagesDogfood());
    return apps;
  }
#endif  // BUILDFLAG(IS_CHROMEOS)

  return GetChromeBrandedApps(profile, device_info, include_premium_oem);
#else
  std::vector<ExternalInstallOptions> apps;
#if BUILDFLAG(IS_CHROMEOS)
  if (include_premium_oem) {
    // Jema Apps (OEM Folder) - requires an active Pro/Pro+ subscription.
    apps.push_back(GetConfigForJemaNotes());
    apps.push_back(GetConfigForAnima());
    apps.push_back(GetConfigForOsivibe());
    apps.push_back(GetConfigForSetSound());
    apps.push_back(GetConfigForJemaChess());
    apps.push_back(GetConfigForNephtys());
    apps.push_back(GetConfigForJemaPDF());
  }
#endif  // BUILDFLAG(IS_CHROMEOS)
  apps.insert(apps.end(), {
      // QuickText remains available for every tier (including Freemium).
      GetConfigForQuickText(),

      // Root Apps
      GetConfigForJemaCommunity(),
      GetConfigForJemaRemoteDesktop(),
      GetConfigForMistral(),
      GetConfigForQwant(),
      GetConfigForBentoPDF(),
      GetConfigForScreenNow(),
      GetConfigForProtonMail(),
      GetConfigForProtonDrive(),
      GetConfigForGalerie(),
      GetConfigForGmail(),
      GetConfigForJemaCalculator(),
      GetConfigForOffice365(),
      GetConfigForToffeeShare(),
      GetConfigForExcalidraw(),
      GetConfigForVSCode(),
      GetConfigForPhotopea(),
      GetConfigForTelegram(),
      GetConfigForWhatsApp(),
      GetConfigForTeams(),
      GetConfigForGoogleDocs(),
      GetConfigForGoogleSheets(),
      GetConfigForGoogleMeet(),
      GetConfigForJemaYouTube(),
  });
  return apps;
#endif  // BUILDFLAG(GOOGLE_CHROME_BRANDING)
}

ScopedTestingPreinstalledAppData::ScopedTestingPreinstalledAppData() {
  DCHECK_EQ(nullptr, g_preinstalled_app_data_for_testing);
  g_preinstalled_app_data_for_testing = &apps;
}

ScopedTestingPreinstalledAppData::~ScopedTestingPreinstalledAppData() {
  DCHECK_EQ(&apps, g_preinstalled_app_data_for_testing);
  g_preinstalled_app_data_for_testing = nullptr;
}

PreinstalledWebAppMigration::PreinstalledWebAppMigration() = default;
PreinstalledWebAppMigration::PreinstalledWebAppMigration(
    PreinstalledWebAppMigration&&) noexcept = default;
PreinstalledWebAppMigration::~PreinstalledWebAppMigration() = default;

std::vector<PreinstalledWebAppMigration> GetPreinstalledWebAppMigrations(
    Profile& profile) {
  std::vector<PreinstalledWebAppMigration> migrations;
  for (const ExternalInstallOptions& options :
       GetPreinstalledWebApps(profile)) {
    if (!options.expected_app_id)
      continue;

    // All entries in the default web app migration had only one Chrome app to
    // replace.
    if (options.uninstall_and_replace.size() != 1)
      continue;

    if (options.gate_on_feature && !IsPreinstalledAppInstallFeatureEnabled(
                                       *options.gate_on_feature, profile)) {
      continue;
    }

    PreinstalledWebAppMigration migration;
    migration.install_url = options.install_url;
    migration.expected_web_app_id = *options.expected_app_id;
    migration.old_chrome_app_id = options.uninstall_and_replace[0];
    migrations.push_back(std::move(migration));
  }

#if BUILDFLAG(GOOGLE_CHROME_BRANDING) && BUILDFLAG(IS_CHROMEOS)
  if (!g_preinstalled_app_data_for_testing && !PreinstalledWebAppsDisabled()) {
    // Manually hard coded entries from
    // https://chrome-internal.googlesource.com/chromeos/overlays/chromeos-overlay/+/main/chromeos-base/chromeos-default-apps/files/web_apps
    // for any json configs that include a uninstall_and_replace field.
    // This is a temporary measure while the default web app duplication
    // issue is cleaned up.
    // TODO(crbug.com/40818306): Clean up once no longer needed.
    // Default installed GSuite web apps.
    {
      PreinstalledWebAppMigration keep_migration;
      keep_migration.install_url =
          GURL("https://keep.google.com/installwebapp?usp=chrome_default");
      keep_migration.expected_web_app_id = ash::kGoogleKeepAppId;
      keep_migration.old_chrome_app_id = extension_misc::kGoogleKeepAppId;
      migrations.push_back(std::move(keep_migration));
    }

    // Default installed non-GSuite web apps.
    {
      PreinstalledWebAppMigration books_migration;
      books_migration.install_url =
          GURL("https://play.google.com/books/installwebapp?usp=chromedefault");
      books_migration.expected_web_app_id = ash::kPlayBooksAppId;
      books_migration.old_chrome_app_id = extension_misc::kGooglePlayBooksAppId;
      migrations.push_back(std::move(books_migration));

      PreinstalledWebAppMigration maps_migration;
      maps_migration.install_url =
          GURL("https://www.google.com/maps/preview/pwa/ttinstall.html");
      maps_migration.expected_web_app_id = ash::kGoogleMapsAppId;
      maps_migration.old_chrome_app_id = extension_misc::kGoogleMapsAppId;
      migrations.push_back(std::move(maps_migration));

      PreinstalledWebAppMigration movies_migration;
      movies_migration.install_url = GURL(
          "https://play.google.com/store/movies/"
          "installwebapp?usp=chrome_default");
      movies_migration.expected_web_app_id = ash::kGoogleMoviesAppId;
      movies_migration.old_chrome_app_id =
          extension_misc::kGooglePlayMoviesAppId;
      migrations.push_back(std::move(movies_migration));
    }
  }
#endif  // BUILDFLAG(GOOGLE_CHROME_BRANDING) && BUILDFLAG(IS_CHROMEOS)
  return migrations;
}

}  // namespace web_app
