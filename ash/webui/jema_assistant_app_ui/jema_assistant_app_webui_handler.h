#ifndef ASH_WEBUI_JEMA_ASSISTANT_APP_UI_JEMA_ASSISTANT_APP_WEBUI_HANDLER_H_
#define ASH_WEBUI_JEMA_ASSISTANT_APP_UI_JEMA_ASSISTANT_APP_WEBUI_HANDLER_H_

#include "ash/assistant/model/assistant_ui_model_observer.h"
#include "ash/jemaos_ai/jemaos_ai_view.h"
#include "ash/public/cpp/assistant/controller/assistant_controller.h"
#include "ash/public/cpp/assistant/controller/assistant_controller_observer.h"
#include "ash/webui/jema_assistant_app_ui/jema_assistant_app_ui.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/web_ui_message_handler.h"
#include "services/network/public/cpp/network_connection_tracker.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "ui/native_theme/native_theme.h"
#include "ui/native_theme/native_theme_observer.h"

namespace ash {

class JemaAssistantWebUIHandler
    : public content::WebUIMessageHandler,
      public network::NetworkConnectionTracker::NetworkConnectionObserver,
      public ui::NativeThemeObserver,
      public AssistantUiModelObserver,
      public JemaAssistantViewObserver,
      public AssistantControllerObserver {
 public:
  explicit JemaAssistantWebUIHandler(JemaAssistantAppUI* jemaAsisstantAppUi);

  JemaAssistantWebUIHandler(const JemaAssistantWebUIHandler&) = delete;
  JemaAssistantWebUIHandler& operator=(const JemaAssistantWebUIHandler&) =
      delete;

  ~JemaAssistantWebUIHandler() override;

  // content::WebUIMessageHandler:
  void RegisterMessages() override;

  // ash::AssistantUiModelObserver:
  void OnUiVisibilityChanged(
      AssistantVisibility new_visibility,
      AssistantVisibility old_visibility,
      std::optional<AssistantEntryPoint> entry_point,
      std::optional<AssistantExitPoint> exit_point) override;

  // AssistantControllerObserver:
  void OnDeepLinkReceived(
      assistant::util::DeepLinkType type,
      const std::map<std::string, std::string>& params) override;

  // ui::NativeThemeObserver:
  void OnNativeThemeUpdated(ui::NativeTheme* observed_theme) override;

  // network::NetworkConnectionTracker::NetworkConnectionObserver:
  void OnConnectionChanged(network::mojom::ConnectionType type) override;

  void OnBubbleQueryChanged(
      const JemaAssistantViewObserver::ClipboardItemForAssistant& item)
      override;
  void OnBubbleVisibilityChanged(bool visible) override;
  void OnCreateAgentRequested() override;
  void OnAgentActivated(const std::string& agent_id) override;
  void OnAgentVoiceInputRequested(const std::string& agent_id) override;
  void OnDeskAgentActivated(const std::string& agent_id) override;
  void OnVoiceInputRequested() override;

 private:
  void OnJemaAssistantSwaInit(const base::Value::List& args);
  void OnRequestCloseAssistant(const base::Value::List& args);

  void OnJemaAssistantOpenUrl(const base::Value::List& args);
  void HandleSetAssistantBubbleRect(const base::Value::List& args);
  void HandleCenterAssistantBubbleRect(const base::Value::List& args);
  void HandleSetJemaAgentState(const base::Value::List& args);
  void HandleLlmRequest(const base::Value::List& args);
  void HandleExecuteSystemTool(const base::Value::List& args);
  void HandleSaveApiKey(const base::Value::List& args);
  void HandleLoadApiKey(const base::Value::List& args);
  void OnSystemToolComplete(base::Value callback_id,
                            JemaAssistantAppUIDelegate::ToolResult result);
  void OnLlmRequestComplete(base::Value callback_id,
                            std::unique_ptr<network::SimpleURLLoader> loader,
                            std::unique_ptr<std::string> response_body);

  raw_ptr<JemaAssistantAppUI> jema_assistant_app_ui_;

  base::ScopedObservation<AssistantController, AssistantControllerObserver>
      assistant_controller_observation_{this};

  base::ScopedObservation<ui::NativeTheme, ui::NativeThemeObserver>
      theme_observation_{this};

  base::WeakPtrFactory<JemaAssistantWebUIHandler> weak_ptr_factory_{this};
};

}  // namespace ash

#endif  // !#ifndef
        // ASH_WEBUI_JEMA_ASSISTANT_APP_UI_JEMA_ASSISTANT_APP_WEBUI_HANDLER_H_
