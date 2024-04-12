#include "chrome/browser/ash/web_applications/jema_assistant_app_info.h"

#include "ash/constants/ash_features.h"
#include "ash/webui/jema_assistant_app_ui/url_constants.h"
#include "ash/webui/grit/ash_jema_assistant_app_resources.h"
#include "chrome/browser/ash/web_applications/system_web_app_install_utils.h"

#include <memory>

std::unique_ptr<WebAppInstallInfo> CreateWebAppInfoForJemaAssistantApp() {
  std::unique_ptr<WebAppInstallInfo> info =
      std::make_unique<WebAppInstallInfo>();
  info->start_url = GURL(ash::kChromeUIJemaAssistantAppURL);
  info->scope = GURL(ash::kChromeUIJemaAssistantAppURL);
  info->title = u"JemaOS AI";
  web_app::CreateIconInfoForSystemWebApp(
      info->start_url,
      {{"app_icon_192.png", 192, IDR_ASH_JEMA_ASSISTANT_APP_APP_ICON_192_PNG}},
      *info);
  info->display_mode = blink::mojom::DisplayMode::kStandalone;
  info->user_display_mode = web_app::mojom::UserDisplayMode::kStandalone;

  return info;
}

JemaAssistantAppDelegate::JemaAssistantAppDelegate(Profile* profile)
    : ash::SystemWebAppDelegate(ash::SystemWebAppType::JEMA_ASSISTANT,
                                    "JemaAssistant",
                                    GURL(ash::kChromeUIJemaAssistantAppURL),
                                    profile) {}

bool JemaAssistantAppDelegate::ShouldCaptureNavigations() const  {
  return true;
}

std::unique_ptr<WebAppInstallInfo> JemaAssistantAppDelegate::GetWebAppInfo() const {
  return CreateWebAppInfoForJemaAssistantApp();
}
bool JemaAssistantAppDelegate::IsAppEnabled() const {
  return ash::features::IsJemaAssistantEnabled();
}
