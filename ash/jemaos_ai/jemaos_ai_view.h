#ifndef ASH_JEMAOS_AI_JEMAOS_AI_VIEW_H_
#define ASH_JEMAOS_AI_JEMAOS_AI_VIEW_H_

#include <memory>
#include "ash/ash_export.h"
#include "ash/clipboard/clipboard_history_item.h"
#include "base/memory/raw_ptr.h"
#include "ash/public/cpp/session/session_observer.h"
#include "base/observer_list.h"
#include "base/observer_list_types.h"
#include "ui/views/widget/widget_observer.h"

namespace views {
class View;
}

namespace aura {
class Window;
}

namespace ash {

class JemaAssistantViewObserver : public base::CheckedObserver  {
 public:
  struct ClipboardItemForAssistant {
    std::u16string display_text;
    int            display_format;
  };

  JemaAssistantViewObserver(const JemaAssistantViewObserver&) = delete;
  JemaAssistantViewObserver& operator=(const JemaAssistantViewObserver&) = delete;

  virtual void OnBubbleQueryChanged(const ClipboardItemForAssistant& item) {}
  virtual void OnBubbleVisibilityChanged(bool visible) {}
 protected:
  JemaAssistantViewObserver() = default;
  ~JemaAssistantViewObserver() override = default;
};

class JemaAssistantBubble;

class ASH_EXPORT JemaAssistantView : public SessionObserver, public views::WidgetObserver {
 public:
  JemaAssistantView();

  JemaAssistantView(const JemaAssistantView&) = delete;
  JemaAssistantView& operator=(const JemaAssistantView&) = delete;

  ~JemaAssistantView() override;

  void AddObserver(JemaAssistantViewObserver* observer) const;
  void RemoveObserver(JemaAssistantViewObserver* observer) const;

  void CreateAssistantWidget(aura::Window* window);

  void ShowBubble();
  void HideBubble();

  void UpdateLastClipboardItem(const ClipboardHistoryItem& item);
  bool CanHandleToggleJemaOSAssistant();

  void OnBubbleReady();

  void SetBubbleRect(int x, int y, int width, int height);

 private:
  void OnSessionStateChanged(session_manager::SessionState state) override;

  void OnWidgetActivationChanged(views::Widget* widget, bool active) override;
  void OnWidgetClosing(views::Widget* widget) override;

  void Show();
  void Hide();

  bool visible_ = false;
  bool can_show_ = false;
  bool should_show_bubble_delay_ = false;
  base::TimeTicks last_clipboard_item_time_ = base::TimeTicks::Min();
  base::TimeTicks last_time_triggered_ = base::TimeTicks::Min();
  JemaAssistantViewObserver::ClipboardItemForAssistant last_clipboard_item_;
  aura::Window* window_ = nullptr;
  raw_ptr<JemaAssistantBubble, ExperimentalAsh> bubble_;

  gfx::Point current_anchor_point_ = gfx::Point();

  mutable base::ObserverList<JemaAssistantViewObserver> observers_;
};

}  // namespace ash

#endif // !ASH_JEMAOS_AI_JEMAOS_AI_VIEW_H_
