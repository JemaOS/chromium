#include "ash/webui/jema_assistant_app_ui/jema_assistant_app_ui.h"

#include "ash/bubble/bubble_constants.h"
#include "ash/public/cpp/assistant/assistant_state.h"
#include "ash/webui/grit/ash_jema_assistant_app_resources.h"
#include "ash/webui/grit/ash_jema_assistant_app_resources_map.h"
#include "ash/webui/jema_assistant_app_ui/jema_assistant_app_webui_handler.h"
#include "ash/webui/jema_assistant_app_ui/url_constants.h"
#include "chrome/browser/ash/system_web_apps/apps/system_web_app_install_utils.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "content/public/browser/web_ui_message_handler.h"

namespace ash {

JemaAssistantAppUI::JemaAssistantAppUI(
    content::WebUI* web_ui,
    std::unique_ptr<JemaAssistantAppUIDelegate> delegate)
    : content::WebUIController(web_ui), delegate_(std::move(delegate)) {
  auto* browser_context = web_ui->GetWebContents()->GetBrowserContext();
  content::WebUIDataSource* html_source =
      content::WebUIDataSource::CreateAndAdd(browser_context,
                                             kChromeUIJemaAssistantAppHost);
  html_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ScriptSrc,
      "script-src chrome://resources chrome://test chrome://webui-test "
      "'self';");
  html_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::WorkerSrc,
      "worker-src blob: chrome://resources 'self';");
  html_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ConnectSrc, "connect-src 'self';");
  html_source->DisableTrustedTypesCSP();
  html_source->AddResourcePath("", IDR_ASH_JEMA_ASSISTANT_APP_INDEX_HTML);
  html_source->AddResourcePaths(base::make_span(
      kAshJemaAssistantAppResources, kAshJemaAssistantAppResourcesSize));

  html_source->SetDefaultResource(IDR_ASH_JEMA_ASSISTANT_APP_INDEX_HTML);

  html_source->AddInteger("borderRadiusInLauncher", kBubbleCornerRadius);
  html_source->AddInteger("borderRadiusInBubble", kBubbleCornerRadiusForAI);
  html_source->AddBoolean(
      "isJemaOSAssistantEnabled",
      ash::AssistantState::Get()->jema_assistant_enabled().value_or(false));
  html_source->UseStringsJs();

  delegate_->PopulateLoadTimeData(html_source);

  web_ui->AddMessageHandler(std::make_unique<JemaAssistantWebUIHandler>(this));
}

JemaAssistantAppUI::~JemaAssistantAppUI() = default;

WEB_UI_CONTROLLER_TYPE_IMPL(JemaAssistantAppUI)

}  // namespace ash
