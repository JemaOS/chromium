#include "chrome/browser/ash/system_web_apps/apps/jema_assistant_app_ui_delegate.h"

#include <utility>
#include <vector>

#include "ash/constants/notifier_catalogs.h"
#include "ash/display/cros_display_config.h"
#include "ash/public/cpp/new_window_delegate.h"
#include "ash/public/cpp/system_notification_builder.h"
#include "ash/public/cpp/window_properties.h"
#include "ash/shell.h"
#include "ash/system/power/power_status.h"
#include "ash/wm/desks/desk.h"
#include "ash/wm/desks/desks_controller.h"
#include "ash/wm/mru_window_tracker.h"
#include "ash/wm/window_state.h"
#include "ash/wm/window_util.h"
#include "base/base64.h"
#include "base/files/file_enumerator.h"
#include "base/files/file_util.h"
#include "base/functional/callback_helpers.h"
#include "base/strings/escape.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "base/uuid.h"
#include "chrome/browser/apps/app_service/app_service_proxy.h"
#include "chrome/browser/apps/app_service/app_service_proxy_factory.h"
#include "chrome/browser/ash/base/locale_util.h"
#include "chrome/browser/ash/file_manager/path_util.h"
#include "chrome/browser/ash/system_web_apps/color_helpers.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/aura/accessibility/automation_manager_aura.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chromeos/ash/components/audio/cras_audio_handler.h"
#include "chromeos/ash/components/network/network_handler.h"
#include "chromeos/ash/components/network/network_state.h"
#include "chromeos/ash/components/network/network_state_handler.h"
#include "chromeos/ash/components/network/network_type_pattern.h"
#include "chromeos/ash/components/network/technology_state_controller.h"
#include "chromeos/crosapi/mojom/cros_display_config.mojom.h"
#include "chromeos/ui/base/window_properties.h"
#include "components/language/core/browser/pref_names.h"
#include "components/os_crypt/sync/os_crypt.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "components/services/app_service/public/cpp/app_launch_util.h"
#include "components/services/app_service/public/cpp/app_registry_cache.h"
#include "components/services/app_service/public/cpp/app_update.h"
#include "components/services/app_service/public/cpp/intent_util.h"
#include "components/services/app_service/public/cpp/types_util.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "device/bluetooth/bluetooth_adapter.h"
#include "device/bluetooth/bluetooth_adapter_factory.h"
#include "device/bluetooth/bluetooth_device.h"
#include "jemaos/prefs/jemaos_pref_names.h"
#include "ui/accessibility/ax_enum_util.h"
#include "ui/accessibility/ax_mode.h"
#include "ui/aura/window.h"
#include "ui/chromeos/styles/cros_tokens_color_mappings.h"
#include "ui/color/color_provider.h"
#include "ui/color/color_provider_utils.h"
#include "ui/message_center/message_center.h"
#include "ui/message_center/public/cpp/notifier_id.h"
#include "ui/native_theme/native_theme.h"
#include "ui/snapshot/snapshot.h"

ChromeJemaAssistantAppUIDelegate::ChromeJemaAssistantAppUIDelegate(
    content::WebUI* web_ui)
    : web_ui_(web_ui) {
  AutomationManagerAura::GetInstance()->AddNativeClient();
}

ChromeJemaAssistantAppUIDelegate::~ChromeJemaAssistantAppUIDelegate() {
  AutomationManagerAura::GetInstance()->RemoveNativeClient();
}

struct ChromeJemaAssistantAppUIDelegate::AccessibilitySnapshotRequest {
  ToolCallback callback;
  base::WeakPtr<content::WebContents> web_contents;
  GURL url;
  std::u16string title;
  base::Value::List nodes;
  int pending = 0;
  bool completed = false;
};

void ChromeJemaAssistantAppUIDelegate::PopulateLoadTimeData(
    content::WebUIDataSource* source) {
  Profile* profile = Profile::FromWebUI(web_ui_);
  source->AddString("user", profile->GetProfileUserName());

  auto colors = GetSystemColorInfo();
  source->AddString("base_color", ui::ConvertSkColorToCSSColor(colors.base));
  source->AddString("shaded_color",
                    ui::ConvertSkColorToCSSColor(colors.shaded));
  source->AddString("header_color",
                    ui::ConvertSkColorToCSSColor(colors.header));
  source->AddString("primary_color",
                    ui::ConvertSkColorToCSSColor(colors.primary));
}

ash::JemaAssistantAppUIDelegate::ColorInfo
ChromeJemaAssistantAppUIDelegate::GetSystemColorInfo() {
  auto* native_theme = ui::NativeTheme::GetInstanceForNativeUi();
  auto* color_provider = ui::ColorProviderManager::Get().GetColorProviderFor(
      native_theme->GetColorProviderKey(nullptr));
  return {
      color_provider->GetColor(cros_tokens::kCrosSysAppBase),
      color_provider->GetColor(cros_tokens::kCrosSysAppBaseShaded),
      color_provider->GetColor(cros_tokens::kCrosSysHeader),
      color_provider->GetColor(cros_tokens::kCrosSysPrimary),
  };
}

