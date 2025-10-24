// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/ash/login/os_install_screen_handler.h"

#include <string>

#include "base/notreached.h"
#include "base/time/time.h"
#include "chrome/browser/ash/login/screens/os_install_screen.h"
#include "chrome/grit/branded_strings.h"
#include "chrome/grit/generated_resources.h"
#include "components/login/localized_values_builder.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/l10n/time_format.h"
#include "ui/strings/grit/ui_strings.h"
#include "components/user_manager/user_manager.h"
#include "base/system/sys_info.h"

namespace ash {
namespace {
constexpr const char kInProgressStep[] = "in-progress";
constexpr const char kFailedStep[] = "failed";
constexpr const char kNoDestinationDeviceFoundStep[] =
    "no-destination-device-found";
constexpr const char kSuccessStep[] = "success";
}  // namespace

OsInstallScreenHandler::OsInstallScreenHandler()
    : BaseScreenHandler(kScreenId) {}

OsInstallScreenHandler::~OsInstallScreenHandler() = default;

void OsInstallScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {
  builder->AddF("osInstallDialogIntroTitle", IDS_OS_INSTALL_SCREEN_INTRO_TITLE,
                IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->AddF("osInstallDialogIntroSubtitle",
                IDS_OS_INSTALL_SCREEN_INTRO_SUBTITLE,
                IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->AddF("osInstallDialogIntroBody0",
                IDS_OS_INSTALL_SCREEN_INTRO_CONTENT_0,
                IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->Add("osInstallDialogIntroBody1",
               IDS_OS_INSTALL_SCREEN_INTRO_CONTENT_1);
  builder->AddF("osInstallDialogIntroFooter",
                IDS_OS_INSTALL_SCREEN_INTRO_FOOTER,
                IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->AddF("osInstallDialogIntroNextButton",
                IDS_OS_INSTALL_SCREEN_INTRO_NEXT_BUTTON,
                IDS_INSTALLED_PRODUCT_OS_NAME);

  builder->AddF("osInstallDialogConfirmTitle",
                IDS_OS_INSTALL_SCREEN_CONFIRM_TITLE,
                IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->Add("osInstallDialogConfirmBody",
               IDS_OS_INSTALL_SCREEN_CONFIRM_BODY);
  builder->Add("osInstallDialogConfirmNextButton",
               IDS_OS_INSTALL_SCREEN_CONFIRM_NEXT_BUTTON);

  builder->AddF("osInstallDialogInProgressTitle",
                IDS_OS_INSTALL_SCREEN_IN_PROGRESS_TITLE,
                IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->Add("osInstallDialogInProgressSubtitle",
               IDS_OS_INSTALL_SCREEN_IN_PROGRESS_SUBTITLE);

  builder->Add("osInstallDialogErrorTitle", IDS_OS_INSTALL_SCREEN_ERROR_TITLE);
  builder->AddF("osInstallDialogErrorFailedSubtitle",
                IDS_OS_INSTALL_SCREEN_ERROR_FAILED_SUBTITLE,
                IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->AddF("osInstallDialogErrorNoDestSubtitle",
                IDS_OS_INSTALL_SCREEN_ERROR_NO_DEST_SUBTITLE,
                IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->Add("osInstallDialogErrorNoDestContent",
               IDS_OS_INSTALL_SCREEN_ERROR_NO_DEST_CONTENT);
  builder->Add("osInstallDialogServiceLogsTitle",
               IDS_OS_INSTALL_SCREEN_SERVICE_LOGS_TITLE);
  builder->Add("osInstallDialogErrorViewLogs",
               IDS_OS_INSTALL_SCREEN_ERROR_VIEW_LOGS);

  builder->Add("osInstallDialogSuccessTitle",
               IDS_OS_INSTALL_SCREEN_SUCCESS_TITLE);

  builder->Add("osInstallDialogSendFeedback",
               IDS_OS_INSTALL_SCREEN_SEND_FEEDBACK);
  builder->Add("osInstallDialogShutdownButton",
               IDS_OS_INSTALL_SCREEN_SHUTDOWN_BUTTON);

  DeclareJemaInstallerLocalizedValues(builder);
}

void OsInstallScreenHandler::GetAdditionalParameters(base::Value::Dict* parameters) {
  if (user_manager::UserManager::Get()->IsUserLoggedIn())
    return;
  parameters->Set("lsbReleaseBoard", base::SysInfo::GetLsbReleaseBoardWithoutSuffix());
}

void OsInstallScreenHandler::DeclareJemaInstallerLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {
  builder->Add("jemaosInstallerAppName", IDS_JEMAOS_INSTALLER_APP_NAME);
  builder->AddF("jemaosInstallerAppDesc", IDS_JEMAOS_INSTALLER_APP_DESC,
                IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->Add("jemaosInstallerAppDisclaimer",
               IDS_JEMAOS_INSTALLER_APP_DISCLAIMER);
  builder->Add("jemaosInstallerFullDiskInstallationTitle",
               IDS_JEMAOS_INSTALLER_FULL_DISK_INSTALLATION_TITLE);
  builder->AddF("jemaosInstallerFullDiskInstallationHelperText",
               IDS_JEMAOS_INSTALLER_FULL_DISK_INSTALLATION_HELPER_TEXT,
               IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->Add("jemaosInstallerDualbootInstallationTitle",
               IDS_JEMAOS_INSTALLER_DUALBOOT_INSTALLATION_TITLE);
  builder->AddF("jemaosInstallerDualbootInstallationHelperText",
               IDS_JEMAOS_INSTALLER_DUALBOOT_INSTALLATION_HELPER_TEXT,
               IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->AddF("jemaosInstallerFullDiskInstallationDisclaimer",
               IDS_JEMAOS_INSTALLER_FULL_DISK_INSTALLATION_DISCLAIMER,
               IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->AddF("jemaosInstallerDualbootInstallationDesc",
               IDS_JEMAOS_INSTALLER_DUALBOOT_INSTALLATION_DESC,
               IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->Add("jemaosInstallerDualbootInstallationDisclaimer",
               IDS_JEMAOS_INSTALLER_DUALBOOT_INSTALLATION_DISCLAIMER);
  builder->Add("jemaosInstallerInstallButton",
               IDS_JEMAOS_INSTALLER_TEXT_ON_INSTALL_BUTTON);
  builder->Add("jemaosInstallerConfigDiskSelectLabel",
               IDS_JEMAOS_INSTALLER_CONFIG_DISK_SELECT_LABEL);
  builder->Add("jemaosInstallerNormalConfigNoAvailableDisk",
               IDS_JEMAOS_INSTALLER_NORMAL_CONFIG_NO_AVAILABLE_DISK);
  builder->Add("jemaosInstallerInstalling",
               IDS_JEMAOS_INSTALLER_INSTALLING_TITLE);
  builder->Add("jemaosInstallerShutdownButton",
               IDS_JEMAOS_INSTALLER_TEXT_ON_SHUTDOWN_BUTTON);
  builder->Add("jemaosInstallerInstallSuccessTitle",
               IDS_JEMAOS_INSTALLER_INSTALL_SUCCESS_TITLE);
  builder->Add("jemaosInstallerInstallFailedTitle",
               IDS_JEMAOS_INSTALLER_INSTALL_FAILED_TITLE);
  builder->AddF("jemaosInstallerInstallSuccessMessage",
               IDS_JEMAOS_INSTALLER_INSTALL_SUCCESS_MESSAGE,
               IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->Add("jemaosInstallerInstallFailedMessage",
               IDS_JEMAOS_INSTALLER_INSTALL_FAILED_MESSAGE);
  builder->AddF("jemaosInstallDualbootConfigSelectOSPart",
               IDS_JEMAOS_INSTALLER_DUALBOOT_CONFIG_SELECT_OS_PARTITION,
               IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->Add("jemaosInstallerDualbootConfigPartSelectLabel",
               IDS_JEMAOS_INSTALLER_DUALBOOT_CONFIG_PARTITION_SELECT_LABEL);
  builder->Add("jemaosInstallDualbootConfigSelectEFIPart",
               IDS_JEMAOS_INSTALLER_DUALBOOT_CONFIG_SELECT_EFI_PARTITION);
  builder->Add("jemaosInstallerDualbootSelectedDiskHasNoAvailOSPart",
               IDS_JEMAOS_INSTALLER_DUALBOOT_SELECTED_DISK_HAS_NO_AVAIL_OS_PARTITION);
  builder->Add("jemaosInstallerDualbootSelectedDiskHasNoAvailEFIPart",
               IDS_JEMAOS_INSTALLER_DUALBOOT_SELECTED_DISK_HAS_NO_AVAIL_EFI_PARTITION);
  builder->Add("jemaosInstallerDualbootSelectedOSPartSizeSmaller",
               IDS_JEMAOS_INSTALLER_DUALBOOT_SELECTED_PARTITION_SIZE_SMALLER);
  builder->AddF("jemaosInstallerDualbootSelectedOSPartAlreadyInstalled",
               IDS_JEMAOS_INSTALLER_DUALBOOT_SELECTED_PARTITION_ALREADY_INSTALLED,
               IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->AddF("jemaosInstallerMultiBootOptionTitle",
               IDS_JEMAOS_INSTALLER_MULTI_BOOT_OPTION_TITLE,
               IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->Add("jemaosInstallerInstallREFIndTitle",
               IDS_JEMAOS_INSTALLER_INSTALL_REFIND_TITLE);
  builder->Add("jemaosInstallerInstallREFIndHelperText",
               IDS_JEMAOS_INSTALLER_INSTALL_REFIND_HELPER_TEXT);
  builder->Add("jemaosInstallerInstallUefiBootTitle",
               IDS_JEMAOS_INSTALLER_INSTALL_UEFI_BOOT_TITLE);
  builder->AddF("jemaosInstallerInstallUefiBootHelperText",
               IDS_JEMAOS_INSTALLER_INSTALL_UEFI_BOOT_HELPER_TEXT,
               IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->Add("jemaosInstallerMultiBootConfigureByYourselfTitle",
               IDS_JEMAOS_INSTALLER_MULTI_BOOT_CONFIGURE_BY_YOURSELF_TITLE);
  builder->AddF("jemaosInstallerMultiBootConfigureByYourselfHelperText",
               IDS_JEMAOS_INSTALLER_MULTI_BOOT_CONFIGURE_BY_YOURSELF_HELPER_TEXT,
               IDS_INSTALLED_PRODUCT_OS_NAME);
  builder->Add("jemaosInstallerAbortButton",
               IDS_JEMAOS_INSTALLER_TEXT_ON_ABORT_BUTTON);
  builder->Add("jemaosInstallerAbortTitle",
               IDS_JEMAOS_INSTALLER_ABORT_TITLE);
  builder->Add("jemaosInstallerAbortMessage",
               IDS_JEMAOS_INSTALLER_ABORT_MESSAGE);
  builder->Add("jemaosInstallerGetDiskInfoFailed",
               IDS_JEMAOS_INSTALLER_GET_DISK_INFO_FAILED);
  builder->Add("jemaosInstallerAbortingMessage",
               IDS_JEMAOS_INSTALLER_ABORTING_MESAGE);

  builder->Add("jemaosOobeWelcomeDisableNextDesc",
               IDS_JEMAOS_OOBE_WELCOME_DISABLE_NEXT_DESC);
  builder->Add("jemaosOobeWelcomeDisableNextContinueAnyway",
               IDS_JEMAOS_OOBE_WELCOME_DISABLE_NEXT_CONTINUE_ANYWAY);
}

void OsInstallScreenHandler::Show() {
  ShowInWebUI();
}

void OsInstallScreenHandler::ShowStep(const char* step) {
  CallExternalAPI("showStep", std::string(step));
}

void OsInstallScreenHandler::SetStatus(OsInstallClient::Status status) {
  switch (status) {
    case OsInstallClient::Status::InProgress:
      ShowStep(kInProgressStep);
      break;
    case OsInstallClient::Status::Succeeded:
      ShowStep(kSuccessStep);
      break;
    case OsInstallClient::Status::Failed:
      ShowStep(kFailedStep);
      break;
    case OsInstallClient::Status::NoDestinationDeviceFound:
      ShowStep(kNoDestinationDeviceFoundStep);
      break;
  }
}

void OsInstallScreenHandler::SetServiceLogs(const std::string& service_log) {
  CallExternalAPI("setServiceLogs", service_log);
}

void OsInstallScreenHandler::UpdateCountdownStringWithTime(
    base::TimeDelta time_left) {
  CallExternalAPI(
      "updateCountdownString",
      l10n_util::GetStringFUTF8(
          IDS_OS_INSTALL_SCREEN_SUCCESS_SUBTITLE,
          ui::TimeFormat::Simple(ui::TimeFormat::FORMAT_DURATION,
                                 ui::TimeFormat::LENGTH_LONG, time_left),
          l10n_util::GetStringUTF16(IDS_INSTALLED_PRODUCT_OS_NAME)));
}

base::WeakPtr<OsInstallScreenView> OsInstallScreenHandler::AsWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

}  // namespace ash
