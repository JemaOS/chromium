// Copyright 2025 jema technology. All rights reserved

#include "chrome/browser/ui/webui/ash/login/jema_local_signin_screen_handler.h"

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
    const std::string& username, const std::string& password) {
  if (username.empty()) {
    SetErrorState(username,
        static_cast<int>(JEMA_LOCAL_SIGNIN_ERROR_STATE::BAD_USERNAME));
    return;
  }
  if (password.empty()) {
    SetErrorState(username,
        static_cast<int>(JEMA_LOCAL_SIGNIN_ERROR_STATE::BAD_AUTH_PASSWORD));
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
  const AccountId account_id(known_user.GetAccountId(
        username, "ft_id_" + userId , AccountType::FLINT_ACCOUNT));
  UserContext user_context(
      user_manager::UserType::kFlintAccount, account_id);
  Key key(password);
  key.SetLabel(kCryptohomeGaiaKeyLabel);
  user_context.SetKey(key);
  user_context.SetJemaLocalPasswordInput(LocalPasswordInput{password});
  user_context.SetAuthFlow(UserContext::AUTH_FLOW_FLINT_ACCOUNT);
  user_context.SetIsUsingOAuth(false);
  if (newUser) {
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
