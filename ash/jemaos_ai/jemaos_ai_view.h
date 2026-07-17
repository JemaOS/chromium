#ifndef ASH_JEMAOS_AI_JEMAOS_AI_VIEW_H_
#define ASH_JEMAOS_AI_JEMAOS_AI_VIEW_H_

#include <memory>
#include <optional>
#include <string>

#include "ash/ash_export.h"
#include "ash/clipboard/clipboard_history_item.h"
#include "ash/public/cpp/assistant/assistant_state.h"
#include "ash/public/cpp/session/session_observer.h"
#include "base/memory/raw_ptr.h"
#include "base/observer_list.h"
#include "base/observer_list_types.h"
#include "ui/events/event.h"
#include "ui/events/event_handler.h"

namespace views {
class View;
}

namespace aura {
class Window;
}

namespace ash {

class JemaAssistantViewObserver : public base::CheckedObserver {
 public:
  struct ClipboardItemForAssistant {
    std::u16string display_text;
    int display_format;
  };

  JemaAssistantViewObserver(const JemaAssistantViewObserver&) = delete;
  JemaAssistantViewObserver& operator=(const JemaAssistantViewObserver&) =
      delete;

  virtual void OnBubbleQueryChanged(const ClipboardItemForAssistant& item) {}
  virtual void OnBubbleVisibilityChanged(bool visible) {}
  virtual void OnCreateAgentRequested() {}
  virtual void OnAgentActivated(const std::string& agent_id) {}
  virtual void OnDeskAgentActivated(const std::string& agent_id) {}
  virtual void OnVoiceInputRequested() {}
  virtual void OnJemaAssistantEnabledChanged(bool enabled) {}

 protected:
  JemaAssistantViewObserver() = default;
  ~JemaAssistantViewObserver() override = default;
};

class JemaAssistantBubble;

class ASH_EXPORT JemaAssistantView : public SessionObserver,
                                     public ui::EventHandler,
                                     public AssistantStateObserver {
 public:
  explicit JemaAssistantView(aura::Window* container);

  JemaAssistantView(const JemaAssistantView&) = delete;
  JemaAssistantView& operator=(const JemaAssistantView&) = delete;

  ~JemaAssistantView() override;

  void AddObserver(JemaAssistantViewObserver* observer) const;
  void RemoveObserver(JemaAssistantViewObserver* observer) const;

  bool IsVisible() const;
  bool enabled() const { return enabled_; }

  void ShowBubble(bool update_anchor_point = true);
  void HideBubble();

  void UpdateLastClipboardItem(const ClipboardHistoryItem& item);
  bool CanHandleToggleJemaOSAssistant();

  bool CanHandleTouchSelectionMenuAction();
  void HandleSendTextToAI(const gfx::Rect& anchor_rect,
                          const std::u16string& text);

  void OnBubbleReady();

  void RequestCreateAgent();
  void ActivateAgent(const std::string& agent_id);
  void NotifyDeskAgentActivated(const std::string& agent_id);
  void RequestVoiceInput();

  void SetBubbleRect(int x, int y, int width, int height);

  void CenterBubble(int width, int height);

 private:
  void OnSessionStateChanged(session_manager::SessionState state) override;
  void OnChromeTerminating() override;

  void OnMouseEvent(ui::MouseEvent* event) override;
  void OnTouchEvent(ui::TouchEvent* event) override;

  void ProcessPressedEvent(ui::LocatedEvent* event);

  void OnJemaAssistantExtraAcceleratorEnabled(bool enabled) override;
  void OnJemaAssistantEnabled(bool enabled) override;

  void Show(bool update_anchor_point);
  void Hide();

  void InitializeBubble();
  void ScheduleInitializeBubble();

  bool init_scheduled_ = false;
  bool enabled_ = false;
  bool ready_to_show_bubble_ = false;
  bool bubble_initialized_ = false;
  bool should_show_bubble_delay_ = false;
  bool show_when_ready_ = false;
  bool create_agent_requested_ = false;
  bool voice_input_requested_ = false;
  std::optional<std::string> pending_agent_id_;
  base::TimeTicks last_clipboard_item_time_ = base::TimeTicks::Min();
  base::TimeTicks last_time_triggered_ = base::TimeTicks::Min();
  JemaAssistantViewObserver::ClipboardItemForAssistant last_clipboard_item_;
  raw_ptr<JemaAssistantBubble, DanglingUntriaged> bubble_;

  raw_ptr<aura::Window, DanglingUntriaged> window_ = nullptr;

  gfx::Point current_anchor_point_;

  mutable base::ObserverList<JemaAssistantViewObserver> observers_;

  base::WeakPtrFactory<JemaAssistantView> weak_factory_{this};
};

}  // namespace ash

#endif  // !ASH_JEMAOS_AI_JEMAOS_AI_VIEW_H_
