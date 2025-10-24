#include "chrome/browser/ash/system_web_apps/apps/jema_assistant_app_info.h"

#include "ash/constants/ash_features.h"
#include "ash/webui/jema_assistant_app_ui/url_constants.h"
#include "ash/webui/grit/ash_jema_assistant_app_resources.h"
#include "chrome/browser/ash/system_web_apps/apps/system_web_app_install_utils.h"
#include "chrome/browser/web_applications/web_app_install_info.h"

#include <memory>

std::unique_ptr<web_app::WebAppInstallInfo> CreateWebAppInfoForJemaAssistantApp() {
  auto start_url = GURL(ash::kChromeUIJemaAssistantAppURL);
  auto info =
      web_app::CreateSystemWebAppInstallInfoWithStartUrlAsIdentity(start_url);
  info->scope = GURL(ash::kChromeUIJemaAssistantAppURL);
  info->title = u"JemaOS AI";
  web_app::CreateIconInfoForSystemWebApp(
      info->start_url(),
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

std::unique_ptr<web_app::WebAppInstallInfo> JemaAssistantAppDelegate::GetWebAppInfo() const {
  return CreateWebAppInfoForJemaAssistantApp();
}
bool JemaAssistantAppDelegate::IsAppEnabled() const {
  return ash::features::IsJemaAssistantEnabled();
}
