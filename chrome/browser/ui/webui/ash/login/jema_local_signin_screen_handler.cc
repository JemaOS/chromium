// Copyright 2025 jema technology. All rights reserved

#include "chrome/browser/ui/webui/ash/login/jema_local_signin_screen_handler.h"

#include "base/base64.h"
#include "base/no_destructor.h"
#include "base/strings/string_util.h"
#include "base/time/time.h"
#include "chrome/browser/ash/login/existing_user_controller.h"
#include "chrome/browser/ash/login/oobe_screen.h"
#include "chrome/browser/ash/login/screens/jema_local_signin_screen.h"
#include "chrome/browser/ash/login/startup_utils.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/ui/ash/login/login_display_host.h"
#include "chrome/grit/generated_resources.h"
#include "chromeos/ash/components/login/auth/public/cryptohome_key_constants.h"
#include "chromeos/ash/components/login/auth/public/key.h"
#include "chromeos/ash/services/auth_factor_config/password_factor_editor.h"
#include "chromeos/ash/services/auth_factor_config/public/mojom/auth_factor_config.mojom-shared.h"
#include "components/login/localized_values_builder.h"
#include "components/os_crypt/sync/os_crypt.h"
#include "components/prefs/pref_service.h"
#include "components/user_manager/known_user.h"
#include "jemaos/prefs/jemaos_pref_names.h"

