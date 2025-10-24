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
  const char kJemaOSSettingsLicensePath[] =
    "chrome://os-settings/jemaos/license";
  const char kJemaOSLicenseForceQuitNotificationId[] =
    "jemaos.license.enforcement.forcequit";
  const char kJemaOSLicenseEnforceonmentNotifierId[] =
    "jemaos.license-enforcement";
  const int kJemaOSForceQuitDelayInMinutes = 15;
  const int kJemaOSRefreshForceQuitNotificationIntervalInSeconds = 10;
  const int kJemaOSEnforceIntervalInMinutes = 10;
}  // namespace

LicenseEnforcement::LicenseEnforcement():
    enforce_timer_(std::make_unique<base::RepeatingTimer>()),
    force_quit_timer_(std::make_unique<base::OneShotTimer>()),
    notification_timer_(std::make_unique<base::RepeatingTimer>()),
    profile_(nullptr), is_eol_(false) {}

LicenseEnforcement::~LicenseEnforcement() = default;

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
  if (mode_ > EnforcementModeLevel0) {
    PopupLicenseWindow(false);
  }

  if (mode_ > EnforcementModeLevel1) {
    ForceQuitCurrentUser();
  }
}

void LicenseEnforcement::StopEnforcement() {
  if (is_eol_) {
    return;
  }
  if (force_quit_timer_->IsRunning()) {
    force_quit_timer_->Stop();
  }
  if (notification_timer_->IsRunning()) {
    notification_timer_->Stop();
  }
  if (enforce_timer_->IsRunning()) {
    enforce_timer_->Stop();
  }
  CloseLicenseWindow();
  RemoveForceQuitNotification();
}

void LicenseEnforcement::PopupLicenseWindow(bool from_user_interaction) {
  if (!profile_) return;

  VLOG(2) << "JemaOS License Enforcement, PopupLicenseWindow";
  ash::NewWindowDelegate::GetInstance()->OpenUrl(
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

  if (force_quit_timer_->IsRunning()) {
    PopupForceQuitNotification();
    return;
  }

  VLOG(2) << "JemaOS LicenseEnforcement, logout after "
          << log_out_interval_ << " seconds";
  ForceQuitNotification();
  force_quit_timer_->Start(
      FROM_HERE,
      base::Seconds(log_out_interval_),
      base::BindOnce(&LicenseEnforcement::KickOut, base::Unretained(this)));
}

void LicenseEnforcement::KickOut() {
  StopEnforcement();
  chrome::AttemptUserExit();
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
      std::u16string(), std::u16string(), GURL(),
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

  notification_timer_->Start(
      FROM_HERE,
      base::Seconds(kJemaOSRefreshForceQuitNotificationIntervalInSeconds),
      base::BindRepeating(
        &LicenseEnforcement::UpdateForceQuitNotification,
        base::Unretained(this)));

  UpdateForceQuitNotification();
}

std::u16string LicenseEnforcement::ForceQuitNotificationMessage() {
  base::TimeDelta left =
    force_quit_timer_->desired_run_time() - base::TimeTicks::Now();
  int seconds = left.InSeconds();
  return l10n_util::GetStringFUTF16(
      IDS_ASH_JEMAOS_ENFORCEMENT_FORCE_QUIT_NOTIFICATION_MESSAGE,
      base::NumberToString16(
        seconds > 0 ? seconds : log_out_interval_));
}

void LicenseEnforcement::UpdateForceQuitNotification() {
  if (notification_ == nullptr) return;
  notification_->set_message(ForceQuitNotificationMessage());
  notification_->set_renotify(false);

  NotificationDisplayServiceFactory::GetForProfile(profile_)->Display(
      NotificationHandler::Type::TRANSIENT, *notification_,
      /*metadata=*/nullptr);
}

void LicenseEnforcement::PopupForceQuitNotification() {
  if (notification_ == nullptr) {
    return;
  }
  notification_->set_renotify(true);

  NotificationDisplayServiceFactory::GetForProfile(profile_)->Display(
      NotificationHandler::Type::TRANSIENT, *notification_,
      /*metadata=*/nullptr);
}

void LicenseEnforcement::RemoveForceQuitNotification() {
  if (!profile_ || notification_ == nullptr) return;
  NotificationDisplayServiceFactory::GetForProfile(profile_)->Close(
      NotificationHandler::Type::TRANSIENT,
      kJemaOSLicenseForceQuitNotificationId);
  notification_.reset();
}

}  // namespace jemaos::license
