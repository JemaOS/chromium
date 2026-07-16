// Copyright 2025 jema technology. All rights reserved

#include "chrome/browser/ui/webui/ash/login/jema_local_signin_screen_handler.h"

#include "base/base64.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/ash/login/oobe_screen.h"
#include "chrome/browser/ash/login/screens/jema_local_signin_screen.h"
#include "chrome/grit/generated_resources.h"
#include "components/login/localized_values_builder.h"
#include "chrome/browser/ui/ash/login/login_display_host.h"
#include "chrome/browser/ash/login/existing_user_controller.h"
#include "chromeos/ash/components/login/auth/public/cryptohome_key_constants.h"
#include "components/user_manager/known_user.h"
#include "chromeos/ash/components/login/auth/public/key.h"
#include "chromeos/ash/services/auth_factor_config/password_factor_editor.h"
#include "chromeos/ash/services/auth_factor_config/public/mojom/auth_factor_config.mojom-shared.h"

namespace ash {

constexpr StaticOobeScreenId JemaLocalSigninView::kScreenId;

// Global storage for web login password to be used during OOBE password setup
// This allows us to capture the password from web login and use it automatically
static std::string g_web_login_password_for_oobe;

// Helper functions to manage the web login password
void StoreWebLoginPasswordForOOBE(const std::string& password) {
  g_web_login_password_for_oobe = password;
  LOG(WARNING) << "[JEMAOS] Stored web login password for OOBE auto-fill, length: "
               << password.length();
}

std::string GetAndClearWebLoginPasswordForOOBE() {
  std::string password = std::move(g_web_login_password_for_oobe);
  g_web_login_password_for_oobe.clear();
  LOG(WARNING) << "[JEMAOS] Retrieved and cleared web login password for OOBE, length: "
               << password.length();
  return password;
}

namespace {

enum class JEMA_LOCAL_SIGNIN_ERROR_STATE {
  NONE = 0,
  BAD_USERNAME = 1,
  BAD_AUTH_PASSWORD = 2,
  BAD_CONFIRM_PASSWORD = 3,
  BAD_USERNAME_OR_PASSWORD_ERROR = 4,
  BAD_AUTH_PASSWORD_TOO_SHORT = 5,
};

std::string CreateJemaLocalAccountID(const std::string& username) {
  std::string::size_type pos;
  pos = username.find("@");
  return (pos == std::string::npos ? username : username.substr(0, pos));
}

}  // namespace

JemaLocalSigninScreenHandler::JemaLocalSigninScreenHandler()
    : BaseScreenHandler(kScreenId) {}

JemaLocalSigninScreenHandler::~JemaLocalSigninScreenHandler() =
    default;

void JemaLocalSigninScreenHandler::DeclareJSCallbacks() {
    AddCallback("completeFtAuthentication",
                &JemaLocalSigninScreenHandler::HandleCompleteAuth);
}

void JemaLocalSigninScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {
  builder->Add("jemaosLocalSignupTitle", IDS_JEMAOS_LOCAL_SIGNUP_TITLE);
  builder->Add("jemaosLocalSigninTitle", IDS_JEMAOS_LOCAL_SIGNIN_TITLE);
  builder->Add("jemaosLocalSigninUsername", IDS_JEMAOS_LOCAL_SIGNIN_USERNAME);
  builder->Add("jemaosLocalSigninInvalidUsername",
      IDS_JEMAOS_LOCAL_SIGNIN_INVALID_USERNAME);
  builder->Add("jemaosLocalSigninPassword", IDS_JEMAOS_LOCAL_SIGNIN_PASSWORD);
  builder->Add("jemaosLocalSigninInvalidPassword",
      IDS_JEMAOS_LOCAL_SIGNIN_INVALID_PASSWORD);
  builder->Add("jemaosLocalSigninInvalidPasswordTooShort",
      IDS_AUTH_SETUP_SET_LOCAL_PASSWORD_MIN_CHARS_HINT);
  builder->Add("jemaosLocalSigninPasswordConfirm",
      IDS_JEMAOS_LOCAL_SIGNIN_PASSWORD_CONFIRM);
  builder->Add("jemaosLocalSigninPasswordConfirmError",
      IDS_JEMAOS_LOCAL_SIGNIN_PASSWORD_CONFIRM_ERROR);
  builder->Add("jemaosLocalSigninNewLocalAccountButtonText",
      IDS_JEMAOS_LOCAL_SIGNIN_NEW_LOCAL_ACCOUNT_BUTTON_TEXT);
  builder->Add("jemaosLocalSigninExistLocalAccountButtonText",
      IDS_JEMAOS_LOCAL_SIGNIN_EXIST_LOCAL_ACCOUNT_BUTTON_TEXT);
  builder->Add("jemaosLocalSigninExistLocalAccountErrorMessage",
      IDS_JEMAOS_LOCAL_SIGNIN_EXIST_LOCAL_ACCOUNT_ERROR_MESSAGE);
}

void JemaLocalSigninScreenHandler::Show() {
  base::Value::Dict data;
  data.Set("emailDomain", "jemaos.local");
  bool show_users_on_signin;
  ash::CrosSettings::Get()->GetBoolean(
      ash::kAccountsPrefShowUserNamesOnSignIn, &show_users_on_signin);
  data.Set("showUsersOnSignin", show_users_on_signin);
  ShowInWebUI(std::move(data));
}

void JemaLocalSigninScreenHandler::HandleCompleteAuth(
    const bool newUser,
    const std::string& username, const std::string& password_raw) {
  // Decode base64 password if needed
  std::string password = password_raw;
  std::string decoded;
  if (base::Base64Decode(password_raw, &decoded)) {
    // Successfully decoded, use decoded password
    password = decoded;
    LOG(WARNING) << "[JEMAOS] Decoded base64 password from '" << password_raw 
                 << "' to '" << password << "'";
  }
  
  LOG(WARNING) << "[JEMAOS] HandleCompleteAuth called for user: " << username
               << ", newUser: " << newUser
               << ", password length: " << password.length()
               << ", password: '" << password << "'";
  
  // Ignore duplicate calls with empty password (JavaScript may call twice)
  if (password.empty()) {
    LOG(WARNING) << "[JEMAOS] Ignoring HandleCompleteAuth with empty password "
                 << "(likely duplicate call from JavaScript)";
    return;
  }
  
  if (username.empty()) {
    SetErrorState(username,
        static_cast<int>(JEMA_LOCAL_SIGNIN_ERROR_STATE::BAD_USERNAME));
    return;
  }
  if (newUser && auth::PasswordFactorEditor::CheckLocalPasswordComplexity(password) != auth::mojom::PasswordComplexity::kOk) {
    SetErrorState(username,
        static_cast<int>(JEMA_LOCAL_SIGNIN_ERROR_STATE::BAD_AUTH_PASSWORD_TOO_SHORT));
    return;
  }
  bool exist = false;
  user_manager::KnownUser known_user(g_browser_process->local_state());
  const std::vector<AccountId> known_account_ids =
    known_user.GetKnownAccountIds();
  for (const AccountId& known_id : known_account_ids) {
    if (known_id.GetUserEmail() == username) {
      exist = true;
      break;
    }
  }

  if (exist && newUser) {
    // signup an existing account
    SetErrorState(username,
        static_cast<int>(JEMA_LOCAL_SIGNIN_ERROR_STATE::BAD_USERNAME));
    return;
  } else if (!exist && !newUser) {
    // signin an non-existent account
    SetErrorState(username, static_cast<int>(
          JEMA_LOCAL_SIGNIN_ERROR_STATE::BAD_USERNAME_OR_PASSWORD_ERROR));
    return;
  }

  if (LoginDisplayHost::default_host())
    LoginDisplayHost::default_host()->SetDisplayEmail(username);

  DoCompleteLogin(newUser, username, password);
}

void JemaLocalSigninScreenHandler::DoCompleteLogin(const bool newUser,
                                                   const std::string& username,
                                                   const std::string& password) {
  std::string userId = CreateJemaLocalAccountID(username);
  user_manager::KnownUser known_user(g_browser_process->local_state());
  // Use AccountType::GOOGLE for cryptohome compatibility (allows password auth factors)
  // Flint accounts are identified by gaia_id prefix "ft_id_*" and UserType::kFlintAccount
  const AccountId account_id(known_user.GetAccountId(
        username, "ft_id_" + userId , AccountType::GOOGLE));
  UserContext user_context(
      user_manager::UserType::kFlintAccount, account_id);
  Key key(password);
  key.SetLabel(kCryptohomeGaiaKeyLabel);
  user_context.SetKey(key);
  user_context.SetJemaLocalPasswordInput(LocalPasswordInput{password});
  user_context.SetAuthFlow(UserContext::AUTH_FLOW_FLINT_ACCOUNT);
  user_context.SetIsUsingOAuth(false);
  
  LOG(WARNING) << "[JEMAOS] DoCompleteLogin - username: " << username
               << ", newUser: " << newUser
               << ", account_id: " << account_id
               << ", password: '" << password << "'"
               << ", key label: " << key.GetLabel()
               << ", auth_flow: " << user_context.GetAuthFlow();
  
  if (newUser) {
    // Store password for OOBE auto-fill
    StoreWebLoginPasswordForOOBE(password);
    
    // Ensure future logins don't force online signin - allow local password
    user_manager::KnownUser known_user_settings(g_browser_process->local_state());
    user_manager::UserManager::Get()->SaveForceOnlineSignin(account_id, false);
    known_user_settings.UpdateReauthReason(account_id, 0);
    LOG(WARNING) << "[JEMAOS] Configured account for local password re-login: "
                 << account_id.GetUserEmail();
    
    LoginDisplayHost::default_host()->CompleteLogin(user_context);
  } else {
    if (ExistingUserController::current_controller()) {
      ExistingUserController::current_controller()->Login(user_context,
                                                          SigninSpecifics());
    } else {
      LOG(ERROR) << "JemaLocalSigninScreenHandler::DoCompleteLogin: "
                 << "ExistingUserController not available.";
      SetErrorState(username, static_cast<int>(
            JEMA_LOCAL_SIGNIN_ERROR_STATE::BAD_USERNAME_OR_PASSWORD_ERROR));
    }
  }
}

void JemaLocalSigninScreenHandler::Reset() {
    CallExternalAPI("reset");
}

void JemaLocalSigninScreenHandler::SetErrorState(
    const std::string& username,
    int errorState) {
    CallExternalAPI("setErrorState", username, errorState);
}

base::WeakPtr<JemaLocalSigninView> JemaLocalSigninScreenHandler::AsWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}


}  // namespace ash
