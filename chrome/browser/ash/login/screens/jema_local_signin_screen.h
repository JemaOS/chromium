// Copyright 2022 Jema Technology. All rights reserved

#ifndef CHROME_BROWSER_ASH_LOGIN_SCREENS_JEMA_LOCAL_SIGNIN_SCREEN_H_
#define CHROME_BROWSER_ASH_LOGIN_SCREENS_JEMA_LOCAL_SIGNIN_SCREEN_H_

#include <string>
#include "base/functional/callback.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/ash/login/screens/base_screen.h"
#include "chrome/browser/ui/webui/ash/login/jema_local_signin_screen_handler.h"

namespace ash {

class JemaLocalSigninScreen
    : public BaseScreen {
  public:
    enum class Result { CANCEL, ACCOUNT_TYPE_SELECTION_BACK, BACK };
    
    static std::string GetResultString(Result result);
    
    JemaLocalSigninScreen(base::WeakPtr<JemaLocalSigninView> view,
                          const base::RepeatingCallback<void(Result)>& exit_callback);

    ~JemaLocalSigninScreen() override;

    JemaLocalSigninScreen(const JemaLocalSigninScreen&) = delete;
    JemaLocalSigninScreen& operator=(const JemaLocalSigninScreen&) = delete;

  private:
    void HandleCancel();
    void HandleBackToUserSelection();

    // BaseScreen:
    void ShowImpl() override;
    void HideImpl() override;
    void OnUserAction(const base::Value::List& args) override;
    bool HandleAccelerator(LoginAcceleratorAction action) override;

    base::WeakPtr<JemaLocalSigninView> view_;

    base::RepeatingCallback<void(Result)> exit_callback_;

    base::WeakPtrFactory<JemaLocalSigninScreen> weak_factory_{this};
};

}

namespace chromeos {
using ::ash::JemaLocalSigninScreen;
}
#endif // ifndef CHROME_BROWSER_ASH_LOGIN_SCREENS_JEMA_LOCAL_SIGNIN_SCREEN_H_
