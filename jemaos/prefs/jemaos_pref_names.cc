// Copyright (c) 2021 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/prefs/jemaos_pref_names.h"

namespace jemaos {
namespace prefs {

const char kPrefHideJemaOSStoreIcon[] = "hide_jemaos_store_icon";

const char kJemaOSImprovementPlanEnabled[] = "jemaos_improvement_plan_enabled";

// local state
const char kEnableArcIMEGlobally[] = "enable_arc_ime_globally";
const char kCurrentEnableArcIMEGlobally[] = "current_enable_arc_ime_globally";

const char kForceTpmFallbackNecessary[] = "force_tpm_fallback_necessary";
const char kCurrentForceTpmFallback[] = "current_force_tpm_fallback";
const char kForceTpmFallback[] = "force_tpm_fallback";

const char kShowSwitchTabletLaptopButton[] = "show_switch_tablet_laptop_button";
const char kShowRebootButtonInTray[] = "show_reboot_button_in_tray";
const char kShowRotateScreenButton[] = "show_rotate_screen_button";

const char kOfflineAutoSigninAccountIdKey[] =
    "offline_auto_signin.account_id_key";
const char kOfflineAutoSigninPassword[] = "offline_auto_signin.password";
const char kOfflineAutoSigninPasswordFormat[] =
    "offline_auto_signin.password_format";
const char kOfflineAutoSigninIsChromeLastSignout[] =
    "offline_auto_signin.chrome_signout";

const char kConnectApiUserId[] = "connect_api.user_id";
const char kConnectApiUserStatus[] = "connect_api.user_status";

const char kFactoryResetRequested[] = "FactoryResetRequested";

const char kRebootRequiredForWidevine[] = "reboot_required_for_widevine";

const char kJemaAssistantEnabled[] = "jema_assistant_enabled";
const char kJemaAssistantExtraAcceleratorEnabled[] =
    "jema_assistant_extra_accelerator_enabled";
const char kJemaAssistantEncryptedApiKeys[] =
    "jema_assistant.encrypted_api_keys";
const char kJemaAssistantAgentPermissions[] =
    "jema_assistant.agent_permissions";
const char kJemaAssistantAuditLog[] = "jema_assistant.audit_log";

const char kJemaOSArcMediaAutoScanEnabled[] =
    "jemaos_arc_media_auto_scan_enabled";

#if BUILDFLAG(USE_JEMAOS_LICENSE)
const char kJemaLicenseShouldShowInSettings[] =
    "jema_license_should_show_in_settings";
const char kJemaLicenseStateType[] = "jema_license_state_type";
const char kJemaLicenseEnforcementLevel[] = "jema_license_enforcement_level";
const char kJemaLicenseEnforcementLogOutInterval[] =
    "jemao_license_enforcement_log_out_interval";
#endif

#if BUILDFLAG(USE_JEMAOS_COM)
const char kCrostiniInstallerNotificationUserInteracted[] =
    "jemaos_crostini_installer_notificaion_user_interacted";
#endif
}  // namespace prefs
}  // namespace jemaos
