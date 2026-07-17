#include "ash/webui/jema_assistant_app_ui/jema_assistant_app_webui_handler.h"

#include <algorithm>
#include <utility>

#include "ash/app_list/app_list_controller_impl.h"
#include "ash/assistant/model/assistant_ui_model.h"
#include "ash/assistant/util/deep_link_util.h"
#include "ash/jemaos_ai/jemaos_ai_bar.h"
#include "ash/public/cpp/assistant/controller/assistant_ui_controller.h"
#include "ash/public/cpp/new_window_delegate.h"
#include "ash/root_window_controller.h"
#include "ash/shelf/shelf.h"
#include "ash/shell.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/ash/system_web_apps/color_helpers.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/network_service_instance.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/color_parser.h"
#include "net/base/net_errors.h"
#include "net/http/http_request_headers.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "ui/color/color_provider_utils.h"
#include "url/gurl.h"
#include "url/url_constants.h"

namespace ash {

namespace {

constexpr size_t kMaxLlmResponseSize = 4 * 1024 * 1024;

constexpr net::NetworkTrafficAnnotationTag kJemaLlmTrafficAnnotation =
    net::DefineNetworkTrafficAnnotation("jemaos_ai_llm_request", R"(
      semantics {
        sender: "JemaOS AI Environment"
        description:
          "Sends a user-approved assistant request to the configured LLM "
          "provider."
        trigger: "The user submits a prompt or an agent executes a task."
        data: "Prompt text, agent context, tool schemas, and API credential."
        destination: OTHER
      }
      policy {
        cookies_allowed: NO
        setting: "The user configures and can disable JemaOS AI."
        policy_exception_justification: "Not implemented."
      })");

bool IsAllowedLlmEndpoint(const GURL& url) {
  if (!url.SchemeIs(url::kHttpsScheme) || url.has_username() ||
      url.has_password() || url.has_ref()) {
    return false;
  }
  const std::string_view host = url.host_piece();
  return host == "api.mistral.ai" || host == "api.moonshot.ai" ||
         host == "open.bigmodel.cn" || host == "api.anthropic.com";
}

}  // namespace

JemaAssistantWebUIHandler::JemaAssistantWebUIHandler(JemaAssistantAppUI* app_ui)
    : jema_assistant_app_ui_(app_ui) {
  assistant_controller_observation_.Observe(AssistantController::Get());
  theme_observation_.Observe(ui::NativeTheme::GetInstanceForNativeUi());
  AssistantUiController::Get()->GetModel()->AddObserver(this);
  content::GetNetworkConnectionTracker()->AddNetworkConnectionObserver(this);
  Shelf* shelf = Shelf::ForWindow(Shell::GetPrimaryRootWindow());
  if (shelf && shelf->jema_assistant_view()) {
    shelf->jema_assistant_view()->AddObserver(this);
  }
}

JemaAssistantWebUIHandler::~JemaAssistantWebUIHandler() {
  if (AssistantUiController::Get()) {
    AssistantUiController::Get()->GetModel()->RemoveObserver(this);
  }
  content::GetNetworkConnectionTracker()->RemoveNetworkConnectionObserver(this);
  Shelf* shelf = Shelf::ForWindow(Shell::GetPrimaryRootWindow());
  if (shelf && shelf->jema_assistant_view()) {
    shelf->jema_assistant_view()->RemoveObserver(this);
  }
}

void JemaAssistantWebUIHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "onJemaAssistantSwaInit",
      base::BindRepeating(&JemaAssistantWebUIHandler::OnJemaAssistantSwaInit,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "onCloseAssistant",
      base::BindRepeating(&JemaAssistantWebUIHandler::OnRequestCloseAssistant,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "onOpenAssistantUrl",
      base::BindRepeating(&JemaAssistantWebUIHandler::OnJemaAssistantOpenUrl,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "setAssistantBubbleRect",
      base::BindRepeating(
          &JemaAssistantWebUIHandler::HandleSetAssistantBubbleRect,
          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "centerAssistantBubble",
      base::BindRepeating(
          &JemaAssistantWebUIHandler::HandleCenterAssistantBubbleRect,
          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "setJemaAgentState",
      base::BindRepeating(&JemaAssistantWebUIHandler::HandleSetJemaAgentState,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "llmRequest",
      base::BindRepeating(&JemaAssistantWebUIHandler::HandleLlmRequest,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "executeSystemTool",
      base::BindRepeating(&JemaAssistantWebUIHandler::HandleExecuteSystemTool,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "saveJemaApiKey",
      base::BindRepeating(&JemaAssistantWebUIHandler::HandleSaveApiKey,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "loadJemaApiKey",
      base::BindRepeating(&JemaAssistantWebUIHandler::HandleLoadApiKey,
                          base::Unretained(this)));
}

void JemaAssistantWebUIHandler::OnJemaAssistantSwaInit(
    const base::Value::List& args) {
  AllowJavascript();
}

void JemaAssistantWebUIHandler::OnRequestCloseAssistant(
    const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  const std::string source = args[0].GetString();
  if (source == "launcher") {
    auto* const app_list_controller = Shell::Get()->app_list_controller();
    if (app_list_controller) {
      app_list_controller->CloseJemaAssistant();
    }
    return;
  }

  if (source == "bubble") {
    Shelf* shelf = Shelf::ForWindow(Shell::GetPrimaryRootWindow());
    if (shelf && shelf->jema_assistant_view()) {
      shelf->jema_assistant_view()->HideBubble();
    }
  }
}

void JemaAssistantWebUIHandler::OnJemaAssistantOpenUrl(
    const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  const std::string url = args[0].GetString();
  VLOG(3) << "OnJemaAssistantOpenUrl, url: " << url;
  ash::NewWindowDelegate::GetPrimary()->OpenUrl(
      GURL(url), ash::NewWindowDelegate::OpenUrlFrom::kUserInteraction,
      ash::NewWindowDelegate::Disposition::kNewWindow);
}

void JemaAssistantWebUIHandler::HandleSetAssistantBubbleRect(
    const base::Value::List& args) {
  CHECK_EQ(4u, args.size());
  const int x = args[0].GetInt();
  const int y = args[1].GetInt();
  const int width = args[2].GetInt();
  const int height = args[3].GetInt();
  VLOG(3) << "HandleSetAssistantBubbleRect: " << x << ", " << y << ", " << width
          << ", " << height;
  Shelf* shelf = Shelf::ForWindow(Shell::GetPrimaryRootWindow());
  if (shelf && shelf->jema_assistant_view()) {
    shelf->jema_assistant_view()->SetBubbleRect(x, y, width, height);
  }
}

void JemaAssistantWebUIHandler::HandleCenterAssistantBubbleRect(
    const base::Value::List& args) {
  CHECK_EQ(2u, args.size());
  const int width = args[0].GetInt();
  const int height = args[1].GetInt();
  VLOG(3) << "HandleCenterAssistantBubbleRect: " << width << ", " << height;
  Shelf* shelf = Shelf::ForWindow(Shell::GetPrimaryRootWindow());
  if (shelf && shelf->jema_assistant_view()) {
    shelf->jema_assistant_view()->CenterBubble(width, height);
  }
}

void JemaAssistantWebUIHandler::OnDeepLinkReceived(
    assistant::util::DeepLinkType type,
    const std::map<std::string, std::string>& params) {
  if (!IsJavascriptAllowed()) {
    return;
  }
  if (type == assistant::util::DeepLinkType::kQuery) {
    const std::optional<std::string>& query =
        GetDeepLinkParam(params, assistant::util::DeepLinkParam::kQuery);
    if (query.has_value() && IsJavascriptAllowed()) {
      FireWebUIListener("query-from-launcher", base::Value(query.value()));
    }
  }
}

void JemaAssistantWebUIHandler::OnUiVisibilityChanged(
    AssistantVisibility new_visibility,
    AssistantVisibility old_visibility,
    std::optional<AssistantEntryPoint> entry_point,
    std::optional<AssistantExitPoint> exit_point) {
  if (!IsJavascriptAllowed()) {
    return;
  }
  if (new_visibility == AssistantVisibility::kVisible ||
      new_visibility == AssistantVisibility::kClosed) {
    base::Value::Dict value;
    value.Set("visible", new_visibility == AssistantVisibility::kVisible);
    value.Set("entry",
              static_cast<int>(entry_point.has_value()
                                   ? entry_point.value()
                                   : AssistantEntryPoint::kUnspecified));
    FireWebUIListener("ui-visibility-changed", value);
  }
}

void JemaAssistantWebUIHandler::OnNativeThemeUpdated(
    ui::NativeTheme* observed_theme) {
  if (!IsJavascriptAllowed()) {
    return;
  }
  auto colors = jema_assistant_app_ui_->delegate()->GetSystemColorInfo();
  base::Value::Dict value;
  value.Set("base", ui::ConvertSkColorToCSSColor(colors.base));
  value.Set("shaded", ui::ConvertSkColorToCSSColor(colors.shaded));
  value.Set("header", ui::ConvertSkColorToCSSColor(colors.header));
  value.Set("primary", ui::ConvertSkColorToCSSColor(colors.primary));
  FireWebUIListener("system-color-changed", value);
}

void JemaAssistantWebUIHandler::OnConnectionChanged(
    network::mojom::ConnectionType type) {
  if (!IsJavascriptAllowed()) {
    return;
  }
  FireWebUIListener("network-connection-changed",
                    base::Value(static_cast<int>(type)));
}

void JemaAssistantWebUIHandler::OnBubbleQueryChanged(
    const JemaAssistantViewObserver::ClipboardItemForAssistant& item) {
  if (!IsJavascriptAllowed()) {
    return;
  }
  if (item.display_text.size() > 0) {
    base::Value::Dict value;
    value.Set("query", item.display_text);
    value.Set("format", item.display_format);
    FireWebUIListener("query-from-bubble", value);
  }
}

void JemaAssistantWebUIHandler::OnBubbleVisibilityChanged(bool visible) {
  if (!IsJavascriptAllowed()) {
    return;
  }
  FireWebUIListener("bubble-visibility-changed", base::Value(visible));
}

void JemaAssistantWebUIHandler::OnCreateAgentRequested() {
  if (IsJavascriptAllowed()) {
    FireWebUIListener("create-agent-requested");
  }
}

void JemaAssistantWebUIHandler::OnAgentActivated(const std::string& agent_id) {
  if (IsJavascriptAllowed()) {
    FireWebUIListener("agent-activated", base::Value(agent_id));
  }
}

void JemaAssistantWebUIHandler::OnDeskAgentActivated(
    const std::string& agent_id) {
  if (IsJavascriptAllowed()) {
    FireWebUIListener("desk-agent-activated", base::Value(agent_id));
  }
}

void JemaAssistantWebUIHandler::OnVoiceInputRequested() {
  if (IsJavascriptAllowed()) {
    FireWebUIListener("voice-input-requested");
  }
}

void JemaAssistantWebUIHandler::HandleSetJemaAgentState(
    const base::Value::List& args) {
  if (args.size() != 1u || !args[0].is_list()) {
    return;
  }

  constexpr size_t kMaxAgents = 12;
  constexpr size_t kMaxStringLength = 64;
  std::vector<JemaAssistantAgentSummary> agents;
  for (const auto& value : args[0].GetList()) {
    if (!value.is_dict() || agents.size() >= kMaxAgents) {
      continue;
    }
    const base::Value::Dict& dict = value.GetDict();
    const std::string* id = dict.FindString("id");
    const std::string* name = dict.FindString("name");
    if (!id || !name || id->empty() || id->size() > kMaxStringLength ||
        name->empty() || name->size() > kMaxStringLength) {
      continue;
    }

    JemaAssistantAgentSummary agent;
    agent.id = *id;
    agent.name = base::UTF8ToUTF16(*name);
    agent.progress = std::clamp(dict.FindInt("progress").value_or(0), 0, 100);
    agent.active = dict.FindBool("active").value_or(false);
    const std::string* status = dict.FindString("status");
    agent.working = status && *status == "working";
    agent.autonomous = dict.FindBool("autonomous").value_or(false);
    if (const std::string* color = dict.FindString("color")) {
      SkColor parsed_color;
      if (color->size() <= 9u &&
          content::ParseCssColorString(*color, &parsed_color)) {
        agent.color = parsed_color;
      }
    }
    agents.push_back(std::move(agent));
  }

  for (RootWindowController* controller :
       RootWindowController::root_window_controllers()) {
    Shelf* shelf = controller->shelf();
    if (shelf && shelf->jema_assistant_bar()) {
      shelf->jema_assistant_bar()->SetAgents(agents);
    }
  }
}

void JemaAssistantWebUIHandler::HandleLlmRequest(
    const base::Value::List& args) {
  AllowJavascript();
  if (args.size() != 2u || !args[0].is_string() || !args[1].is_dict()) {
    return;
  }

  base::Value callback_id = args[0].Clone();
  const base::Value::Dict& request = args[1].GetDict();
  const std::string* endpoint = request.FindString("endpoint");
  const std::string* api_key = request.FindString("apiKey");
  const std::string* body = request.FindString("body");
  const std::string* format = request.FindString("format");
  const GURL url(endpoint ? *endpoint : std::string());
  if (!endpoint || !api_key || !body || !format ||
      (*format != "openai" && *format != "anthropic") || api_key->empty() ||
      body->size() > 2 * 1024 * 1024 || !IsAllowedLlmEndpoint(url)) {
    RejectJavascriptCallback(std::move(callback_id),
                             base::Value("Invalid LLM request"));
    return;
  }

  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = url;
  resource_request->method = "POST";
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
  resource_request->headers.SetHeader(net::HttpRequestHeaders::kAccept,
                                      "application/json");
  if (*format == "anthropic") {
    resource_request->headers.SetHeader("x-api-key", *api_key);
    resource_request->headers.SetHeader("anthropic-version", "2023-06-01");
  } else {
    resource_request->headers.SetHeader(net::HttpRequestHeaders::kAuthorization,
                                        "Bearer " + *api_key);
  }

  auto loader = network::SimpleURLLoader::Create(std::move(resource_request),
                                                 kJemaLlmTrafficAnnotation);
  loader->SetAllowHttpErrorResults(true);
  loader->AttachStringForUpload(*body, "application/json");
  auto* loader_ptr = loader.get();
  loader_ptr->DownloadToString(
      web_ui()
          ->GetWebContents()
          ->GetBrowserContext()
          ->GetDefaultStoragePartition()
          ->GetURLLoaderFactoryForBrowserProcess()
          .get(),
      base::BindOnce(&JemaAssistantWebUIHandler::OnLlmRequestComplete,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback_id),
                     std::move(loader)),
      kMaxLlmResponseSize);
}

void JemaAssistantWebUIHandler::HandleExecuteSystemTool(
    const base::Value::List& args) {
  AllowJavascript();
  if (args.size() != 2u || !args[0].is_string() || !args[1].is_dict()) {
    return;
  }
  base::Value callback_id = args[0].Clone();
  jema_assistant_app_ui_->delegate()->ExecuteSystemTool(
      args[1].GetDict(),
      base::BindOnce(&JemaAssistantWebUIHandler::OnSystemToolComplete,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback_id)));
}

void JemaAssistantWebUIHandler::HandleSaveApiKey(
    const base::Value::List& args) {
  AllowJavascript();
  if (args.size() != 3u || !args[0].is_string() || !args[1].is_string() ||
      !args[2].is_string()) {
    return;
  }
  base::Value callback_id = args[0].Clone();
  jema_assistant_app_ui_->delegate()->SaveApiKey(
      args[1].GetString(), args[2].GetString(),
      base::BindOnce(&JemaAssistantWebUIHandler::OnSystemToolComplete,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback_id)));
}

void JemaAssistantWebUIHandler::HandleLoadApiKey(
    const base::Value::List& args) {
  AllowJavascript();
  if (args.size() != 2u || !args[0].is_string() || !args[1].is_string()) {
    return;
  }
  base::Value callback_id = args[0].Clone();
  jema_assistant_app_ui_->delegate()->LoadApiKey(
      args[1].GetString(),
      base::BindOnce(&JemaAssistantWebUIHandler::OnSystemToolComplete,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback_id)));
}

void JemaAssistantWebUIHandler::OnSystemToolComplete(
    base::Value callback_id,
    JemaAssistantAppUIDelegate::ToolResult result) {
  if (result.has_value()) {
    ResolveJavascriptCallback(std::move(callback_id),
                              base::Value(std::move(result.value())));
  } else {
    RejectJavascriptCallback(std::move(callback_id),
                             base::Value(result.error()));
  }
}

void JemaAssistantWebUIHandler::OnLlmRequestComplete(
    base::Value callback_id,
    std::unique_ptr<network::SimpleURLLoader> loader,
    std::unique_ptr<std::string> response_body) {
  base::Value::Dict response;
  int status = 0;
  if (loader->ResponseInfo() && loader->ResponseInfo()->headers) {
    status = loader->ResponseInfo()->headers->response_code();
  }
  response.Set("status", status);
  response.Set("netError", loader->NetError());
  response.Set("body", response_body ? *response_body : std::string());
  ResolveJavascriptCallback(std::move(callback_id),
                            base::Value(std::move(response)));
}

}  // namespace ash
