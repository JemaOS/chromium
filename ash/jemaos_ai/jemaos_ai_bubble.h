#ifndef ASH_JEMAOS_AI_JEMAOS_AI_BUBBLE_H_
#define ASH_JEMAOS_AI_JEMAOS_AI_BUBBLE_H_

#include "ash/ash_export.h"
#include "ash/jemaos_ai/jemaos_ai_view.h"
#include "ui/views/bubble/bubble_dialog_delegate_view.h"
#include "ash/public/cpp/ash_web_view.h"

namespace aura {
class Window;
}

namespace ash {

class ASH_EXPORT JemaAssistantBubble : public views::BubbleDialogDelegateView,
                                       public AshWebView::Observer {
 public:
  explicit JemaAssistantBubble(aura::Window* window, const gfx::Rect& anchor_rect);

  JemaAssistantBubble(const JemaAssistantBubble&) = delete;
  JemaAssistantBubble& operator=(const JemaAssistantBubble&) =
      delete;
  ~JemaAssistantBubble() override;

  void OpenWebView(JemaAssistantView* owner);

  // views::BubbleDialogDelegateView:
  void Init() override;
  gfx::Size CalculatePreferredSize() const override;
  void OnThemeChanged() override;

 private:
  // AshWebView::Observer:
  void DidStopLoading() override;

  AshWebView* WebView();
  void OpenUrl(const GURL& url);

  std::unique_ptr<AshWebView> web_view_;
  raw_ptr<AshWebView, ExperimentalAsh> web_view_ptr_ = nullptr;
  raw_ptr<JemaAssistantView, ExperimentalAsh> owner_ = nullptr;
};

}

#endif // !ASH_JEMAOS_AI_JEMAOS_AI_BUBBLE_H_
