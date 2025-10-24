// Copyright (c) 2021 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/ui/webui/settings/ash/jemaos_handler.h"

#include "base/logging.h"
#include "base/notreached.h"
#include "base/values.h"
#include "base/files/file_util.h"
#include "base/task/thread_pool.h"
#include "ash/shell.h"
#include "chrome/browser/lifetime/application_lifetime.h"
#include "chrome/browser/profiles/profile.h"
#include "chromeos/ash/components/cryptohome/error_util.h"
#include "chromeos/ash/components/cryptohome/userdataauth_util.h"
#include "components/prefs/pref_service.h"
#include "chrome/browser/browser_process.h"
#include "components/user_manager/user_manager.h"
#include "ash/wm/tablet_mode/tablet_mode_controller.h"
#include "jemaos/prefs/jemaos_pref_names.h"
#include "chrome/browser/ash/profiles/profile_helper.h"
#include "ui/base/l10n/l10n_util.h"
// #include "chromeos/cryptohome/system_salt_getter.h"
// #include "chrome/browser/ash/settings/token_encryptor.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/chrome_select_file_policy.h"
#include "content/public/browser/browser_thread.h"
#include "chrome/browser/ash/file_manager/path_util.h"
#include "chrome/grit/generated_resources.h"
#include "ui/shell_dialogs/selected_file_info.h"
#include "chrome/browser/ash/file_manager/volume_manager.h"
#include "jemaos/switches/misc/misc_constants.h"
#include "jemaos/misc/jemaos_dev_mode.h"
#include "chromeos/ash/components/login/auth/auth_factor_editor.h"
#include "chromeos/ash/components/cryptohome/auth_factor_conversions.h"

