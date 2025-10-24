// Copyright 2025 jema technology. All rights reserved
#include "chrome/browser/ash/login/screens/jema_local_signin_screen.h"

#include "chrome/browser/ui/webui/ash/login/jema_local_signin_screen_handler.h"

namespace ash {
namespace {

constexpr char kUserActionCancel[] = "cancel";

}

JemaLocalSigninScreen::JemaLocalSigninScreen(
    base::WeakPtr<JemaLocalSigninView> view,
    const base::RepeatingClosure& exit_callback)
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
  }
  BaseScreen::OnUserAction(args);
}

void JemaLocalSigninScreen::HandleCancel() {
    view_->Reset();
    exit_callback_.Run();
}

bool JemaLocalSigninScreen::HandleAccelerator(LoginAcceleratorAction action) {
  return false;
}

}  // namespace ash
