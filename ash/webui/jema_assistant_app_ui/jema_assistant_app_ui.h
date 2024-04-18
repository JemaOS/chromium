#ifndef ASH_WEBUI_JEMA_ASSISTANT_APP_UI_JEMA_ASSISTANT_APP_UI_H_
#define ASH_WEBUI_JEMA_ASSISTANT_APP_UI_JEMA_ASSISTANT_APP_UI_H_

#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_controller.h"
#include "ash/webui/jema_assistant_app_ui/jema_assistant_app_ui_delegate.h"

namespace ash {

class JemaAssistantAppUI : public content::WebUIController {
 public:
  explicit JemaAssistantAppUI(content::WebUI* web_ui,
                              std::unique_ptr<JemaAssistantAppUIDelegate> delegate);
  JemaAssistantAppUI(const JemaAssistantAppUI&) = delete;
  JemaAssistantAppUI& operator=(const JemaAssistantAppUI&) = delete;
  ~JemaAssistantAppUI() override;

  JemaAssistantAppUIDelegate* delegate() { return delegate_.get(); }

private:
  std::unique_ptr<JemaAssistantAppUIDelegate> delegate_;
  WEB_UI_CONTROLLER_TYPE_DECL();
};

} // namespace ash


#endif  // JEMAOS_ASH_WEBUI_JEMA_ASSISTANT_APP_UI_JEMA_ASSISTANT_APP_UI_H_
