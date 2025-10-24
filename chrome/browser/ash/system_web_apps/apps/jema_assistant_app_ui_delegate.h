#ifndef CHROME_BROWSER_ASH_WEB_APPLICATIONS_JEMA_ASSISTANT_APP_UI_DELEGATE_H_
#define CHROME_BROWSER_ASH_WEB_APPLICATIONS_JEMA_ASSISTANT_APP_UI_DELEGATE_H_

#include "ash/webui/jema_assistant_app_ui/jema_assistant_app_ui_delegate.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"

namespace content {
class WebUI;
}

class ChromeJemaAssistantAppUIDelegate : public ash::JemaAssistantAppUIDelegate {
 public:
  explicit ChromeJemaAssistantAppUIDelegate(content::WebUI* web_ui);

  ChromeJemaAssistantAppUIDelegate(const ChromeJemaAssistantAppUIDelegate&) = delete;
  ChromeJemaAssistantAppUIDelegate& operator=(const ChromeJemaAssistantAppUIDelegate&) = delete;
  ~ChromeJemaAssistantAppUIDelegate() override;

  // JemaAssistantAppUIDelegate:
  void PopulateLoadTimeData(content::WebUIDataSource* source) override;

  ash::JemaAssistantAppUIDelegate::ColorInfo GetSystemColorInfo() override;

 private:
  raw_ptr<content::WebUI> web_ui_;  // Owns |this|.
  base::WeakPtrFactory<ChromeJemaAssistantAppUIDelegate> weak_ptr_factory_{this};
};

#endif // !#ifndef CHROME_BROWSER_ASH_WEB_APPLICATIONS_JEMA_ASSISTANT_APP_UI_DELEGATE_H_