void ChromeJemaAssistantAppUIDelegate::ExecuteSystemTool(
    const base::Value::Dict& request,
    ToolCallback callback) {
  const std::string* tool = request.FindString("tool");
  if (!tool) {
    std::move(callback).Run(base::unexpected(std::string("Missing tool name")));
    return;
  }
  if (GetPermission(request) == "deny") {
    std::move(callback).Run(
        base::unexpected(std::string("Tool denied for this agent")));
    return;
  }
  auto audited_callback = base::BindOnce(
      [](base::WeakPtr<ChromeJemaAssistantAppUIDelegate> self,
         base::Value::Dict request, ToolCallback callback, ToolResult result) {
        if (self) {
          self->AppendAuditEntry(request, result.has_value(),
                                 result.has_value() ? "ok" : result.error());
        }
        std::move(callback).Run(std::move(result));
      },
      weak_ptr_factory_.GetWeakPtr(), request.Clone(), std::move(callback));
  if (*tool == "apps.list") {
    ListApps(std::move(audited_callback));
  } else if (*tool == "apps.launch") {
    LaunchApp(request, std::move(audited_callback));
  } else if (*tool == "quicktext.open_content") {
    OpenQuickText(request, std::move(audited_callback));
  } else if (*tool == "locale.get") {
    GetLocale(std::move(audited_callback));
  } else if (*tool == "locale.change") {
    ChangeLocale(request, std::move(audited_callback));
  } else if (*tool == "display.list_modes") {
    ListDisplayModes(std::move(audited_callback));
  } else if (*tool == "display.set_mode") {
    SetDisplayMode(request, std::move(audited_callback));
  } else if (*tool == "files.search") {
    SearchFiles(request, std::move(audited_callback));
  } else if (*tool == "files.read") {
    ReadFile(request, std::move(audited_callback));
  } else if (*tool == "files.write") {
    WriteFile(request, std::move(audited_callback));
  } else if (*tool == "audio.get") {
    GetAudioState(std::move(audited_callback));
  } else if (*tool == "audio.set") {
    SetAudioState(request, std::move(audited_callback));
  } else if (*tool == "power.get") {
    GetPowerState(std::move(audited_callback));
  } else if (*tool == "notifications.show") {
    ShowNotification(request, std::move(audited_callback));
  } else if (*tool == "email.compose") {
    ComposeEmail(request, std::move(audited_callback));
  } else if (*tool == "wifi.get") {
    GetWifiState(std::move(audited_callback));
  } else if (*tool == "wifi.set_enabled") {
    SetWifiState(request, std::move(audited_callback));
  } else if (*tool == "bluetooth.get") {
    GetBluetoothState(std::move(audited_callback));
  } else if (*tool == "bluetooth.set_enabled") {
    SetBluetoothState(request, std::move(audited_callback));
  } else if (*tool == "environment.get_active_window") {
    GetActiveWindowContext(std::move(audited_callback));
  } else if (*tool == "browser.open_url") {
    OpenBrowserUrl(request, std::move(audited_callback));
  } else if (*tool == "environment.capture_active_window") {
    CaptureActiveWindow(request, std::move(audited_callback));
  } else if (*tool == "accessibility.get_active_tree") {
    GetActiveAccessibilityTree(request, std::move(audited_callback));
  } else if (*tool == "accessibility.perform_action") {
    PerformAccessibilityAction(request, std::move(audited_callback));
  } else if (*tool == "permissions.set") {
    const std::string* agent_id = request.FindString("targetAgentId");
    const std::string* target_tool = request.FindString("targetTool");
    const std::string* value = request.FindString("value");
    if (!request.FindBool("confirmed").value_or(false) || !agent_id ||
        !target_tool || !value ||
        (*value != "allow" && *value != "ask" && *value != "deny")) {
      std::move(audited_callback)
          .Run(base::unexpected(std::string("Invalid permission update")));
      return;
    }
    Profile* profile = Profile::FromWebUI(web_ui_);
    ScopedDictPrefUpdate update(profile->GetPrefs(),
                                jemaos::prefs::kJemaAssistantAgentPermissions);
    base::Value::Dict* agent = update->FindDict(*agent_id);
    if (!agent) {
      update->Set(*agent_id, base::Value::Dict());
      agent = update->FindDict(*agent_id);
    }
    agent->Set(*target_tool, *value);
    base::Value::Dict result;
    result.Set("updated", true);
    std::move(audited_callback).Run(std::move(result));
  } else {
    std::move(audited_callback)
        .Run(base::unexpected(std::string("Unsupported system tool")));
  }
}

aura::Window* ChromeJemaAssistantAppUIDelegate::GetTargetUserWindow() const {
  aura::Window* assistant =
      web_ui_->GetWebContents()->GetTopLevelNativeWindow();
  for (aura::Window* window :
       ash::Shell::Get()->mru_window_tracker()->BuildMruWindowList(
           ash::DesksMruType::kActiveDesk)) {
    if (window && window != assistant) {
      return window;
    }
  }
  aura::Window* active = ash::window_util::GetActiveWindow();
  return active == assistant ? nullptr : active;
}

content::WebContents* ChromeJemaAssistantAppUIDelegate::GetWebContentsForWindow(
    aura::Window* window) const {
  Browser* browser = window ? chrome::FindBrowserWithWindow(window) : nullptr;
  return browser && browser->tab_strip_model()
             ? browser->tab_strip_model()->GetActiveWebContents()
             : nullptr;
}

void ChromeJemaAssistantAppUIDelegate::CaptureActiveWindow(
    const base::Value::Dict& request,
    ToolCallback callback) {
  if (!request.FindBool("confirmed").value_or(false)) {
    std::move(callback).Run(
        base::unexpected(std::string("User confirmation required")));
    return;
  }
  aura::Window* target = GetTargetUserWindow();
  if (!target) {
    std::move(callback).Run(
        base::unexpected(std::string("No user window available")));
    return;
  }
  ui::GrabWindowSnapshotAsPNG(
      target, gfx::Rect(target->bounds().size()),
      base::BindOnce(&ChromeJemaAssistantAppUIDelegate::OnActiveWindowCaptured,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback),
                     target->GetTitle()));
}

void ChromeJemaAssistantAppUIDelegate::OnActiveWindowCaptured(
    ToolCallback callback,
    std::u16string title,
    scoped_refptr<base::RefCountedMemory> png) {
  if (!png || png->size() == 0 || png->size() > 4 * 1024 * 1024) {
    std::move(callback).Run(
        base::unexpected(std::string("Window capture failed or too large")));
    return;
  }
  base::Value::Dict result;
  result.Set("title", base::UTF16ToUTF8(title));
  result.Set("mimeType", "image/png");
  result.Set("base64", base::Base64Encode(*png));
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::GetActiveAccessibilityTree(
    const base::Value::Dict& request,
    ToolCallback callback) {
  if (!request.FindBool("confirmed").value_or(false)) {
    std::move(callback).Run(
        base::unexpected(std::string("User confirmation required")));
    return;
  }
  content::WebContents* contents =
      GetWebContentsForWindow(GetTargetUserWindow());
  if (!contents) {
    GetNativeAccessibilityTree(GetTargetUserWindow(), std::move(callback));
    return;
  }
  if (contents->GetBrowserContext()->IsOffTheRecord()) {
    std::move(callback).Run(base::unexpected(
        std::string("Semantic tree unavailable for this window")));
    return;
  }
  action_tokens_.clear();
  auto snapshot = std::make_shared<AccessibilitySnapshotRequest>();
  snapshot->callback = std::move(callback);
  snapshot->web_contents = contents->GetWeakPtr();
  snapshot->url = contents->GetLastCommittedURL();
  snapshot->title = contents->GetTitle();
  content::RenderFrameHost* root = contents->GetPrimaryMainFrame();
  root->ForEachRenderFrameHostWithAction([&](content::RenderFrameHost* frame) {
    if (!frame->IsRenderFrameLive() || !frame->IsActive() ||
        !root->GetLastCommittedOrigin().IsSameOriginWith(
            frame->GetLastCommittedOrigin())) {
      return content::RenderFrameHost::FrameIterationAction::kSkipChildren;
    }
    ++snapshot->pending;
    RequestFrameAccessibilitySnapshot(snapshot, frame);
    return content::RenderFrameHost::FrameIterationAction::kContinue;
  });
  if (snapshot->pending == 0) {
    CompleteAccessibilitySnapshot(snapshot);
    return;
  }
  base::SequencedTaskRunner::GetCurrentDefault()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(
          &ChromeJemaAssistantAppUIDelegate::CompleteAccessibilitySnapshot,
          weak_ptr_factory_.GetWeakPtr(), snapshot),
      base::Milliseconds(2500));
}

