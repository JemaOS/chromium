// Copyright 2022 Jema Technology. All rights reserved
#include "chrome/browser/ash/login/screens/jema_local_signin_screen.h"
#include "chrome/browser/ui/webui/ash/login/jema_local_signin_screen_handler.h"
#include "base/logging.h"

namespace ash {
namespace {

constexpr char kUserActionCancel[] = "cancel";
constexpr char kUserActionBackToUserSelection[] = "accountTypeSelectionBack";

}

JemaLocalSigninScreen::JemaLocalSigninScreen(
    base::WeakPtr<JemaLocalSigninView> view,
    const base::RepeatingCallback<void(Result)>& exit_callback)
    : BaseScreen(JemaLocalSigninView::kScreenId,
                 OobeScreenPriority::DEFAULT),
      view_(std::move(view)),
      exit_callback_(exit_callback) {}

JemaLocalSigninScreen::~JemaLocalSigninScreen() = default;

void JemaLocalSigninScreen::ShowImpl() {
  if (!view_)
    return;
  view_->Show();
}

void JemaLocalSigninScreen::HideImpl() {
  if (!view_)
    return;
  view_->Reset();
}

void JemaLocalSigninScreen::OnUserAction(const base::Value::List& args) {
  const std::string& action_id = args[0].GetString();
  if (action_id == kUserActionCancel) {
    HandleCancel();
    return;
  } else if (action_id == kUserActionBackToUserSelection) {
    HandleBackToUserSelection();
    return;
  }
  BaseScreen::OnUserAction(args);
}

void JemaLocalSigninScreen::HandleCancel() {
    view_->Reset();
    exit_callback_.Run(Result::CANCEL);
}

void JemaLocalSigninScreen::HandleBackToUserSelection() {
  view_->Reset();
  LOG(INFO) << "JemaLocalSigninScreen: Back to user selection";
  // Just use the exit callback - the parent controller will handle navigation
   exit_callback_.Run(Result::ACCOUNT_TYPE_SELECTION_BACK);
}

bool JemaLocalSigninScreen::HandleAccelerator(LoginAcceleratorAction action) {
  return false;
}

}  // namespace ash
