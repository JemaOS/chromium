// Copyright (c) 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_JEMAOS_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_JEMAOS_HANDLER_H_

#include "base/memory/weak_ptr.h"
#include "chrome/browser/ui/webui/settings/settings_page_ui_handler.h"
#include "components/prefs/pref_change_registrar.h"
#include "ash/public/cpp/tablet_mode_observer.h"
#include "ui/shell_dialogs/select_file_dialog.h"
#include "jemaos/ui/webui/settings/ash/jemaos_handler_backup_task_manager.h"

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
  // Constructor and destructor
  // NOTE FOR DEVELOPERS: Ensure proper cleanup of resources in the destructor.
  explicit JemaOsHandler(Profile* profile, PrefService* pref_service);
  ~JemaOsHandler() override;

  // SettingsPageUIHandler implementation
  void RegisterMessages() override;
  void OnJavascriptAllowed() override;
  void OnJavascriptDisallowed() override;

  // TabletModeObserver implementation
  void OnTabletPhysicalStateChanged() override;

 private:
  // Enum representing the type of file dialog
  enum class FileDialogType {
    kUnspecified,  // No specific file dialog type
    kLibwidevine,  // File dialog for selecting Widevine files
    kBackup,       // File dialog for selecting backup files
  };

  // Handles system salt retrieval
  void OnSystemSaltObtained(const std::string& system_salt);

  // Handlers for offline auto sign-in
  void HandleGetIsOfflineAutoSigninEnabled(const base::Value::List& args);
  void HandleSaveOfflineLoginPassword(const base::Value::List& args);
  void HandleCleanOfflineLoginPassword(const base::Value::List& args);

  // Handlers for reboot button in tray
  void OnShowRebootButtonInTrayChanged();
  void HandleSetShowRebootButtonInTray(const base::Value::List& args);
  void HandleGetShowRebootButtonInTray(const base::Value::List& args);

  // Handlers for rotate screen button
  void OnShowRotateScreenButtonChanged();
  void HandleSetShowRotateScreenButton(const base::Value::List& args);
  void HandleGetShowRotateScreenButton(const base::Value::List& args);

  // Handlers for tablet/laptop mode switching
  void HandleGetIsInTabletPhysicalState(const base::Value::List& args);
  void OnShowSwitchTabletLaptopButtonChanged();
  void HandleSetShowSwitchTabletLaptopButton(const base::Value::List& args);
  void HandleGetShowSwitchTabletLaptopButton(const base::Value::List& args);

  // Handlers for TPM fallback
  void HandleGetIsForceTpmFallback(const base::Value::List& args);
  void HandleSetForceTpmFallback(const base::Value::List& args);
  void OnForceTpmFallbackChanged();

  // Handlers for Widevine file selection
  void HandleSelectLibwidevineFile(const base::Value::List& args);
  void HandleGetRebootRequiredForWidevine(const base::Value::List& args);
  void HandleToggleRebootRequiredForWidevine(const base::Value::List& args);

  // File dialog callbacks
  void FileSelected(const base::FilePath& path, int index, void* params) override;
  void FileSelectionCanceled(void* params) override;

  // Widevine file selection callbacks
  void OnLibwidevineFileSelected(const base::FilePath& path);
  void OnLibwidevineFileSelectionCanceled();

  // Backup-related handlers
  void HandleJemaOSBackupSupported(const base::Value::List& args);
  void HandleJemaOSBackupSelectFile(const base::Value::List& args);
  void HandleJemaOSBackupStarted(const base::Value::List& args);
  void HandleGetJemaOSBackupState(const base::Value::List& args);

  // Backup task callbacks
  void OnJemaOSBackupScriptChecked(const std::string& callback_id, bool supported);
  void OnBackupFileSelected(const base::FilePath& path);
  void OnBackupFileSelectionCanceled();
  void OnBackupTaskFinished(BackupTaskManager::TaskState state);

  // Member variables
  std::string system_salt_;  // System salt for encryption
  Profile* profile_;         // Associated user profile
  PrefService* const prefs_; // Preference service

  scoped_refptr<ui::SelectFileDialog> select_file_dialog_; // File dialog instance
  FileDialogType file_dialog_type_ = FileDialogType::kUnspecified; // Current file dialog type

  PrefChangeRegistrar pref_change_registrar_; // Registrar for preference changes
  PrefChangeRegistrar local_state_pref_change_registrar_; // Registrar for local state preference changes

  base::WeakPtrFactory<JemaOsHandler> weak_ptr_factory_{this}; // Weak pointer factory
};

}  // namespace ash::settings

#endif  // CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_JEMAOS_HANDLER_H_