// Copyright (c) 2025 The Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/ui/webui/settings/ash/jemaos_handler.h"

#include "base/logging.h"
#include "base/values.h"
#include "base/files/file_util.h"
#include "base/files/file_enumerator.h"
#include "base/files/file.h"
#include "base/task/thread_pool.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "ash/shell.h"
#include "chrome/browser/lifetime/application_lifetime.h"
#include "chrome/browser/profiles/profile.h"
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
#include "chrome/browser/ash/file_manager/volume_manager.h"
#include "jemaos/switches/misc/misc_constants.h"
#include "jemaos/ui/webui/settings/ash/jemaos_handler_backup_task_manager.h"

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

}  // namespace

// JemaOsHandler::JemaOsHandler(Profile* profile, PrefService* prefs) :
//   profile_(profile), prefs_(prefs) {}
JemaOsHandler::JemaOsHandler(Profile* profile, PrefService* prefs) :
  profile_(profile), prefs_(prefs) {
  DCHECK(ash::Shell::Get());
  ash::Shell::Get()->tablet_mode_controller()->AddObserver(this);
  // TODO(johndoe) use real system_salt_ and encryptor
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
      "getShowRebootButtonInTray",
      base::BindRepeating(&JemaOsHandler::HandleGetShowRebootButtonInTray,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "setShowRebootButtonInTray",
      base::BindRepeating(&JemaOsHandler::HandleSetShowRebootButtonInTray,
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
      "createJemaosBackupScript",
      base::BindRepeating(&JemaOsHandler::HandleCreateJemaOSBackupScript,
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
}

void JemaOsHandler::OnJavascriptAllowed() {
  pref_change_registrar_.Init(prefs_);

  local_state_pref_change_registrar_.Init(g_browser_process->local_state());
  local_state_pref_change_registrar_.Add(
      jemaos::prefs::kShowRebootButtonInTray,
      base::BindRepeating(
          &JemaOsHandler::OnShowRebootButtonInTrayChanged,
          base::Unretained(this)));
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

void JemaOsHandler::OnSystemSaltObtained(const std::string& system_salt) {
  system_salt_ = system_salt;
  if (IsJavascriptAllowed()) {
    FireWebUIListener("offline-auto-signin-system-salt-obtained");
  }
}

void JemaOsHandler::HandleGetIsOfflineAutoSigninEnabled(
    const base::Value::List& args) {
  AllowJavascript();
  CHECK(args.size());
  const base::Value& callback_id = args[0];
  PrefService* prefs = g_browser_process->local_state();
  const std::string& password =
    prefs->GetString(jemaos::prefs::kOfflineAutoSigninPassword);
  const std::string& account_id_key =
    prefs->GetString(jemaos::prefs::kOfflineAutoSigninAccountIdKey);
  base::Value response(base::Value::Type::DICT);
  response.SetBoolKey("enabled", !account_id_key.empty() && !password.empty());
  const user_manager::User* user =
      ProfileHelper::Get()->GetUserByProfile(profile_);
  if (!user->GetAccountId().HasAccountIdKey() || !user->IsFlintAccountUser()) {
    response.SetBoolKey("is_current_user", false);
  }
  if (user->GetAccountId().GetAccountIdKey() == account_id_key) {
    response.SetBoolKey("is_current_user", true);
  } else {
    response.SetBoolKey("is_current_user", false);
  }
  response.SetBoolKey("system_salt_obtained", !system_salt_.empty());
  ResolveJavascriptCallback(callback_id, response);
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

void JemaOsHandler::OnShowRebootButtonInTrayChanged() {
  PrefService* prefs = g_browser_process->local_state();
  bool showReboot = prefs->GetBoolean(jemaos::prefs::kShowRebootButtonInTray);
  FireWebUIListener("show-reboot-button-in-tray-changed",
                    base::Value(showReboot));
}

void JemaOsHandler::HandleSetShowRebootButtonInTray(
    const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool showReboot = args[0].GetBool();
  PrefService* prefs = g_browser_process->local_state();
  prefs->SetBoolean(jemaos::prefs::kShowRebootButtonInTray, showReboot);
}

void JemaOsHandler::HandleGetShowRebootButtonInTray(
    const base::Value::List& args) {
  AllowJavascript();

  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  PrefService* prefs = g_browser_process->local_state();
  bool showReboot = prefs->GetBoolean(jemaos::prefs::kShowRebootButtonInTray);
  ResolveJavascriptCallback(base::Value(callback_id),
                            base::Value(showReboot));
}

void JemaOsHandler::OnShowRotateScreenButtonChanged() {
  PrefService* prefs = g_browser_process->local_state();
  bool showRotate = prefs->GetBoolean(jemaos::prefs::kShowRotateScreenButton);
  FireWebUIListener("show-rotate-screen-button-changed",
                    base::Value(showRotate));
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
  base::Value response(base::Value::Type::DICT);
  bool tpm_fallback = prefs->GetBoolean(jemaos::prefs::kForceTpmFallback);
  bool current_tpm_fallback =
    prefs->GetBoolean(jemaos::prefs::kCurrentForceTpmFallback);
  response.SetBoolKey("current", current_tpm_fallback);
  response.SetBoolKey("pref", tpm_fallback);
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
  base::Value response(base::Value::Type::DICT);
  bool tpm_fallback = prefs->GetBoolean(jemaos::prefs::kForceTpmFallback);
  bool current_tpm_fallback =
    prefs->GetBoolean(jemaos::prefs::kCurrentForceTpmFallback);
  response.SetBoolKey("current", current_tpm_fallback);
  response.SetBoolKey("pref", tpm_fallback);
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
      chrome::FindBrowserWithWebContents(web_ui()->GetWebContents());
  base::FilePath default_path =
    file_manager::util::GetDownloadsFolderForProfile(profile_);
  file_dialog_type_ = FileDialogType::kLibwidevine;
  select_file_dialog_->SelectFile(
      ui::SelectFileDialog::SELECT_OPEN_FILE,
      l10n_util::GetStringUTF16(
        IDS_PASSWORD_MANAGER_UI_SELECT_FILE),
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
    NOTREACHED();
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

void JemaOsHandler::FileSelected(const base::FilePath& path,
                                 int /*index*/,
                                 void* /*params*/) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  select_file_dialog_ = nullptr;
  switch (file_dialog_type_) {
    case FileDialogType::kLibwidevine:
      OnLibwidevineFileSelected(path);
      break;
    case FileDialogType::kBackup:
      OnBackupFileSelected(path);
      break;
    case FileDialogType::kUnspecified:
      NOTREACHED();
      break;
  }
}

void JemaOsHandler::FileSelectionCanceled(void* params) {
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
      break;
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
        base::FilePath(jemaos::constants::kJemaOSBackupScriptPath)),
      base::BindOnce(&JemaOsHandler::OnJemaOSBackupScriptChecked,
                     weak_ptr_factory_.GetWeakPtr(), callback_id));
}

void JemaOsHandler::OnJemaOSBackupScriptChecked(const std::string& callback_id,
                                                bool is_backup_supported) {
  ResolveJavascriptCallback(
      base::Value(callback_id), base::Value(is_backup_supported));
}

void JemaOsHandler::HandleCreateJemaOSBackupScript(
    const base::Value::List& args) {
  AllowJavascript();
  CHECK_EQ(1u, args.size());
  const std::string& callback_id = args[0].GetString();
  
  // Create backup script content
  const std::string script_content = 
      "#!/bin/bash\n"
      "EMAIL=\"$1\"\n"
      "PASSWORD=\"$2\"\n"
      "BACKUP_FILE=\"$3\"\n"
      "PROFILE_PATH=\"${4:-/home/chronos/user}\"\n"
      "\n"
      "echo \"JemaOS Backup starting...\"\n"
      "echo \"Email: $EMAIL\"\n"
      "echo \"Target: $BACKUP_FILE\"\n"
      "\n"
      "TARGET_DIR=$(dirname \"$BACKUP_FILE\")\n"
      "mkdir -p \"$TARGET_DIR\"\n"
      "TEMP_DIR=\"/tmp/jemaos_backup_$$\"\n"
      "mkdir -p \"$TEMP_DIR\"\n"
      "\n"
      "echo \"JemaOS System Backup\" > \"$TEMP_DIR/backup_info.txt\"\n"
      "echo \"User: $EMAIL\" >> \"$TEMP_DIR/backup_info.txt\"\n"
      "echo \"Date: $(date)\" >> \"$TEMP_DIR/backup_info.txt\"\n"
      "\n"
      "if [ -d \"$PROFILE_PATH/Downloads\" ]; then\n"
      "    mkdir -p \"$TEMP_DIR/Downloads\"\n"
      "    cp -r \"$PROFILE_PATH/Downloads\"/* \"$TEMP_DIR/Downloads/\" 2>/dev/null || true\n"
      "fi\n"
      "\n"
      "if [ -d \"$PROFILE_PATH\" ]; then\n"
      "    mkdir -p \"$TEMP_DIR/profile\"\n"
      "    for file in \"Preferences\" \"Bookmarks\" \"Local State\"; do\n"
      "        if [ -f \"$PROFILE_PATH/$file\" ]; then\n"
      "            cp \"$PROFILE_PATH/$file\" \"$TEMP_DIR/profile/\" 2>/dev/null || true\n"
      "        fi\n"
      "    done\n"
      "fi\n"
      "\n"
      "if [ -n \"$PASSWORD\" ] && [ \"$PASSWORD\" != \"undefined\" ]; then\n"
      "    tar czf - -C \"$TEMP_DIR\" . | openssl enc -aes-256-cbc -salt -pass pass:\"$PASSWORD\" > \"$BACKUP_FILE\"\n"
      "else\n"
      "    tar czf \"$BACKUP_FILE\" -C \"$TEMP_DIR\" .\n"
      "fi\n"
      "\n"
      "rm -rf \"$TEMP_DIR\"\n"
      "\n"
      "if [ -f \"$BACKUP_FILE\" ]; then\n"
      "    echo \"Backup completed: $BACKUP_FILE\"\n"
      "    exit 0\n"
      "else\n"
      "    echo \"Backup failed\"\n"
      "    exit 1\n"
      "fi\n";
  
  // Use ThreadPool to create the file directly
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce([](const std::string& content) -> bool {
        // Check if script already exists
        base::FilePath script_path("/usr/bin/jemaos-backup");
        if (base::PathExists(script_path)) {
          LOG(INFO) << "Backup script already exists at " << script_path;
          return true;  // Already exists, consider it success
        }
        
        // Try to write directly to /usr/bin (unlikely to work in ChromeOS)
        if (base::WriteFile(script_path, content)) {
          base::SetPosixFilePermissions(script_path, 0755);
          LOG(INFO) << "Created backup script at " << script_path;
          return true;
        }
        
        // Fallback: write to /tmp first
        base::FilePath temp_path("/tmp/jemaos-backup");
        if (base::WriteFile(temp_path, content)) {
          base::SetPosixFilePermissions(temp_path, 0755);
          
          // Try to move to final location (may also fail without sudo)
          if (base::Move(temp_path, script_path)) {
            LOG(INFO) << "Created backup script via temp file";
            return true;
          }
          
          // If move fails, at least log where we put it
          LOG(WARNING) << "Created backup script in /tmp, move to /usr/bin failed";
          return false;
        }
        
        LOG(ERROR) << "Failed to create backup script";
        return false;
      }, script_content),
      base::BindOnce([](const std::string& callback_id, 
                        base::WeakPtr<JemaOsHandler> handler,
                        bool success) {
        if (handler) {
          LOG(INFO) << "Backup script creation result: " << success;
          handler->ResolveJavascriptCallback(
              base::Value(callback_id), base::Value(success));
        }
      }, callback_id, weak_ptr_factory_.GetWeakPtr()));
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
      chrome::FindBrowserWithWebContents(web_ui()->GetWebContents());

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
        IDS_PASSWORD_MANAGER_UI_SELECT_FILE),
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
      NOTREACHED();
      break;
  }
  ResolveJavascriptCallback(callback_id, base::Value(state_str));
}


}  // namespace ash::settings