namespace ash::settings {

namespace {

bool SuitableForBackupVolume(const file_manager::Volume* volume) {
  if (!volume) {
    return false;
  }
  return volume->type() == file_manager::VOLUME_TYPE_REMOVABLE_DISK_PARTITION
    && base::StartsWith(volume->mount_path().value(),
        "/media/removable", base::CompareCase::INSENSITIVE_ASCII)
    && !volume->hidden()
    && !volume->is_read_only_removable_device()
    && !volume->is_read_only()
    && volume->has_media();
}

const char kJemaOSArcMediaAutoScanIndicatorFile[] = "/home/chronos/user/.enable_arc_media_auto_scan";

bool ArcMediaAutoScanIndicatorFileExists() {
  return base::PathExists(base::FilePath(kJemaOSArcMediaAutoScanIndicatorFile));
}

bool DeleteArcMediaAutoScanIndicatorFile() {
  return base::DeleteFile(base::FilePath(kJemaOSArcMediaAutoScanIndicatorFile));
}

bool CreateArcMediaAutoScanIndicatorFile() {
  return base::WriteFile(base::FilePath(kJemaOSArcMediaAutoScanIndicatorFile), "");
}

}  // namespace

// JemaOsHandler::JemaOsHandler(Profile* profile, PrefService* prefs) :
//   profile_(profile), prefs_(prefs) {}
JemaOsHandler::JemaOsHandler(Profile* profile, PrefService* prefs) :
  profile_(profile), prefs_(prefs) {
  DCHECK(ash::Shell::Get());
  ash::Shell::Get()->tablet_mode_controller()->AddObserver(this);
  // TODO(fangzhou) use real system_salt_ and encryptor
  // SystemSaltGetter::Get()->GetSystemSalt(base::BindOnce(
  //     &JemaOsHandler::OnSystemSaltObtained, weak_ptr_factory_.GetWeakPtr()));
  OnSystemSaltObtained("JEMAOS");
}

JemaOsHandler::~JemaOsHandler() {
  if (ash::Shell::Get()->tablet_mode_controller())
    ash::Shell::Get()->tablet_mode_controller()->RemoveObserver(this);
  if (select_file_dialog_.get())
    select_file_dialog_->ListenerDestroyed();
}

void JemaOsHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "getIsOfflineAutoSigninEnabled",
      base::BindRepeating(&JemaOsHandler::HandleGetIsOfflineAutoSigninEnabled,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
               "saveOfflineLoginPassword",
                base::BindRepeating(&JemaOsHandler::HandleSaveOfflineLoginPassword,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
               "cleanOfflineLoginPassword",
                base::BindRepeating(&JemaOsHandler::HandleCleanOfflineLoginPassword,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getShowRotateScreenButton",
      base::BindRepeating(&JemaOsHandler::HandleGetShowRotateScreenButton,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "setShowRotateScreenButton",
      base::BindRepeating(&JemaOsHandler::HandleSetShowRotateScreenButton,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getIsInTabletPhysicalState",
      base::BindRepeating(&JemaOsHandler::HandleGetIsInTabletPhysicalState,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getShowSwitchTabletLaptopButton",
      base::BindRepeating(&JemaOsHandler::HandleGetShowSwitchTabletLaptopButton,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "setShowSwitchTabletLaptopButton",
      base::BindRepeating(&JemaOsHandler::HandleSetShowSwitchTabletLaptopButton,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getIsForceTpmFallback",
      base::BindRepeating(&JemaOsHandler::HandleGetIsForceTpmFallback,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "setForceTpmFallback",
      base::BindRepeating(&JemaOsHandler::HandleSetForceTpmFallback,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "selectLibwidevineFile",
      base::BindRepeating(&JemaOsHandler::HandleSelectLibwidevineFile,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getRebootRequiredForWidevine",
      base::BindRepeating(&JemaOsHandler::HandleGetRebootRequiredForWidevine,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "toggleRebootRequiredForWidevine",
      base::BindRepeating(&JemaOsHandler::HandleToggleRebootRequiredForWidevine,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "jemaosBackupSupported",
      base::BindRepeating(&JemaOsHandler::HandleJemaOSBackupSupported,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "jemaosBackupSelectFile",
      base::BindRepeating(&JemaOsHandler::HandleJemaOSBackupSelectFile,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "jemaosBackupStarted",
      base::BindRepeating(&JemaOsHandler::HandleJemaOSBackupStarted,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getJemaosBackupState",
      base::BindRepeating(&JemaOsHandler::HandleGetJemaOSBackupState,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getArcMediaAutoScanState",
      base::BindRepeating(&JemaOsHandler::HandleGetArcMediaAutoScanState,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "setArcMediaAutoScanState",
      base::BindRepeating(&JemaOsHandler::HandleSetArcMediaAutoScanState,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "setArcMediaAutoScanStateForCurrentSession",
      base::BindRepeating(&JemaOsHandler::HandleSetArcMediaAutoScanStateForCurrentSession,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "setDevMode",
      base::BindRepeating(&JemaOsHandler::HandleSetDevMode,
                      base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getDevModeSwitchSupported",
      base::BindRepeating(&JemaOsHandler::HandleGetDevModeSwitchSupported,
                      base::Unretained(this)));
}

void JemaOsHandler::OnJavascriptAllowed() {
  pref_change_registrar_.Init(prefs_);

  local_state_pref_change_registrar_.Init(g_browser_process->local_state());
  local_state_pref_change_registrar_.Add(
      jemaos::prefs::kShowRotateScreenButton,
      base::BindRepeating(
          &JemaOsHandler::OnShowRotateScreenButtonChanged,
          base::Unretained(this)));
  local_state_pref_change_registrar_.Add(
      jemaos::prefs::kShowSwitchTabletLaptopButton,
      base::BindRepeating(
          &JemaOsHandler::OnShowSwitchTabletLaptopButtonChanged,
          base::Unretained(this)));
  local_state_pref_change_registrar_.Add(
      jemaos::prefs::kForceTpmFallback,
      base::BindRepeating(
          &JemaOsHandler::OnForceTpmFallbackChanged,
          base::Unretained(this)));
}

void JemaOsHandler::OnJavascriptDisallowed() {
  pref_change_registrar_.RemoveAll();
  local_state_pref_change_registrar_.RemoveAll();
}

void JemaOsHandler::OnShowRotateScreenButtonChanged() {
  PrefService* prefs = g_browser_process->local_state();
  bool showRotate = prefs->GetBoolean(jemaos::prefs::kShowRotateScreenButton);
  FireWebUIListener("show-rotate-screen-button-changed",
                    base::Value(showRotate));
}

void JemaOsHandler::OnSystemSaltObtained(const std::string& system_salt) {
  system_salt_ = system_salt;
  if (IsJavascriptAllowed()) {
    FireWebUIListener("offline-auto-signin-system-salt-obtained");
  }
}

void JemaOsHandler::ListAuthFactors(const AccountId& account_id, const std::string& callback_id) {
  auto client = UserDataAuthClient::Get();
  user_data_auth::ListAuthFactorsRequest request;
  *request.mutable_account_id() =
      cryptohome::CreateAccountIdentifierFromAccountId(account_id);
  client->ListAuthFactors(
      request, base::BindOnce(&JemaOsHandler::OnListAuthFactors,
                              weak_ptr_factory_.GetWeakPtr(),
                              callback_id));
}

void JemaOsHandler::OnListAuthFactors(const std::string& callback_id, std::optional<user_data_auth::ListAuthFactorsReply> reply) {
  auto error = user_data_auth::ReplyToCryptohomeError(reply);
  if (cryptohome::HasError(error)) {
    LOG(ERROR) << "Could not list auth factors " << error;
    return;
  }
  CHECK(reply.has_value());
  auth_factor_has_password_ = false;
  for (const auto& factor_with_status_proto :
       reply->configured_auth_factors_with_status()) {
    if (factor_with_status_proto.auth_factor().type() == user_data_auth::AUTH_FACTOR_TYPE_PASSWORD) {
      auth_factor_has_password_ = true;
      break;
    }
  }

  const user_manager::User* user =
      ProfileHelper::Get()->GetUserByProfile(profile_);
  PrefService* prefs = g_browser_process->local_state();
  const std::string& password =
    prefs->GetString(jemaos::prefs::kOfflineAutoSigninPassword);
  const std::string& account_id_key =
    prefs->GetString(jemaos::prefs::kOfflineAutoSigninAccountIdKey);
  base::Value::Dict response;
  response.Set("enabled", !account_id_key.empty() && !password.empty());
  if (!user->GetAccountId().HasAccountIdKey() || !user->IsFlintAccountUser()) {
    response.Set("is_current_user", false);
  }
  if (user->GetAccountId().GetAccountIdKey() == account_id_key) {
    response.Set("is_current_user", true);
  } else {
    response.Set("is_current_user", false);
  }
  response.Set("system_salt_obtained", !system_salt_.empty());
  response.Set("auth_factor_has_password", auth_factor_has_password_);
  ResolveJavascriptCallback(callback_id, response);
}

void JemaOsHandler::HandleGetIsOfflineAutoSigninEnabled(
    const base::Value::List& args) {
  AllowJavascript();
  CHECK(args.size());
  std::string callback_id = args[0].GetString();
  const user_manager::User* user =
      ProfileHelper::Get()->GetUserByProfile(profile_);
  ListAuthFactors(user->GetAccountId(), callback_id);
}

void JemaOsHandler::HandleSaveOfflineLoginPassword(
    const base::Value::List& args) {
  CHECK_EQ(2u, args.size());
  const base::Value& callback_id = args[0];
  if (!g_browser_process || system_salt_.empty()) {
    ResolveJavascriptCallback(callback_id, base::Value(false));
    return;
  }
  std::string password = args[1].GetString();
  if (password.empty()) {
    ResolveJavascriptCallback(callback_id, base::Value(false));
  }
  user_manager::User* user =
    ash::ProfileHelper::Get()->GetUserByProfile(profile_);
  const AccountId account_id = user->GetAccountId();
  if (!account_id.HasAccountIdKey()) {
    ResolveJavascriptCallback(callback_id, base::Value(false));
    return;
  }
  PrefService* prefs = g_browser_process->local_state();
  // ash::CryptohomeTokenEncryptor encryptor(system_salt_);
  // prefs->SetString(jemaos::prefs::kOfflineAutoSigninPassword,
  //    encryptor.EncryptWithSystemSalt(password));
  prefs->SetString(jemaos::prefs::kOfflineAutoSigninPassword, password);
  prefs->SetString(jemaos::prefs::kOfflineAutoSigninPasswordFormat,
      "plaintext");
  prefs->SetString(jemaos::prefs::kOfflineAutoSigninAccountIdKey,
      account_id.GetAccountIdKey());
  ResolveJavascriptCallback(callback_id, base::Value(true));
}

void JemaOsHandler::HandleCleanOfflineLoginPassword(
    const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  const base::Value& callback_id = args[0];
  if (!g_browser_process) {
    ResolveJavascriptCallback(callback_id, base::Value(false));
    return;
  }
  PrefService* prefs = g_browser_process->local_state();
  prefs->SetString(jemaos::prefs::kOfflineAutoSigninPassword, std::string());
  prefs->SetString(jemaos::prefs::kOfflineAutoSigninAccountIdKey,
      std::string());
  ResolveJavascriptCallback(callback_id, base::Value(true));
}

void JemaOsHandler::HandleSetShowRotateScreenButton(
    const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool showRotate = args[0].GetBool();
  PrefService* prefs = g_browser_process->local_state();
  prefs->SetBoolean(jemaos::prefs::kShowRotateScreenButton, showRotate);
}

void JemaOsHandler::HandleGetShowRotateScreenButton(
    const base::Value::List& args) {
  AllowJavascript();

  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  PrefService* prefs = g_browser_process->local_state();
  bool showRotate = prefs->GetBoolean(jemaos::prefs::kShowRotateScreenButton);
  ResolveJavascriptCallback(base::Value(callback_id),
                            base::Value(showRotate));
}

void JemaOsHandler::OnTabletPhysicalStateChanged() {
  if (!IsJavascriptAllowed()) {
    return;
  }
  bool is_in_tablet_physical_state =
    ash::Shell::Get()->tablet_mode_controller()->is_in_tablet_physical_state();
  FireWebUIListener("is-in-tablet-physical-state-changed",
                    base::Value(is_in_tablet_physical_state));
}

void JemaOsHandler::HandleGetIsInTabletPhysicalState(
    const base::Value::List& args) {
  AllowJavascript();

  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  bool is_in_tablet_physical_state =
    ash::Shell::Get()->tablet_mode_controller()->is_in_tablet_physical_state();
  ResolveJavascriptCallback(base::Value(callback_id),
                            base::Value(is_in_tablet_physical_state));
}

void JemaOsHandler::OnShowSwitchTabletLaptopButtonChanged() {
  PrefService* prefs = g_browser_process->local_state();
  bool showButton = prefs->GetBoolean(jemaos::prefs::kShowSwitchTabletLaptopButton);
  FireWebUIListener("show-switch-tablet-laptop-button-changed",
                    base::Value(showButton));
}

void JemaOsHandler::HandleSetShowSwitchTabletLaptopButton(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool showButton = args[0].GetBool();
  PrefService* prefs = g_browser_process->local_state();
  prefs->SetBoolean(jemaos::prefs::kShowSwitchTabletLaptopButton, showButton);
}

void JemaOsHandler::HandleGetShowSwitchTabletLaptopButton(const base::Value::List& args) {
  AllowJavascript();

  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  PrefService* prefs = g_browser_process->local_state();
  bool showButton = prefs->GetBoolean(jemaos::prefs::kShowSwitchTabletLaptopButton);
  ResolveJavascriptCallback(base::Value(callback_id),
                            base::Value(showButton));
}

void JemaOsHandler::HandleGetIsForceTpmFallback(const base::Value::List& args) {
  AllowJavascript();

  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  PrefService* prefs = g_browser_process->local_state();
  base::Value::Dict response;
  bool tpm_fallback = prefs->GetBoolean(jemaos::prefs::kForceTpmFallback);
  bool current_tpm_fallback =
    prefs->GetBoolean(jemaos::prefs::kCurrentForceTpmFallback);
  response.Set("current", current_tpm_fallback);
  response.Set("pref", tpm_fallback);
  ResolveJavascriptCallback(base::Value(callback_id), response);
}

void JemaOsHandler::HandleSetForceTpmFallback(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool tpm_fallback = args[0].GetBool();
  if (!user_manager::UserManager::Get()->IsCurrentUserOwner()) {
    VLOG(2) << "non owner user tried to set "
      << jemaos::prefs::kForceTpmFallback << ", ignored";
    return;
  }
  PrefService* prefs = g_browser_process->local_state();
  const bool necessary =
    prefs->GetBoolean(jemaos::prefs::kForceTpmFallbackNecessary);
  if (!necessary) {
    VLOG(2) << "tried to set " << jemaos::prefs::kForceTpmFallback
      << ", which is not necessary, ignored";
    return;
  }
  prefs->SetBoolean(jemaos::prefs::kForceTpmFallback, tpm_fallback);
}

void JemaOsHandler::OnForceTpmFallbackChanged() {
  PrefService* prefs = g_browser_process->local_state();
  base::Value::Dict response;
  bool tpm_fallback = prefs->GetBoolean(jemaos::prefs::kForceTpmFallback);
  bool current_tpm_fallback =
    prefs->GetBoolean(jemaos::prefs::kCurrentForceTpmFallback);
  response.Set("current", current_tpm_fallback);
  response.Set("pref", tpm_fallback);
  FireWebUIListener("force-tpm-fallback-changed", response);
}

void JemaOsHandler::HandleSelectLibwidevineFile(const base::Value::List& args) {
  CHECK_EQ(0u, args.size());
  select_file_dialog_ = ui::SelectFileDialog::Create(
      this,
      std::make_unique<ChromeSelectFilePolicy>(web_ui()->GetWebContents()));
  ui::SelectFileDialog::FileTypeInfo file_type_info;
  file_type_info.allowed_paths =
    ui::SelectFileDialog::FileTypeInfo::NATIVE_PATH;
  file_type_info.extensions.resize(1);
  file_type_info.extensions[0].push_back(FILE_PATH_LITERAL("so"));
  Browser* browser =
      chrome::FindBrowserWithTab(web_ui()->GetWebContents());
  base::FilePath default_path =
    file_manager::util::GetDownloadsFolderForProfile(profile_);
  file_dialog_type_ = FileDialogType::kLibwidevine;
  select_file_dialog_->SelectFile(
      ui::SelectFileDialog::SELECT_OPEN_FILE,
      l10n_util::GetStringUTF16(
        IDS_OS_SETTINGS_JEMAOS_SELECT_WIDEVINE_FILE_DIALOG_TITLE),
      default_path, &file_type_info, 0, base::FilePath::StringType(),
      browser->window()->GetNativeWindow(), nullptr);
}

void JemaOsHandler::HandleGetRebootRequiredForWidevine(
    const base::Value::List& args) {
  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  PrefService* prefs = g_browser_process->local_state();
  bool rebootRequired = prefs->GetBoolean(
      jemaos::prefs::kRebootRequiredForWidevine);
  ResolveJavascriptCallback(callback_id, base::Value(rebootRequired));
}

void JemaOsHandler::HandleToggleRebootRequiredForWidevine(
    const base::Value::List& args) {
  if (args.size() < 2 || !args[0].is_string() || !args[1].is_bool()) {
    VLOG(2) << "Invalid arguments for toggleRebootRequiredForWidevine";
    return;
  }
  std::string callback_id = args[0].GetString();
  bool force = args[1].GetBool();
  PrefService* prefs = g_browser_process->local_state();
  bool current = prefs->GetBoolean(jemaos::prefs::kRebootRequiredForWidevine);
  bool rebootRequired;
  if (force) {
    nextToggleRebootRequiredForWidevine_ = current;
    rebootRequired = true;
    lastToggleRebootRequiredForce_ = true;
  } else {
    if (lastToggleRebootRequiredForce_) {
      rebootRequired = nextToggleRebootRequiredForWidevine_;
    } else {
      rebootRequired = !current;
    }
    lastToggleRebootRequiredForce_ = false;
  }
  prefs->SetBoolean(jemaos::prefs::kRebootRequiredForWidevine, rebootRequired);
  ResolveJavascriptCallback(callback_id, base::Value(rebootRequired));
}

void JemaOsHandler::FileSelected(const ui::SelectedFileInfo& file,
                                 int /*index*/) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  select_file_dialog_ = nullptr;
  switch (file_dialog_type_) {
    case FileDialogType::kLibwidevine:
      OnLibwidevineFileSelected(file.path());
      break;
    case FileDialogType::kBackup:
      OnBackupFileSelected(file.path());
      break;
    case FileDialogType::kUnspecified:
      NOTREACHED();
  }
}

void JemaOsHandler::FileSelectionCanceled() {
  select_file_dialog_ = nullptr;
  switch (file_dialog_type_) {
    case FileDialogType::kLibwidevine:
      OnLibwidevineFileSelectionCanceled();
      break;
    case FileDialogType::kBackup:
      OnBackupFileSelectionCanceled();
      break;
    case FileDialogType::kUnspecified:
      NOTREACHED();
  }
}

void JemaOsHandler::OnLibwidevineFileSelected(const base::FilePath& path) {
  FireWebUIListener("jemaos-libwidevine-file-selected",
      base::Value(path.value()));
}

void JemaOsHandler::OnLibwidevineFileSelectionCanceled() {
  FireWebUIListener("jemaos-libwidevine-file-selected", base::Value());
}

void JemaOsHandler::HandleJemaOSBackupSupported(
    const base::Value::List& args) {
  AllowJavascript();
  CHECK_EQ(1u, args.size());
  const std::string& callback_id = args[0].GetString();
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(
        &base::PathExists,
        base::FilePath(jemaos::constants::kJemaOSBackupScriptDirPath)),
      base::BindOnce(&JemaOsHandler::OnJemaOSBackupScriptChecked,
                     weak_ptr_factory_.GetWeakPtr(), callback_id));
}

void JemaOsHandler::OnJemaOSBackupScriptChecked(const std::string& callback_id,
                                                bool is_backup_supported) {
  ResolveJavascriptCallback(
      base::Value(callback_id), base::Value(is_backup_supported));
}

void JemaOsHandler::HandleJemaOSBackupSelectFile(
    const base::Value::List& args) {
  DCHECK(args.size());
  std::string default_filename = args[0].GetString();
  select_file_dialog_ = ui::SelectFileDialog::Create(
      this,
      std::make_unique<ChromeSelectFilePolicy>(web_ui()->GetWebContents()));

  ui::SelectFileDialog::FileTypeInfo file_type_info;
  file_type_info.allowed_paths =
    ui::SelectFileDialog::FileTypeInfo::NATIVE_PATH;
  file_type_info.extensions.resize(1);
  file_type_info.extensions[0].push_back(FILE_PATH_LITERAL(".bak"));

  Browser* browser =
      chrome::FindBrowserWithTab(web_ui()->GetWebContents());

  auto* volume_manager = file_manager::VolumeManager::Get(profile_);
  file_manager::Volume* volume = nullptr;
  for (auto& v : volume_manager->GetVolumeList()) {
    if (SuitableForBackupVolume(v.get())) {
      volume = v.get();
      break;
    }
  }
  base::FilePath default_path = volume
    ? volume->mount_path()
    : file_manager::util::GetMyFilesFolderForProfile(profile_);
  const base::FilePath default_filepath = default_path.Append(default_filename);
  file_dialog_type_ = FileDialogType::kBackup;
  select_file_dialog_->SelectFile(
      ui::SelectFileDialog::SELECT_SAVEAS_FILE,
      l10n_util::GetStringUTF16(
        IDS_OS_SETTINGS_JEMAOS_BACKUP_SAVE_FILE_DIALOG_TITLE),
      default_filepath, &file_type_info, 0, base::FilePath::StringType(),
      browser->window()->GetNativeWindow(), nullptr);
}

void JemaOsHandler::OnBackupFileSelected(const base::FilePath& path) {
  BackupTaskManager::GetInstance()->SetBackupFile(path);
  const bool canceled = false;
  FireWebUIListener("jemaos-backup-file-selected", base::Value(canceled));
}

void JemaOsHandler::OnBackupFileSelectionCanceled() {
  const bool canceled = true;
  FireWebUIListener("jemaos-backup-file-selected", base::Value(canceled));
}

void JemaOsHandler::HandleJemaOSBackupStarted(const base::Value::List& args) {
  DCHECK_EQ(args.size(), 2u);
  std::string email = args[0].GetString();
  std::string password = args[1].GetString();

  BackupTaskManager::GetInstance()->SetCallback(
      base::BindRepeating(&JemaOsHandler::OnBackupTaskFinished,
                          weak_ptr_factory_.GetWeakPtr()));
  BackupTaskManager::GetInstance()->StartTask(profile_, email, password);
}

void JemaOsHandler::OnBackupTaskFinished(BackupTaskManager::TaskState state) {
  if (state == BackupTaskManager::TaskState::kRunning
      || state == BackupTaskManager::TaskState::kIdle) {
    return;
  }
  FireWebUIListener("jemaos-backup-task-finished",
      base::Value(state == BackupTaskManager::TaskState::kFinished));
}

void JemaOsHandler::HandleGetJemaOSBackupState(const base::Value::List& args) {
  AllowJavascript();
  CHECK_EQ(1u, args.size());
  const base::Value& callback_id = args[0];
  BackupTaskManager::TaskState state =
    BackupTaskManager::GetInstance()->GetTaskState();
  std::string state_str;
  switch (state) {
    case BackupTaskManager::TaskState::kIdle:
    case BackupTaskManager::TaskState::kFinished:
    case BackupTaskManager::TaskState::kFailed:
      state_str = "not_running";
      break;
    case BackupTaskManager::TaskState::kRunning:
      state_str = "running";
      break;
    default:
      NOTREACHED_IN_MIGRATION();
      break;
  }
  ResolveJavascriptCallback(callback_id, base::Value(state_str));
}

void JemaOsHandler::HandleGetArcMediaAutoScanState(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  const std::string& callback_id = args[0].GetString();
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&ArcMediaAutoScanIndicatorFileExists),
      base::BindOnce(&JemaOsHandler::OnArcMediaAutoScanIndicatorFileExistenceChecked,
                    weak_ptr_factory_.GetWeakPtr(), callback_id));

}

void JemaOsHandler::OnArcMediaAutoScanIndicatorFileExistenceChecked(const std::string& callback_id, bool result) {
  const PrefService::Preference* pref = prefs_->FindPreference(jemaos::prefs::kJemaOSArcMediaAutoScanEnabled);
  int saved = 0;
  if (!pref || pref->IsDefaultValue()) {
    saved = -1;
  } else {
    const bool n = pref->GetValue()->GetBool();
    saved = n ? 1 : 0;
  }
  base::Value::Dict response;
  response.Set("enabled", result);
  response.Set("saved", saved);
  if (callback_id.empty()) {
    FireWebUIListener("jemaos-arc-media-auto-scan-changed", response);
  } else {
    ResolveJavascriptCallback(callback_id, response);
  }
}

void JemaOsHandler::HandleSetArcMediaAutoScanStateForCurrentSession(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool enabled = args[0].GetBool();
  prefs_->SetBoolean(jemaos::prefs::kJemaOSArcMediaAutoScanEnabled, enabled);
}

void JemaOsHandler::HandleSetArcMediaAutoScanState(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool enable = args[0].GetBool();
  if (!enable) {
    base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&DeleteArcMediaAutoScanIndicatorFile),
      base::BindOnce(&JemaOsHandler::OnEnableArcMediaAutoScan,
                     weak_ptr_factory_.GetWeakPtr()));
  } else {
    const std::string empty = std::string();
    base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&CreateArcMediaAutoScanIndicatorFile),
      base::BindOnce(&JemaOsHandler::OnDisableArcMediaAutoScan,
                     weak_ptr_factory_.GetWeakPtr()));
  }
}

void JemaOsHandler::OnEnableArcMediaAutoScan(bool result) {
  RefreshArcMediaAutoScanState();
}

void JemaOsHandler::OnDisableArcMediaAutoScan(bool result) {
  RefreshArcMediaAutoScanState();
}

void JemaOsHandler::RefreshArcMediaAutoScanState() {
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&ArcMediaAutoScanIndicatorFileExists),
      base::BindOnce(&JemaOsHandler::OnArcMediaAutoScanIndicatorFileExistenceChecked,
                 weak_ptr_factory_.GetWeakPtr(), ""));
}

void JemaOsHandler::OnSetDevMode(const std::string& callback_id, bool result) {
  ResolveJavascriptCallback(base::Value(callback_id), base::Value(result));
}

void JemaOsHandler::HandleSetDevMode(const base::Value::List& args) {
  CHECK_EQ(2u, args.size());
  const std::string& callback_id = args[0].GetString();
  bool enable = args[1].GetBool();
  jemaos::misc::SetDevMode(enable, base::BindOnce(&JemaOsHandler::OnSetDevMode,
                                                  weak_ptr_factory_.GetWeakPtr(),
                                                  callback_id));
}

void JemaOsHandler::HandleGetDevModeSwitchSupported(const base::Value::List& args) {
  AllowJavascript();
  CHECK_EQ(1u, args.size());
  const std::string& callback_id = args[0].GetString();
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&jemaos::misc::IsDevModeSwitchSupported),
      base::BindOnce(&JemaOsHandler::OnDevModeSwitchSupportedChecked,
                     weak_ptr_factory_.GetWeakPtr(), callback_id));
}

void JemaOsHandler::OnDevModeSwitchSupportedChecked(const std::string& callback_id, bool result) {
  ResolveJavascriptCallback(base::Value(callback_id), base::Value(result));
}

}  // namespace ash::settings