void ChromeJemaAssistantAppUIDelegate::GetNativeAccessibilityTree(
    aura::Window* window,
    ToolCallback callback) {
  ui::AXTreeUpdate update;
  AutomationManagerAura* manager = AutomationManagerAura::GetInstance();
  if (!window ||
      !manager->SnapshotWindow(window, 500, base::Milliseconds(100), &update)) {
    std::move(callback).Run(base::unexpected(
        std::string("Native semantic tree unavailable for this window")));
    return;
  }
  action_tokens_.clear();
  base::Value::List nodes;
  for (const ui::AXNodeData& node : update.nodes) {
    const bool focus = node.HasAction(ax::mojom::Action::kFocus);
    const bool activate = node.HasAction(ax::mojom::Action::kDoDefault);
    const bool set_value = node.HasAction(ax::mojom::Action::kSetValue);
    const std::string name =
        node.GetStringAttribute(ax::mojom::StringAttribute::kName);
    if (name.empty() && !focus && !activate && !set_value) {
      continue;
    }
    const std::string token =
        base::Uuid::GenerateRandomV4().AsLowercaseString();
    ActionToken action_token;
    action_token.node_id = node.id;
    action_token.tree_id = manager->ax_tree_id();
    action_token.expires_at = base::TimeTicks::Now() + base::Seconds(60);
    action_token.can_focus = focus;
    action_token.can_activate = activate;
    action_token.can_set_value = set_value;
    action_token.native_views = true;
    action_token.native_window = window;
    action_tokens_.emplace(token, std::move(action_token));
    base::Value::Dict serialized;
    serialized.Set("token", token);
    serialized.Set("role", ui::ToString(node.role));
    serialized.Set("name", name);
    serialized.Set("focusable", focus);
    serialized.Set("activatable", activate);
    serialized.Set("editable", set_value);
    nodes.Append(std::move(serialized));
  }
  base::Value::Dict result;
  result.Set("title", base::UTF16ToUTF8(window->GetTitle()));
  result.Set("native", true);
  result.Set("nodes", std::move(nodes));
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::RequestFrameAccessibilitySnapshot(
    std::shared_ptr<AccessibilitySnapshotRequest> request,
    content::RenderFrameHost* frame) {
  frame->RequestAXTreeSnapshot(
      base::BindOnce(
          &ChromeJemaAssistantAppUIDelegate::OnFrameAccessibilityTreeSnapshot,
          weak_ptr_factory_.GetWeakPtr(), request, frame->GetWeakDocumentPtr(),
          frame->GetAXTreeID()),
      ui::kAXModeWebContentsOnly, 1000, base::Seconds(2));
}

void ChromeJemaAssistantAppUIDelegate::OnFrameAccessibilityTreeSnapshot(
    std::shared_ptr<AccessibilitySnapshotRequest> request,
    content::WeakDocumentPtr document,
    ui::AXTreeID tree_id,
    ui::AXTreeUpdate& update) {
  if (request->completed) {
    return;
  }
  if (document.AsRenderFrameHostIfValid() &&
      document.AsRenderFrameHostIfValid()->GetAXTreeID() == tree_id) {
    for (const ui::AXNodeData& node : update.nodes) {
      const bool focus = node.HasAction(ax::mojom::Action::kFocus);
      const bool activate = node.HasAction(ax::mojom::Action::kDoDefault);
      const bool set_value = node.HasAction(ax::mojom::Action::kSetValue);
      const std::string name =
          node.GetStringAttribute(ax::mojom::StringAttribute::kName);
      if (name.empty() && !focus && !activate && !set_value) {
        continue;
      }
      const std::string token =
          base::Uuid::GenerateRandomV4().AsLowercaseString();
      action_tokens_.emplace(
          token, ActionToken{node.id, tree_id, document,
                             base::TimeTicks::Now() + base::Seconds(60), focus,
                             activate, set_value});
      base::Value::Dict serialized;
      serialized.Set("token", token);
      serialized.Set("role", ui::ToString(node.role));
      serialized.Set("name", name);
      serialized.Set("focusable", focus);
      serialized.Set("activatable", activate);
      serialized.Set("editable", set_value);
      if (set_value) {
        serialized.Set("value", node.GetStringAttribute(
                                    ax::mojom::StringAttribute::kValue));
      }
      request->nodes.Append(std::move(serialized));
      if (request->nodes.size() >= 500u) {
        break;
      }
    }
  }
}
--request->pending;
if (request->pending == 0) {
  CompleteAccessibilitySnapshot(request);
}
}

