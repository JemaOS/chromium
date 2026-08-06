// Copyright (c) 2021 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_JEMAOS_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_JEMAOS_HANDLER_H_

#include "base/memory/weak_ptr.h"
#include "chrome/browser/ui/webui/settings/settings_page_ui_handler.h"
#include "components/account_id/account_id.h"
#include "components/prefs/pref_change_registrar.h"
#include "ash/public/cpp/tablet_mode_observer.h"
#include "ui/shell_dialogs/select_file_dialog.h"
#include "jemaos/ui/webui/settings/ash/jemaos_handler_backup_task_manager.h"
#include "chromeos/ash/components/dbus/cryptohome/UserDataAuth.pb.h"
#include "chromeos/ash/components/dbus/userdataauth/userdataauth_client.h"
#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/shell_state.h"

class PrefService;
class Profile;

namespace base {
class ListValue;
class FilePath;
}

namespace ash::settings {

class JemaOsHandler :
    public ::settings::SettingsPageUIHandler,
    public ui::SelectFileDialog::Listener,
    public ash::TabletModeObserver {
 public:
  explicit JemaOsHandler(Profile* profile, PrefService* pref_service);
  ~JemaOsHandler() override;

  // SettingsPageUIHandler implementation.
  void RegisterMessages() override;
  void OnJavascriptAllowed() override;
  void OnJavascriptDisallowed() override;

  // TabletModeObserver:
  void OnTabletPhysicalStateChanged() override;
 private:
  enum class FileDialogType {
    kUnspecified,
    kBackup,
    kRestore,
  };
  void OnSystemSaltObtained(const std::string& system_salt);
  void HandleGetIsOfflineAutoSigninEnabled(const base::Value::List& args);
  void HandleSaveOfflineLoginPassword(const base::Value::List& args);
  void HandleCleanOfflineLoginPassword(const base::Value::List& args);

  void OnShowRotateScreenButtonChanged();
  void HandleSetShowRotateScreenButton(const base::Value::List& args);
  void HandleGetShowRotateScreenButton(const base::Value::List& args);
  void HandleGetIsInTabletPhysicalState(const base::Value::List& args);

  void OnShowSwitchTabletLaptopButtonChanged();
  void HandleSetShowSwitchTabletLaptopButton(const base::Value::List& args);
  void HandleGetShowSwitchTabletLaptopButton(const base::Value::List& args);

  void HandleGetIsForceTpmFallback(const base::Value::List& args);
  void HandleSetForceTpmFallback(const base::Value::List& args);
  void OnForceTpmFallbackChanged();

  void HandleGetRebootRequiredForWidevine(const base::Value::List& args);
  void HandleToggleRebootRequiredForWidevine(const base::Value::List& args);
  void HandleTriggerWidevineUpdate(const base::Value::List& args);
  void OnWidevineUpdateCompleted(const std::string& callback_id,
                                 bool enable,
                                 std::optional<ShellState> state);
  void HandleGetCpuTurboEnabled(const base::Value::List& args);
  void HandleTriggerCpuTurbo(const base::Value::List& args);
  void OnCpuTurboCompleted(const std::string& callback_id,
                           bool enable,
                           std::optional<ShellState> state);


  void FileSelected(
      const ui::SelectedFileInfo& path, int index) override;
  void FileSelectionCanceled() override;


  bool nextToggleRebootRequiredForWidevine_ = false;
  bool lastToggleRebootRequiredForce_ = false;

  void HandleJemaOSBackupSupported(const base::Value::List& args);
  void HandleJemaOSBackupSelectFile(const base::Value::List& args);
  void HandleJemaOSBackupStarted(const base::Value::List& args);
  void HandleGetJemaOSBackupState(const base::Value::List& args);

  void OnJemaOSBackupScriptChecked(const std::string& callback_id,
                                   bool supported);

  void OnBackupFileSelected(const base::FilePath& path);
  void OnBackupFileSelectionCanceled();

  void OnBackupTaskFinished(BackupTaskManager::TaskState state);

  void HandleJemaOSRestoreSelectFile(const base::Value::List& args);
  void HandleJemaOSRestoreStarted(const base::Value::List& args);
  void OnRestoreCompleted(std::optional<ShellState> state);

  void OnRestoreFileSelected(const base::FilePath& path);
  void OnRestoreFileSelectionCanceled();

  // Cloud backup/restore handlers
  void HandleJemaOSCloudBackupStarted(const base::Value::List& args);
  void HandleJemaOSCloudRestoreStarted(const base::Value::List& args);
  void HandleJemaOSCloudListBackupFiles(const base::Value::List& args);
  void HandleGetJemaCloudBackupAvailable(const base::Value::List& args);

  // Returns true when cloud backup/restore may be used from this profile:
  // false for local (Flint) accounts and for Jema online accounts without an
  // active Pro/Pro+ subscription (Freemium). Other account types (e.g.
  // Google) keep the previous behavior.
  bool IsCloudBackupAvailableForProfile();
  void OnCloudBackupLocalCompleted(std::optional<ShellState> state);
  void OnCloudBackupPresignedUrlReceived(std::optional<ShellState> state);
  void OnCloudBackupUploadCompleted(std::optional<ShellState> state);
  void OnCloudListFilesCompleted(std::optional<ShellState> state);
  void OnCloudRestoreCompleted(std::optional<ShellState> state);
  void OnCloudRestorePresignedUrlReceived(std::optional<ShellState> state);
  void OnCloudRestoreDownloadCompleted(std::optional<ShellState> state);
  
  // Cloud backup context
  std::string cloud_backup_email_;
  std::string cloud_backup_filename_;
  std::string cloud_backup_temp_file_;
  std::string cloud_restore_password_;
  std::string cloud_restore_temp_file_;

  void HandleGetArcMediaAutoScanState(const base::Value::List& args);
  void OnArcMediaAutoScanIndicatorFileExistenceChecked(const std::string& callback_id, bool result);
  void HandleSetArcMediaAutoScanState(const base::Value::List& args);
  void OnEnableArcMediaAutoScan(bool result);
  void OnDisableArcMediaAutoScan(bool result);
  void RefreshArcMediaAutoScanState();
  void NotifyArcMediaAutoScanState(bool enabled);

  void HandleSetArcMediaAutoScanStateForCurrentSession(const base::Value::List& args);

  void HandleSetDevMode(const base::Value::List& args);
  void OnSetDevMode(const std::string& callback_id, bool result);
  void HandleGetDevModeSwitchSupported(const base::Value::List& args);
  void OnDevModeSwitchSupportedChecked(const std::string& callback_id, bool result);

  void ListAuthFactors(const AccountId& account_id, const std::string& callback_id);
  void OnListAuthFactors(const std::string& callback_id, std::optional<user_data_auth::ListAuthFactorsReply> reply);

  void HandleFetchConnectApiUserId(const base::Value::List& args);
  void HandleGetConnectApiUserId(const base::Value::List& args);
  void OnConnectApiUserIdReceived(const std::string& email, std::optional<ShellState> state);
  void HandleGetBackupAnalysis(const base::Value::List& args);
  void OnBackupAnalysisReceived(const std::string& callback_id, std::optional<ShellState> state);

  std::string system_salt_;
  bool auth_factor_has_password_ = false;
  Profile* profile_;
  PrefService* const prefs_;

  scoped_refptr<ui::SelectFileDialog> select_file_dialog_;
  FileDialogType file_dialog_type_ = FileDialogType::kUnspecified;

  PrefChangeRegistrar pref_change_registrar_;
  PrefChangeRegistrar local_state_pref_change_registrar_;

  base::WeakPtrFactory<JemaOsHandler> weak_ptr_factory_{this};
};

}  // namespace ash::settings

#endif