namespace ash {

constexpr StaticOobeScreenId JemaLocalSigninView::kScreenId;

// Global storage for web login password to be used during OOBE password setup
// This allows us to capture the password from web login and use it
// automatically
static std::string g_web_login_password_for_oobe;

// JEMAOS: Auth tokens are persisted OSCrypt-encrypted in local state so the
// subscription cookie can be re-injected after every restart. The value is
// decrypted on demand; nothing sensitive is kept in plain memory longer than
// the call that uses it.
namespace {

std::string GetDecryptedPref(const char* pref_name) {
  if (!g_browser_process || !g_browser_process->local_state()) {
    return std::string();
  }
  const std::string& encoded =
      g_browser_process->local_state()->GetString(pref_name);
  std::string encrypted;
  std::string plain;
  if (encoded.empty() || !base::Base64Decode(encoded, &encrypted) ||
      !OSCrypt::DecryptString(encrypted, &plain)) {
    return std::string();
  }
  return plain;
}

void SetEncryptedPref(const char* pref_name, const std::string& value) {
  if (!g_browser_process || !g_browser_process->local_state()) {
    return;
  }
  std::string encrypted;
  if (!OSCrypt::EncryptString(value, &encrypted)) {
    LOG(ERROR) << "[JEMAOS] Failed to encrypt pref " << pref_name;
    return;
  }
  g_browser_process->local_state()->SetString(pref_name,
                                              base::Base64Encode(encrypted));
}

//---***JEMAOS BEGIN***---
// The Jema web login payload sometimes delivers the password base64-encoded
// (in a field the JS does not flag as such), while the pod and the local
// signin form always use plaintext. Decode the value only when it is valid
// base64 that decodes to printable UTF-8 text; otherwise keep it as-is.
// Valid base64 requires a length that is a multiple of 4, so genuine
// plaintext passwords of other lengths are never touched, and alphanumeric
// plaintext that happens to be base64-shaped decodes to non-printable bytes
// and is rejected as well.
std::string DecodePasswordIfBase64(const std::string& value) {
  if (value.size() < 8 || value.size() % 4 != 0) {
    return value;
  }
  std::string decoded;
  if (!base::Base64Decode(value, &decoded)) {
    return value;
  }
  if (!base::IsStringUTF8(decoded)) {
    return value;
  }
  for (const char c : decoded) {
    if (static_cast<unsigned char>(c) < 0x20) {
      return value;
    }
  }
  LOG(WARNING) << "[JEMAOS] Password arrived base64-encoded, decoded "
               << value.size() << " -> " << decoded.size() << " chars";
  return decoded;
}
//---***JEMAOS END***---

}  // namespace

void StoreJemaOSAuthTokens(const std::string& access_token,
                           const std::string& refresh_token,
                           const std::string& email) {
  if (!access_token.empty()) {
    SetEncryptedPref(jemaos::prefs::kJemaOsAuthAccessTokenEncrypted,
                     access_token);
  }
  if (!refresh_token.empty()) {
    SetEncryptedPref(jemaos::prefs::kJemaOsAuthRefreshTokenEncrypted,
                     refresh_token);
  }
  if (!email.empty()) {
    g_browser_process->local_state()->SetString(jemaos::prefs::kJemaOsAuthEmail,
                                                email);
  }
  g_browser_process->local_state()->SetInt64(
      jemaos::prefs::kJemaOsAuthIssuedAt,
      base::Time::Now().ToDeltaSinceWindowsEpoch().InSeconds());
  LOG(INFO) << "[JEMAOS] Stored auth tokens (access " << access_token.length()
            << " chars, refresh " << refresh_token.length() << " chars)";
}

void ClearJemaOSAuthTokens() {
  if (!g_browser_process || !g_browser_process->local_state()) {
    return;
  }
  g_browser_process->local_state()->ClearPref(
      jemaos::prefs::kJemaOsAuthAccessTokenEncrypted);
  g_browser_process->local_state()->ClearPref(
      jemaos::prefs::kJemaOsAuthRefreshTokenEncrypted);
  g_browser_process->local_state()->ClearPref(jemaos::prefs::kJemaOsAuthEmail);
  g_browser_process->local_state()->ClearPref(
      jemaos::prefs::kJemaOsAuthIssuedAt);
}

std::string GetJemaOSAccessToken() {
  return GetDecryptedPref(jemaos::prefs::kJemaOsAuthAccessTokenEncrypted);
}

std::string GetJemaOSRefreshToken() {
  return GetDecryptedPref(jemaos::prefs::kJemaOsAuthRefreshTokenEncrypted);
}

// Legacy helper kept for compatibility with the login flow.
void StoreJemaOSAccessToken(const std::string& token) {
  if (!token.empty()) {
    SetEncryptedPref(jemaos::prefs::kJemaOsAuthAccessTokenEncrypted, token);
  }
}

// Helper functions to manage the web login password
void StoreWebLoginPasswordForOOBE(const std::string& password) {
  g_web_login_password_for_oobe = password;
  LOG(WARNING)
      << "[JEMAOS] Stored web login password for OOBE auto-fill, length: "
      << password.length();
}

std::string GetAndClearWebLoginPasswordForOOBE() {
  std::string password = std::move(g_web_login_password_for_oobe);
  g_web_login_password_for_oobe.clear();
  LOG(WARNING)
      << "[JEMAOS] Retrieved and cleared web login password for OOBE, length: "
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

JemaLocalSigninScreenHandler::~JemaLocalSigninScreenHandler() = default;

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
  ash::CrosSettings::Get()->GetBoolean(ash::kAccountsPrefShowUserNamesOnSignIn,
                                       &show_users_on_signin);
  data.Set("showUsersOnSignin", show_users_on_signin);
  ShowInWebUI(std::move(data));
}

void JemaLocalSigninScreenHandler::HandleCompleteAuth(
    const bool newUser,
    const std::string& username,
    const std::string& password_raw,
    const std::string& access_token,
    const std::string& refresh_token) {
  // JEMAOS: Persist the auth tokens (access + optional refresh) encrypted in
  // local state, so the subscription cookie survives restarts and can be
  // refreshed without touching UserContext (which crashes local Flint
  // accounts via the OAuth/Gaia flow).
  if (!access_token.empty()) {
    StoreJemaOSAuthTokens(access_token, refresh_token, username);
  }

  //---***JEMAOS BEGIN***---
  // The Jema web login payload may deliver the password base64-encoded
  // without a reliable flag (proven: online accounts arrived here as
  // 12-char base64 of a 9-char password). Decode only provable base64
  // (see DecodePasswordIfBase64); plaintext passes through unchanged, so
  // the cryptohome vault always seals the real password and the pod login
  // keeps working.
  std::string password = DecodePasswordIfBase64(password_raw);
  //---***JEMAOS END***---

  LOG(WARNING) << "[JEMAOS] HandleCompleteAuth called for user: " << username
               << ", newUser: " << newUser
               << ", password length: " << password.length()
               << ", password: [redacted]";

  // Ignore duplicate calls with empty password (JavaScript may call twice)
  if (password.empty()) {
    LOG(WARNING) << "[JEMAOS] Ignoring HandleCompleteAuth with empty password "
                 << "(likely duplicate call from JavaScript)";
    return;
  }

  if (username.empty()) {
    SetErrorState(username, static_cast<int>(
                                JEMA_LOCAL_SIGNIN_ERROR_STATE::BAD_USERNAME));
    return;
  }
  // JEMAOS: Determine whether the account already exists from local known-user
  // state (authoritative). Do NOT trust the JS `newUser` flag for the online
  // flow: the auth host sends newUser=true on a re-login of an existing online
  // Jema account (its isExistedUser_ is only set for a forced read-only-email
  // re-auth), which would otherwise reject the sign-in below as "signup of an
  // existing account" and make re-login impossible (e.g. after the sign-out
  // triggered by a display-language change).
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

  // An existing account always signs in; a new account is created only when
  // the user actually asked to sign up (preserves the "wrong username" error
  // on the local sign-in screen for a non-existent account).
  bool effective_new_user;
  if (exist) {
    effective_new_user = false;
  } else {
    if (!newUser) {
      // signin a non-existent account
      SetErrorState(
          username,
          static_cast<int>(
              JEMA_LOCAL_SIGNIN_ERROR_STATE::BAD_USERNAME_OR_PASSWORD_ERROR));
      return;
    }
    effective_new_user = true;
  }

  if (effective_new_user &&
      auth::PasswordFactorEditor::CheckLocalPasswordComplexity(password) !=
          auth::mojom::PasswordComplexity::kOk) {
    SetErrorState(
        username,
        static_cast<int>(
            JEMA_LOCAL_SIGNIN_ERROR_STATE::BAD_AUTH_PASSWORD_TOO_SHORT));
    return;
  }

  if (LoginDisplayHost::default_host()) {
    LoginDisplayHost::default_host()->SetDisplayEmail(username);
  }

  DoCompleteLogin(effective_new_user, username, password);
}

void JemaLocalSigninScreenHandler::DoCompleteLogin(
    const bool newUser,
    const std::string& username,
    const std::string& password) {
  user_manager::KnownUser known_user(g_browser_process->local_state());
  //---***JEMAOS BEGIN***---
  // Distinguish Jema ONLINE accounts from local (Flint) accounts: both arrive
  // here via completeFtAuthentication, but online accounts use their real
  // email while local accounts live under @jemaos.local. Online accounts must
  // get the "jema_id_" gaia prefix and the kJemaAccount type, otherwise they
  // are classified as local everywhere (no preinstalled PWAs, cloud backup
  // greyed out, subscription never consulted).
  const bool is_online_account =
      username.find('@') != std::string::npos &&
      !base::EndsWith(username, "@jemaos.local",
                      base::CompareCase::INSENSITIVE_ASCII);
  AccountId account_id;
  if (is_online_account) {
    account_id = known_user.GetAccountId(username, "jema_id_" + username,
                                         AccountType::GOOGLE);
  } else {
    // Use AccountType::GOOGLE for cryptohome compatibility (allows password
    // auth factors) Flint accounts are identified by gaia_id prefix "ft_id_*"
    // and UserType::kFlintAccount
    const std::string userId = CreateJemaLocalAccountID(username);
    account_id = known_user.GetAccountId(username, "ft_id_" + userId,
                                         AccountType::GOOGLE);
  }
  UserContext user_context(is_online_account
                               ? user_manager::UserType::kJemaAccount
                               : user_manager::UserType::kFlintAccount,
                           account_id);
  Key key(password);
  key.SetLabel(kCryptohomeGaiaKeyLabel);
  user_context.SetKey(key);
  user_context.SetJemaLocalPasswordInput(LocalPasswordInput{password});
  user_context.SetAuthFlow(is_online_account
                               ? UserContext::AUTH_FLOW_JEMA_ONLINE
                               : UserContext::AUTH_FLOW_FLINT_ACCOUNT);
  user_context.SetIsUsingOAuth(false);
  //---***JEMAOS END***---

  LOG(WARNING) << "[JEMAOS] DoCompleteLogin - username: " << username
               << ", newUser: " << newUser << ", account_id: " << account_id
               << ", password: [redacted]"
               << ", key label: " << key.GetLabel()
               << ", auth_flow: " << user_context.GetAuthFlow();

  // JemaOS: the local signin screen is reachable from the OOBE network
  // screen, which is shown *before* the device-disabled check that normally
  // marks OOBE as complete. When a local account login completes here, the
  // remaining OOBE screens (EULA, update, device-disabled check) are skipped
  // entirely, so prefs::kOobeComplete would never be set and the device would
  // boot back into OOBE after sign-out instead of showing the login screen.
  // Mark OOBE as completed now: a finished local account login is a finished
  // setup. This is reached only after the password has been verified.
  if (!StartupUtils::IsOobeCompleted()) {
    StartupUtils::MarkOobeCompleted();
  }

  if (newUser) {
    // Store password for OOBE auto-fill
    StoreWebLoginPasswordForOOBE(password);

    // Ensure future logins don't force online signin - allow local password
    user_manager::KnownUser known_user_settings(
        g_browser_process->local_state());
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
      SetErrorState(
          username,
          static_cast<int>(
              JEMA_LOCAL_SIGNIN_ERROR_STATE::BAD_USERNAME_OR_PASSWORD_ERROR));
    }
  }
}

void JemaLocalSigninScreenHandler::Reset() {
  CallExternalAPI("reset");
}

void JemaLocalSigninScreenHandler::SetErrorState(const std::string& username,
                                                 int errorState) {
  CallExternalAPI("setErrorState", username, errorState);
}

base::WeakPtr<JemaLocalSigninView> JemaLocalSigninScreenHandler::AsWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

}  // namespace ash
