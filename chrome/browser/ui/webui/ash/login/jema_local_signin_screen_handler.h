// Copyright 2025 jema technology. All rights reserved

#ifndef CHROME_BROWSER_UI_WEBUI_ASH_LOGIN_JEMA_LOCAL_SIGNIN_SCREEN_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_ASH_LOGIN_JEMA_LOCAL_SIGNIN_SCREEN_HANDLER_H_

#include <string>
#include "base/memory/weak_ptr.h"
#include "chrome/browser/ui/webui/ash/login/base_screen_handler.h"

namespace ash {

class Key;

class JemaLocalSigninView {
 public:
    inline constexpr static StaticOobeScreenId kScreenId{
      "jema-local-signin", "JemaLocalSigninScreen"};

    virtual ~JemaLocalSigninView() = default;

    // Shows the contents of the screen.
    virtual void Show() = 0;

    // Clear the input fields on the screen.
    virtual void Reset() = 0;

    // Set error state.
    virtual void SetErrorState(const std::string& username, int errorState) = 0;

    virtual base::WeakPtr<JemaLocalSigninView> AsWeakPtr() = 0;
};

class JemaLocalSigninScreenHandler : public JemaLocalSigninView,
                                     public BaseScreenHandler {
 public:
    using TView = JemaLocalSigninView;

    JemaLocalSigninScreenHandler();
    ~JemaLocalSigninScreenHandler() override;

    JemaLocalSigninScreenHandler(const JemaLocalSigninScreenHandler&) =
      delete;
    JemaLocalSigninScreenHandler& operator=(
        const JemaLocalSigninScreenHandler&) = delete;

    void Show() override;
    void Reset() override;
    void SetErrorState(const std::string& username, int errorState) override;
    base::WeakPtr<JemaLocalSigninView> AsWeakPtr() override;

    void DeclareJSCallbacks() override;
    void DeclareLocalizedValues(
        ::login::LocalizedValuesBuilder* builder) override;

 private:
  void DoCompleteLogin(const bool newUser,
                       const std::string& username,
                       const std::string& password);
  void HandleCompleteAuth(const bool newUser,
                          const std::string& username,
                          const std::string& password);

  base::WeakPtrFactory<JemaLocalSigninView> weak_ptr_factory_{this};
};

}  // namespace ash

#endif  // ifndef CHROME_BROWSER_UI_WEBUI_ASH_LOGIN_JEMA_LOCAL_SIGNIN_SCREEN_HANDLER_H_
