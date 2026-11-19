// Copyright 2020 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/license/jemaos_license_enforcement.h"

#include "base/timer/timer.h"
#include "chrome/browser/ui/browser_commands.h"
#include "ash/public/cpp/new_window_delegate.h"
#include "chrome/browser/apps/app_service/app_launch_params.h"
#include "chrome/browser/ui/extensions/application_launch.h"
#include "chrome/common/extensions/manifest_handlers/app_launch_info.h"
#include "chrome/browser/lifetime/application_lifetime.h"
#include "ash/public/cpp/notification_utils.h"
#include "chrome/browser/notifications/notification_handler.h"
#include "chrome/browser/notifications/notification_display_service.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/message_center/message_center.h"
#include "content/public/browser/web_contents.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "ui/base/l10n/l10n_util.h"
#include "ash/strings/grit/ash_strings.h"
#include "chrome/browser/notifications/notification_display_service_factory.h"
#include "jemaos/switches/license/license_switches.h"
#include "net/base/url_util.h"
#include "ash/constants/notifier_catalogs.h"
#include "base/system/sys_info.h"
#include "base/time/default_clock.h"
#include "jemaos/license/jemaos_license_user_util.h"

namespace jemaos::license {
namespace {
  // The in-OS settings license route does not exist in this build: opening it
  // landed on a dead page. Send the user to the SaaS billing page instead,
  // where the subscription can actually be renewed.
  const char kJemaOSSettingsLicensePath[] =
    "https://saas-app.jematech.fr/payment";
  const char kJemaOSLicenseForceQuitNotificationId[] =
    "jemaos.license.enforcement.forcequit";
  const char kJemaOSLicenseEnforceonmentNotifierId[] =
    "jemaos.license-enforcement";
  const int kJemaOSForceQuitDelayInMinutes = 15;
  const int kJemaOSEnforceIntervalInMinutes = 10;
}  // namespace

LicenseEnforcement::LicenseEnforcement():
    enforce_timer_(std::make_unique<base::RepeatingTimer>()),
    profile_(nullptr), is_eol_(false) {}

LicenseEnforcement::~LicenseEnforcement() {
  // Drop the notification (and its Unretained(this) click delegate) from the
  // message center, even if Stop()/StopEnforcement() was never called.
  RemoveForceQuitNotification();
}

void LicenseEnforcement::StartEnforcement(Profile* profile,
                                          const std::string& licenseID,
                                          const std::string& serialNumber,
                                          EnforcementMode mode,
                                          int logOutInterval) {
  profile_ = profile;
  id_ = licenseID;
  serial_number_ = serialNumber;
  mode_ = mode;

  if (logOutInterval > 0 && logOutInterval < 2 * 60 * 60) {
    log_out_interval_ = logOutInterval;
  } else {
    log_out_interval_ = kJemaOSForceQuitDelayInMinutes * 60;
  }

  VLOG(2) << "JemaOS License Enforcement, mode " << mode_ << ", log out interval: " << log_out_interval_;

  ::ash::UpdateEngineClient* update_engine_client = ::ash::UpdateEngineClient::Get();
  if (!update_engine_client) return;
  update_engine_client->GetEolInfo(
      base::BindOnce(&LicenseEnforcement::OnGetEolInfo, base::Unretained(this)));
}

void LicenseEnforcement::OnGetEolInfo(::ash::UpdateEngineClient::EolInfo info) {
  base::Clock* clock = base::DefaultClock::GetInstance();
  const base::Time now = clock->Now();
  const base::Time eol_date = info.eol_date;
  if (!eol_date.is_null() && eol_date <= now) {
    is_eol_ = true;
    VLOG(2) << "EOL device, skip license enforcement";
    return;
  }
  this->StartEnforcementInternal();
}

void LicenseEnforcement::StartEnforcementInternal() {

  if (enforce_timer_->IsRunning()) {
    if (jemaos::switches::IsLicenseTestMode()) {
      Enforce();
    }
    return;
  }

  enforce_timer_->Start(
      FROM_HERE,
      base::Minutes(kJemaOSEnforceIntervalInMinutes),
      base::BindRepeating(&LicenseEnforcement::Enforce,
                          base::Unretained(this)));

  Enforce();
}

void LicenseEnforcement::Enforce() {
  // Idempotent per session: the repeating timer may fire again, but the
  // popup and the notification are shown only once until StopEnforcement.
  if (mode_ > EnforcementModeLevel0 && !session_popup_shown_) {
    session_popup_shown_ = true;
    PopupLicenseWindow(false);
  }

  if (mode_ > EnforcementModeLevel1) {
    ForceQuitCurrentUser();
  }
}

void LicenseEnforcement::StopEnforcement() {
  // Safe even if nothing was ever started: the timer is constructed but may
  // never have run, and the notification may be null.
  if (enforce_timer_ && enforce_timer_->IsRunning()) {
    enforce_timer_->Stop();
  }
  session_popup_shown_ = false;
  session_notified_ = false;
  CloseLicenseWindow();
  // Always remove the notification, INCLUDING on EOL devices: the freemium
  // notice must disappear when the subscription activates, and its click
  // delegate holds Unretained(this) which must not outlive this object.
  RemoveForceQuitNotification();
}

void LicenseEnforcement::PopupLicenseWindow(bool from_user_interaction) {
  if (!profile_) return;

  VLOG(2) << "JemaOS License Enforcement, PopupLicenseWindow";
  ash::NewWindowDelegate* new_window_delegate =
      ash::NewWindowDelegate::GetInstance();
  if (!new_window_delegate) return;
  new_window_delegate->OpenUrl(
      GURL(kJemaOSSettingsLicensePath),
      from_user_interaction ?
      ash::NewWindowDelegate::OpenUrlFrom::kUserInteraction :
      ash::NewWindowDelegate::OpenUrlFrom::kUnspecified,
      ash::NewWindowDelegate::Disposition::kNewWindow);
}

void LicenseEnforcement::CloseLicenseWindow() {
  if (!profile_) return;
}

void LicenseEnforcement::ForceQuitCurrentUser() {
  if (!profile_) return;

  // One notification per session, and NO automatic logout. The old code
  // started a countdown (log_out_interval_, 60 s from the server) ending in
  // KickOut -> AttemptUserExit, and re-displayed the notification every 10 s.
  // With an expired license persisted and replayed at each login, that was
  // an endless sign-out loop. The user keeps working; clicking the
  // notification opens the license page.
  if (session_notified_) {
    return;
  }
  session_notified_ = true;
  ForceQuitNotification();
}

void LicenseEnforcement::ShowFreemiumNotice(Profile* profile) {
  if (!profile) return;
  profile_ = profile;
  // One discreet notification per session; never repeated, never a logout.
  // Reuses the one-shot notification built by ForceQuitNotification()
  // (message: the user can continue working and save files).
  if (session_notified_) {
    return;
  }
  session_notified_ = true;
  ForceQuitNotification();
}

void LicenseEnforcement::OnForceQuitNotificationClicked() {
  PopupLicenseWindow(true);
}

void LicenseEnforcement::ForceQuitNotification() {
  notification_ = ash::CreateSystemNotificationPtr(
      message_center::NOTIFICATION_TYPE_SIMPLE,
      kJemaOSLicenseForceQuitNotificationId,
      l10n_util::GetStringUTF16(
        IDS_ASH_JEMAOS_ENFORCEMENT_FORCE_QUIT_NOTIFICATION_TITLE),
      l10n_util::GetStringUTF16(
        IDS_ASH_JEMAOS_ENFORCEMENT_EXPIRED_NOTIFICATION_MESSAGE),
      std::u16string(), GURL(),
      message_center::NotifierId(
          message_center::NotifierType::SYSTEM_COMPONENT,
          kJemaOSLicenseEnforceonmentNotifierId,
          ::ash::NotificationCatalogName::kJemaOSLicenseEnforcement),
      message_center::RichNotificationData(),
      new message_center::HandleNotificationClickDelegate(
          base::BindRepeating(
            &LicenseEnforcement::OnForceQuitNotificationClicked,
            base::Unretained(this))),
      gfx::kNoneIcon,
      message_center::SystemNotificationWarningLevel::CRITICAL_WARNING);
  notification_->SetSystemPriority();
  notification_->set_pinned(true);
  notification_->set_fullscreen_visibility(
      message_center::FullscreenVisibility::OVER_USER);
  notification_->set_renotify(false);

  // Displayed exactly once; no refresh timer. The display service may not
  // exist yet (early startup / profile teardown): drop the notification
  // instead of keeping a pointer to something never shown.
  NotificationDisplayService* display_service =
      NotificationDisplayServiceFactory::GetForProfile(profile_);
  if (!display_service) {
    notification_.reset();
    return;
  }
  display_service->Display(
      NotificationHandler::Type::TRANSIENT, *notification_,
      /*metadata=*/nullptr);
}

void LicenseEnforcement::RemoveForceQuitNotification() {
  if (notification_ == nullptr) return;
  if (profile_) {
    NotificationDisplayService* display_service =
        NotificationDisplayServiceFactory::GetForProfile(profile_);
    if (display_service) {
      display_service->Close(
          NotificationHandler::Type::TRANSIENT,
          kJemaOSLicenseForceQuitNotificationId);
    }
  }
  // Always drop our pointer, even if the service is gone: keeping it would
  // leave session_notified_ semantics and the click delegate dangling.
  notification_.reset();
}

}  // namespace jemaos::license