void ChromeJemaAssistantAppUIDelegate::CompleteAccessibilitySnapshot(
    std::shared_ptr<AccessibilitySnapshotRequest> request) {
  if (request->completed) {
    return;
  }
  request->completed = true;
  if (!request->web_contents ||
      request->web_contents->GetLastCommittedURL() != request->url) {
    std::move(request->callback)
        .Run(base::unexpected(
            std::string("Document changed during inspection")));
    return;
  }
  base::Value::Dict result;
  result.Set("url", request->url.spec());
  result.Set("title", base::UTF16ToUTF8(request->title));
  result.Set("nodes", std::move(request->nodes));
  std::move(request->callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::PerformAccessibilityAction(
    const base::Value::Dict& request,
    ToolCallback callback) {
  if (!request.FindBool("confirmed").value_or(false)) {
    std::move(callback).Run(
        base::unexpected(std::string("User confirmation required")));
    return;
  }
  const std::string* token = request.FindString("token");
  const std::string* action_name = request.FindString("action");
  auto it = token ? action_tokens_.find(*token) : action_tokens_.end();
  if (!action_name || it == action_tokens_.end() ||
      it->second.expires_at < base::TimeTicks::Now()) {
    std::move(callback).Run(
        base::unexpected(std::string("Expired or invalid action token")));
    return;
  }
  ui::AXActionData action;
  action.target_node_id = it->second.node_id;
  action.target_tree_id = it->second.tree_id;
  if (*action_name == "focus" && it->second.can_focus) {
    action.action = ax::mojom::Action::kFocus;
  } else if (*action_name == "activate" && it->second.can_activate) {
    action.action = ax::mojom::Action::kDoDefault;
  } else if (*action_name == "set_value" && it->second.can_set_value) {
    const std::string* value = request.FindString("value");
    if (!value || value->size() > 64 * 1024u) {
      std::move(callback).Run(
          base::unexpected(std::string("Invalid accessibility value")));
      return;
    }
    action.action = ax::mojom::Action::kSetValue;
    action.value = *value;
  } else {
    std::move(callback).Run(
        base::unexpected(std::string("Action not supported by target node")));
    return;
  }
  if (it->second.native_views) {
    if (!it->second.native_window ||
        !AutomationManagerAura::GetInstance()->PerformCheckedAction(action)) {
      std::move(callback).Run(
          base::unexpected(std::string("Native action dispatch failed")));
      return;
    }
  } else {
    content::RenderFrameHost* frame =
        it->second.document.AsRenderFrameHostIfValid();
    if (!frame || frame->GetAXTreeID() != it->second.tree_id) {
      std::move(callback).Run(
          base::unexpected(std::string("Expired document action token")));
      return;
    }
    frame->AccessibilityPerformAction(action);
  }
  const bool native_action = it->second.native_views;
  action_tokens_.erase(it);
  base::Value::Dict result;
  result.Set("dispatched", true);
  result.Set("native", native_action);
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::GetActiveWindowContext(
    ToolCallback callback) {
  aura::Window* window = ash::window_util::GetActiveWindow();
  if (!window) {
    std::move(callback).Run(base::unexpected(std::string("No active window")));
    return;
  }
  base::Value::Dict result;
  result.Set("title", base::UTF16ToUTF8(window->GetTitle()));
  if (const std::string* app_id = window->GetProperty(ash::kAppIDKey)) {
    result.Set("appId", *app_id);
  }
  result.Set("appType",
             static_cast<int>(window->GetProperty(chromeos::kAppTypeKey)));
  if (ash::DesksController::Get() &&
      ash::DesksController::Get()->active_desk()) {
    result.Set("desk", base::UTF16ToUTF8(
                           ash::DesksController::Get()->active_desk()->name()));
  }
  result.Set("maximized", ash::WindowState::Get(window)->IsMaximized());
  result.Set("fullscreen", ash::WindowState::Get(window)->IsFullscreen());
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::OpenBrowserUrl(
    const base::Value::Dict& request,
    ToolCallback callback) {
  if (!request.FindBool("confirmed").value_or(false)) {
    std::move(callback).Run(
        base::unexpected(std::string("User confirmation required")));
    return;
  }
  const std::string* raw_url = request.FindString("url");
  if (!raw_url || raw_url->empty() || raw_url->size() > 2048u) {
    std::move(callback).Run(base::unexpected(std::string("Invalid URL")));
    return;
  }
  std::string normalized = *raw_url;
  if (!base::StartsWith(normalized, "http://",
                        base::CompareCase::INSENSITIVE_ASCII) &&
      !base::StartsWith(normalized, "https://",
                        base::CompareCase::INSENSITIVE_ASCII)) {
    normalized = "https://" + normalized;
  }
  const GURL url(normalized);
  if (!url.is_valid() ||
      (!url.SchemeIs(url::kHttpScheme) && !url.SchemeIs(url::kHttpsScheme)) ||
      url.has_username() || url.has_password()) {
    std::move(callback).Run(
        base::unexpected(std::string("Only public HTTP(S) URLs are allowed")));
    return;
  }
  ash::NewWindowDelegate::GetPrimary()->OpenUrl(
      url, ash::NewWindowDelegate::OpenUrlFrom::kUserInteraction,
      ash::NewWindowDelegate::Disposition::kNewForegroundTab);
  base::Value::Dict result;
  result.Set("opened", true);
  result.Set("url", url.spec());
  std::move(callback).Run(std::move(result));
}

std::string ChromeJemaAssistantAppUIDelegate::GetPermission(
    const base::Value::Dict& request) const {
  const std::string* agent_id = request.FindString("agentId");
  const std::string* tool = request.FindString("tool");
  if (!agent_id || !tool || agent_id->empty() || tool->empty()) {
    return "ask";
  }
  Profile* profile = Profile::FromWebUI(web_ui_);
  const base::Value::Dict& permissions = profile->GetPrefs()->GetDict(
      jemaos::prefs::kJemaAssistantAgentPermissions);
  const base::Value::Dict* agent = permissions.FindDict(*agent_id);
  const std::string* mode = agent ? agent->FindString("__mode__") : nullptr;
  if (mode && *mode == "deny") {
    return "deny";
  }
  const std::string* value = agent ? agent->FindString(*tool) : nullptr;
  if (mode && *mode == "allow" && !value) {
    return "deny";
  }
  return value && (*value == "allow" || *value == "deny" || *value == "ask")
             ? *value
             : "ask";
}

void ChromeJemaAssistantAppUIDelegate::GetWifiState(ToolCallback callback) {
  if (!ash::NetworkHandler::IsInitialized()) {
    std::move(callback).Run(
        base::unexpected(std::string("Network service unavailable")));
    return;
  }
  auto* state = ash::NetworkHandler::Get()->network_state_handler();
  base::Value::Dict result;
  result.Set("available",
             state->IsTechnologyAvailable(ash::NetworkTypePattern::WiFi()));
  result.Set("enabled",
             state->IsTechnologyEnabled(ash::NetworkTypePattern::WiFi()));
  result.Set("prohibited",
             state->IsTechnologyProhibited(ash::NetworkTypePattern::WiFi()));
  ash::NetworkStateHandler::NetworkStateList networks;
  state->GetVisibleNetworkListByType(ash::NetworkTypePattern::WiFi(),
                                     &networks);
  base::Value::List visible;
  for (const ash::NetworkState* network : networks) {
    base::Value::Dict value;
    value.Set("guid", network->guid());
    value.Set("name", network->name());
    value.Set("signal", network->signal_strength());
    value.Set("connected", network->IsConnectedState());
    value.Set("connectable", network->connectable());
    visible.Append(std::move(value));
  }
  result.Set("networks", std::move(visible));
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::SetWifiState(
    const base::Value::Dict& request,
    ToolCallback callback) {
  if (!request.FindBool("confirmed").value_or(false)) {
    std::move(callback).Run(
        base::unexpected(std::string("User confirmation required")));
    return;
  }
  const std::optional<bool> enabled = request.FindBool("enabled");
  if (!enabled || !ash::NetworkHandler::IsInitialized()) {
    std::move(callback).Run(
        base::unexpected(std::string("Invalid Wi-Fi request")));
    return;
  }
  auto* state = ash::NetworkHandler::Get()->network_state_handler();
  if (state->IsTechnologyProhibited(ash::NetworkTypePattern::WiFi())) {
    std::move(callback).Run(
        base::unexpected(std::string("Wi-Fi is policy controlled")));
    return;
  }
  ash::NetworkHandler::Get()
      ->technology_state_controller()
      ->SetTechnologiesEnabled(ash::NetworkTypePattern::WiFi(), *enabled,
                               base::BindOnce([](const std::string&) {}));
  base::Value::Dict result;
  result.Set("requested", true);
  result.Set("enabled", *enabled);
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::GetBluetoothState(
    ToolCallback callback) {
  if (!device::BluetoothAdapterFactory::IsBluetoothSupported()) {
    std::move(callback).Run(
        base::unexpected(std::string("Bluetooth unsupported")));
    return;
  }
  device::BluetoothAdapterFactory::Get()->GetAdapter(base::BindOnce(
      [](ToolCallback callback,
         scoped_refptr<device::BluetoothAdapter> adapter) {
        base::Value::Dict result;
        result.Set("present", adapter->IsPresent());
        result.Set("powered", adapter->IsPowered());
        base::Value::List devices;
        for (device::BluetoothDevice* device : adapter->GetDevices()) {
          base::Value::Dict value;
          value.Set("name", base::UTF16ToUTF8(device->GetNameForDisplay()));
          value.Set("paired", device->IsPaired());
          value.Set("connected", device->IsConnected());
          devices.Append(std::move(value));
        }
        result.Set("devices", std::move(devices));
        std::move(callback).Run(std::move(result));
      },
      std::move(callback)));
}

void ChromeJemaAssistantAppUIDelegate::SetBluetoothState(
    const base::Value::Dict& request,
    ToolCallback callback) {
  if (!request.FindBool("confirmed").value_or(false)) {
    std::move(callback).Run(
        base::unexpected(std::string("User confirmation required")));
    return;
  }
  const std::optional<bool> enabled = request.FindBool("enabled");
  if (!enabled || !device::BluetoothAdapterFactory::IsBluetoothSupported()) {
    std::move(callback).Run(
        base::unexpected(std::string("Invalid Bluetooth request")));
    return;
  }
  device::BluetoothAdapterFactory::Get()->GetAdapter(base::BindOnce(
      [](bool enabled, ToolCallback callback,
         scoped_refptr<device::BluetoothAdapter> adapter) {
        auto split_callback = base::SplitOnceCallback(std::move(callback));
        adapter->SetPowered(
            enabled,
            base::BindOnce(
                [](bool enabled, ToolCallback callback) {
                  base::Value::Dict result;
                  result.Set("enabled", enabled);
                  std::move(callback).Run(std::move(result));
                },
                enabled, std::move(split_callback.first)),
            base::BindOnce(
                [](ToolCallback callback) {
                  std::move(callback).Run(base::unexpected(
                      std::string("Bluetooth power change failed")));
                },
                std::move(split_callback.second)));
      },
      *enabled, std::move(callback)));
}

void ChromeJemaAssistantAppUIDelegate::AppendAuditEntry(
    const base::Value::Dict& request,
    bool success,
    std::string_view detail) {
  Profile* profile = Profile::FromWebUI(web_ui_);
  ScopedListPrefUpdate update(profile->GetPrefs(),
                              jemaos::prefs::kJemaAssistantAuditLog);
  base::Value::Dict entry;
  entry.Set("timestamp",
            base::Time::Now().ToDeltaSinceWindowsEpoch().InMillisecondsF());
  entry.Set("agentId", request.FindString("agentId")
                           ? *request.FindString("agentId")
                           : "unknown");
  entry.Set("tool", request.FindString("tool") ? *request.FindString("tool")
                                               : "unknown");
  entry.Set("success", success);
  entry.Set("detail", std::string(detail.substr(0, 256)));
  update->Append(std::move(entry));
  while (update->size() > 200u) {
    update->erase(update->begin());
  }
}

void ChromeJemaAssistantAppUIDelegate::SaveApiKey(const std::string& provider,
                                                  const std::string& api_key,
                                                  ToolCallback callback) {
  if (provider.empty() || provider.size() > 32u || api_key.empty() ||
      api_key.size() > 4096u) {
    std::move(callback).Run(
        base::unexpected(std::string("Secure secret storage unavailable")));
    return;
  }
  std::string encrypted;
  if (!OSCrypt::EncryptString(api_key, &encrypted)) {
    std::move(callback).Run(
        base::unexpected(std::string("API key encryption failed")));
    return;
  }
  Profile* profile = Profile::FromWebUI(web_ui_);
  ScopedDictPrefUpdate update(profile->GetPrefs(),
                              jemaos::prefs::kJemaAssistantEncryptedApiKeys);
  update->Set(provider, base::Base64Encode(encrypted));
  base::Value::Dict result;
  result.Set("saved", true);
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::LoadApiKey(const std::string& provider,
                                                  ToolCallback callback) {
  Profile* profile = Profile::FromWebUI(web_ui_);
  const std::string* encoded =
      profile->GetPrefs()
          ->GetDict(jemaos::prefs::kJemaAssistantEncryptedApiKeys)
          .FindString(provider);
  std::string encrypted;
  std::string api_key;
  if (!encoded || !base::Base64Decode(*encoded, &encrypted) ||
      !OSCrypt::DecryptString(encrypted, &api_key)) {
    std::move(callback).Run(
        base::unexpected(std::string("API key not configured")));
    return;
  }
  base::Value::Dict result;
  result.Set("apiKey", std::move(api_key));
  std::move(callback).Run(std::move(result));
}

std::optional<base::FilePath> ChromeJemaAssistantAppUIDelegate::ResolveUserPath(
    const base::Value::Dict& request,
    std::string* error) const {
  const std::string* root_name = request.FindString("root");
  const std::string* relative_path = request.FindString("path");
  if (!root_name || !relative_path || relative_path->size() > 512u) {
    *error = "Invalid user path";
    return std::nullopt;
  }
  base::FilePath relative = base::FilePath::FromUTF8Unsafe(*relative_path);
  if (relative.IsAbsolute() || relative.ReferencesParent()) {
    *error = "Path must remain inside the selected root";
    return std::nullopt;
  }
  Profile* profile = Profile::FromWebUI(web_ui_);
  base::FilePath root;
  if (*root_name == "my_files") {
    root = file_manager::util::GetMyFilesFolderForProfile(profile);
  } else if (*root_name == "downloads") {
    root = file_manager::util::GetDownloadsFolderForProfile(profile);
  } else {
    *error = "Unsupported file root";
    return std::nullopt;
  }
  return root.Append(relative);
}

void ChromeJemaAssistantAppUIDelegate::SearchFiles(
    const base::Value::Dict& request,
    ToolCallback callback) {
  const std::string* query = request.FindString("query");
  if (!query || query->empty() || query->size() > 128u) {
    std::move(callback).Run(base::unexpected(std::string("Invalid query")));
    return;
  }
  base::Value::Dict path_request;
  path_request.Set("root", request.FindString("root")
                               ? *request.FindString("root")
                               : "my_files");
  path_request.Set("path", "");
  std::string error;
  const auto root = ResolveUserPath(path_request, &error);
  if (!root) {
    std::move(callback).Run(base::unexpected(error));
    return;
  }
  base::Value::List matches;
  base::FileEnumerator files(*root, /*recursive=*/true,
                             base::FileEnumerator::FILES);
  for (base::FilePath path = files.Next();
       !path.empty() && matches.size() < 100; path = files.Next()) {
    if (base::ToLowerASCII(path.BaseName().AsUTF8Unsafe())
            .find(base::ToLowerASCII(*query)) != std::string::npos) {
      base::FilePath relative;
      if (root->AppendRelativePath(path, &relative)) {
        matches.Append(relative.AsUTF8Unsafe());
      }
    }
  }
  base::Value::Dict result;
  result.Set("matches", std::move(matches));
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::ReadFile(
    const base::Value::Dict& request,
    ToolCallback callback) {
  if (!request.FindBool("confirmed").value_or(false)) {
    std::move(callback).Run(
        base::unexpected(std::string("User confirmation required")));
    return;
  }
  std::string error;
  const auto path = ResolveUserPath(request, &error);
  std::string content;
  base::FilePath normalized;
  base::FilePath normalized_root;
  Profile* profile = Profile::FromWebUI(web_ui_);
  const std::string* root_name = request.FindString("root");
  const base::FilePath root =
      root_name && *root_name == "downloads"
          ? file_manager::util::GetDownloadsFolderForProfile(profile)
          : file_manager::util::GetMyFilesFolderForProfile(profile);
  if (!path || !base::NormalizeFilePath(*path, &normalized) ||
      !base::NormalizeFilePath(root, &normalized_root) ||
      (normalized != normalized_root &&
       !normalized_root.IsParent(normalized)) ||
      base::IsLink(*path) ||
      !base::ReadFileToStringWithMaxSize(normalized, &content, 1024 * 1024)) {
    std::move(callback).Run(base::unexpected(
        path ? std::string("Cannot read file") : std::move(error)));
    return;
  }
  base::Value::Dict result;
  result.Set("content", std::move(content));
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::WriteFile(
    const base::Value::Dict& request,
    ToolCallback callback) {
  if (!request.FindBool("confirmed").value_or(false)) {
    std::move(callback).Run(
        base::unexpected(std::string("User confirmation required")));
    return;
  }
  const std::string* content = request.FindString("content");
  std::string error;
  const auto path = ResolveUserPath(request, &error);
  if (!path || !content || content->size() > 4 * 1024 * 1024 ||
      base::IsLink(*path) || !base::CreateDirectory(path->DirName()) ||
      !base::WriteFile(*path, *content)) {
    std::move(callback).Run(base::unexpected(
        path ? std::string("Cannot write file") : std::move(error)));
    return;
  }
  base::Value::Dict result;
  result.Set("written", true);
  result.Set("path", request.FindString("path") ? *request.FindString("path")
                                                : std::string());
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::GetAudioState(ToolCallback callback) {
  auto* audio = ash::CrasAudioHandler::Get();
  base::Value::Dict result;
  result.Set("volume", audio->GetOutputVolumePercent());
  result.Set("muted", audio->IsOutputMuted());
  result.Set("mutedByPolicy", audio->IsOutputMutedByPolicy());
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::SetAudioState(
    const base::Value::Dict& request,
    ToolCallback callback) {
  if (!request.FindBool("confirmed").value_or(false)) {
    std::move(callback).Run(
        base::unexpected(std::string("User confirmation required")));
    return;
  }
  auto* audio = ash::CrasAudioHandler::Get();
  if (const auto volume = request.FindInt("volume")) {
    audio->SetOutputVolumePercent(std::clamp(*volume, 0, 100));
  }
  if (const auto muted = request.FindBool("muted")) {
    if (!*muted && audio->IsOutputMutedByPolicy()) {
      std::move(callback).Run(
          base::unexpected(std::string("Audio mute is policy controlled")));
      return;
    }
    audio->SetOutputMute(*muted);
  }
  GetAudioState(std::move(callback));
}

void ChromeJemaAssistantAppUIDelegate::GetPowerState(ToolCallback callback) {
  if (!ash::PowerStatus::IsInitialized()) {
    std::move(callback).Run(
        base::unexpected(std::string("Power status unavailable")));
    return;
  }
  ash::PowerStatus* power = ash::PowerStatus::Get();
  base::Value::Dict result;
  result.Set("batteryPresent", power->IsBatteryPresent());
  result.Set("batteryPercent", power->GetBatteryPercent());
  result.Set("charging", power->IsBatteryCharging());
  result.Set("linePower", power->IsLinePowerConnected());
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::ShowNotification(
    const base::Value::Dict& request,
    ToolCallback callback) {
  const std::string* title = request.FindString("title");
  const std::string* message = request.FindString("message");
  if (!title || !message || title->size() > 128u || message->size() > 1024u) {
    std::move(callback).Run(
        base::unexpected(std::string("Invalid notification")));
    return;
  }
  auto notification = ash::SystemNotificationBuilder()
                          .SetId("jemaos-ai-agent")
                          .SetTitle(base::UTF8ToUTF16(*title))
                          .SetMessage(base::UTF8ToUTF16(*message))
                          .SetDisplaySource(u"JemaOS AI")
                          .SetNotifierId(message_center::NotifierId(
                              message_center::NotifierType::SYSTEM_COMPONENT,
                              "jemaos-ai", ash::NotificationCatalogName::kNone))
                          .BuildPtr(false);
  message_center::MessageCenter::Get()->AddNotification(
      std::move(notification));
  base::Value::Dict result;
  result.Set("shown", true);
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::ComposeEmail(
    const base::Value::Dict& request,
    ToolCallback callback) {
  if (!request.FindBool("confirmed").value_or(false)) {
    std::move(callback).Run(
        base::unexpected(std::string("User confirmation required")));
    return;
  }
  const std::string* to = request.FindString("to");
  const std::string* subject = request.FindString("subject");
  const std::string* body = request.FindString("body");
  if (!to || to->size() > 512u || (subject && subject->size() > 512u) ||
      (body && body->size() > 64 * 1024u)) {
    std::move(callback).Run(
        base::unexpected(std::string("Invalid email draft")));
    return;
  }
  std::string spec = "mailto:" + base::EscapeQueryParamValue(*to, true);
  spec += "?subject=" +
          base::EscapeQueryParamValue(subject ? *subject : std::string(), true);
  spec += "&body=" +
          base::EscapeQueryParamValue(body ? *body : std::string(), true);
  ash::NewWindowDelegate::GetPrimary()->OpenUrl(
      GURL(spec), ash::NewWindowDelegate::OpenUrlFrom::kUserInteraction,
      ash::NewWindowDelegate::Disposition::kNewForegroundTab);
  base::Value::Dict result;
  result.Set("composeOpened", true);
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::ListDisplayModes(ToolCallback callback) {
  ash::Shell::Get()->cros_display_config()->GetDisplayUnitInfoList(
      /*single_unified=*/false,
      base::BindOnce(
          [](ToolCallback callback,
             std::vector<crosapi::mojom::DisplayUnitInfoPtr> displays) {
            base::Value::List output;
            for (const auto& display : displays) {
              base::Value::Dict item;
              item.Set("displayId", display->id);
              item.Set("name", display->name);
              item.Set("internal", display->is_internal);
              item.Set("primary", display->is_primary);
              item.Set("selectedModeIndex",
                       display->selected_display_mode_index);
              base::Value::List modes;
              for (size_t index = 0;
                   index < display->available_display_modes.size(); ++index) {
                const auto& mode = display->available_display_modes[index];
                base::Value::Dict value;
                value.Set("modeIndex", static_cast<int>(index));
                value.Set("width", mode->size.width());
                value.Set("height", mode->size.height());
                value.Set("nativeWidth", mode->size_in_native_pixels.width());
                value.Set("nativeHeight", mode->size_in_native_pixels.height());
                value.Set("refreshRate", mode->refresh_rate);
                value.Set("native", mode->is_native);
                modes.Append(std::move(value));
              }
              item.Set("modes", std::move(modes));
              output.Append(std::move(item));
            }
            base::Value::Dict result;
            result.Set("displays", std::move(output));
            std::move(callback).Run(std::move(result));
          },
          std::move(callback)));
}

void ChromeJemaAssistantAppUIDelegate::SetDisplayMode(
    const base::Value::Dict& request,
    ToolCallback callback) {
  if (!request.FindBool("confirmed").value_or(false)) {
    std::move(callback).Run(
        base::unexpected(std::string("User confirmation required")));
    return;
  }
  const std::string* display_id = request.FindString("displayId");
  const std::optional<int> mode_index = request.FindInt("modeIndex");
  if (!display_id || !mode_index || display_id->empty() ||
      display_id->size() > 64u || *mode_index < 0) {
    std::move(callback).Run(
        base::unexpected(std::string("Invalid display mode request")));
    return;
  }

  ash::Shell::Get()->cros_display_config()->GetDisplayUnitInfoList(
      /*single_unified=*/false,
      base::BindOnce(
          [](std::string display_id, int mode_index, ToolCallback callback,
             std::vector<crosapi::mojom::DisplayUnitInfoPtr> displays) {
            for (const auto& display : displays) {
              if (display->id != display_id) {
                continue;
              }
              if (mode_index >=
                  static_cast<int>(display->available_display_modes.size())) {
                std::move(callback).Run(base::unexpected(
                    std::string("Display mode index out of range")));
                return;
              }
              const bool confirmation_pending = !display->is_internal;
              auto properties = crosapi::mojom::DisplayConfigProperties::New();
              properties->display_mode =
                  display->available_display_modes[mode_index].Clone();
              ash::Shell::Get()->cros_display_config()->SetDisplayProperties(
                  display_id, std::move(properties),
                  crosapi::mojom::DisplayConfigSource::kUser,
                  base::BindOnce(
                      [](bool confirmation_pending, ToolCallback callback,
                         crosapi::mojom::DisplayConfigResult result) {
                        base::Value::Dict output;
                        output.Set(
                            "success",
                            result ==
                                crosapi::mojom::DisplayConfigResult::kSuccess);
                        output.Set("result", static_cast<int>(result));
                        output.Set("confirmationPending", confirmation_pending);
                        std::move(callback).Run(std::move(output));
                      },
                      confirmation_pending, std::move(callback)));
              return;
            }
            std::move(callback).Run(
                base::unexpected(std::string("Display not found")));
          },
          *display_id, *mode_index, std::move(callback)));
}

void ChromeJemaAssistantAppUIDelegate::GetLocale(ToolCallback callback) {
  Profile* profile = Profile::FromWebUI(web_ui_);
  base::Value::Dict result;
  result.Set("locale", profile->GetPrefs()->GetString(
                           language::prefs::kApplicationLocale));
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::ChangeLocale(
    const base::Value::Dict& request,
    ToolCallback callback) {
  if (!request.FindBool("confirmed").value_or(false)) {
    std::move(callback).Run(
        base::unexpected(std::string("User confirmation required")));
    return;
  }
  const std::string* locale = request.FindString("locale");
  Profile* profile = Profile::FromWebUI(web_ui_);
  if (!locale || locale->empty() || locale->size() > 32u ||
      !ash::locale_util::IsNativeUILanguage(*locale) ||
      !ash::locale_util::IsAllowedUILanguage(*locale, profile->GetPrefs())) {
    std::move(callback).Run(
        base::unexpected(std::string("Locale unavailable or blocked")));
    return;
  }
  profile->ChangeAppLocale(*locale, Profile::APP_LOCALE_CHANGED_VIA_SETTINGS);
  ash::locale_util::AddLocaleToPreferredLanguages(*locale, profile->GetPrefs());
  base::Value::Dict result;
  result.Set("changed", true);
  result.Set("locale", *locale);
  result.Set("requiresSignOut", true);
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::ListApps(ToolCallback callback) {
  Profile* profile = Profile::FromWebUI(web_ui_);
  if (!apps::AppServiceProxyFactory::IsAppServiceAvailableForProfile(profile)) {
    std::move(callback).Run(
        base::unexpected(std::string("App Service unavailable")));
    return;
  }

  base::Value::List app_list;
  apps::AppServiceProxyFactory::GetForProfile(profile)
      ->AppRegistryCache()
      .ForEachApp([&app_list](const apps::AppUpdate& update) {
        if (!apps_util::IsInstalled(update.Readiness()) ||
            update.Readiness() != apps::Readiness::kReady ||
            update.Name().empty()) {
          return;
        }
        base::Value::Dict app;
        app.Set("appId", update.AppId());
        app.Set("name", update.Name());
        app.Set("shortName", update.ShortName());
        app_list.Append(std::move(app));
      });
  base::Value::Dict result;
  result.Set("apps", std::move(app_list));
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::LaunchApp(
    const base::Value::Dict& request,
    ToolCallback callback) {
  if (!request.FindBool("confirmed").value_or(false)) {
    std::move(callback).Run(
        base::unexpected(std::string("User confirmation required")));
    return;
  }
  const std::string* query = request.FindString("app");
  if (!query || query->empty() || query->size() > 128u) {
    std::move(callback).Run(base::unexpected(std::string("Invalid app name")));
    return;
  }
  std::string error;
  const std::optional<std::string> app_id = ResolveAppId(*query, &error);
  if (!app_id) {
    std::move(callback).Run(base::unexpected(error));
    return;
  }
  Profile* profile = Profile::FromWebUI(web_ui_);
  apps::AppServiceProxyFactory::GetForProfile(profile)->Launch(
      *app_id, /*event_flags=*/0, apps::LaunchSource::kFromOtherApp);
  base::Value::Dict result;
  result.Set("launched", true);
  result.Set("appId", *app_id);
  std::move(callback).Run(std::move(result));
}

void ChromeJemaAssistantAppUIDelegate::OpenQuickText(
    const base::Value::Dict& request,
    ToolCallback callback) {
  if (!request.FindBool("confirmed").value_or(false)) {
    std::move(callback).Run(
        base::unexpected(std::string("User confirmation required")));
    return;
  }
  const std::string* content = request.FindString("content");
  const std::string* title = request.FindString("title");
  if (!content || content->empty() || content->size() > 1024 * 1024) {
    std::move(callback).Run(
        base::unexpected(std::string("Invalid QuickText content")));
    return;
  }
  std::string error;
  const std::optional<std::string> app_id = ResolveAppId("QuickText", &error);
  if (!app_id) {
    std::move(callback).Run(base::unexpected(error));
    return;
  }

  Profile* profile = Profile::FromWebUI(web_ui_);
  apps::AppServiceProxyFactory::GetForProfile(profile)->LaunchAppWithIntent(
      *app_id, /*event_flags=*/0,
      apps_util::MakeShareIntent(*content, title ? *title : "JemaOS AI"),
      apps::LaunchSource::kFromOtherApp, /*window_info=*/nullptr,
      base::DoNothing());
  base::Value::Dict response;
  response.Set("launched", true);
  response.Set("appId", *app_id);
  std::move(callback).Run(std::move(response));
}

std::optional<std::string> ChromeJemaAssistantAppUIDelegate::ResolveAppId(
    const std::string& query,
    std::string* error) const {
  Profile* profile = Profile::FromWebUI(web_ui_);
  if (!apps::AppServiceProxyFactory::IsAppServiceAvailableForProfile(profile)) {
    *error = "App Service unavailable";
    return std::nullopt;
  }

  std::vector<std::string> matches;
  apps::AppServiceProxyFactory::GetForProfile(profile)
      ->AppRegistryCache()
      .ForEachApp([&](const apps::AppUpdate& update) {
        if (update.Readiness() != apps::Readiness::kReady) {
          return;
        }
        if (base::EqualsCaseInsensitiveASCII(update.AppId(), query) ||
            base::EqualsCaseInsensitiveASCII(update.Name(), query) ||
            base::EqualsCaseInsensitiveASCII(update.ShortName(), query)) {
          matches.push_back(update.AppId());
        }
      });
  if (matches.empty()) {
    *error = "Application not found: " + query;
    return std::nullopt;
  }
  if (matches.size() > 1u) {
    *error = "Ambiguous application name: " + query;
    return std::nullopt;
  }
  return matches.front();
}
