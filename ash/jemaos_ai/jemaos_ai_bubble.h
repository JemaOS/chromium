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
  static bool ReadyToInit();

  explicit JemaAssistantBubble(aura::Window* container, const gfx::Rect& anchor_rect);

  JemaAssistantBubble(const JemaAssistantBubble&) = delete;
  JemaAssistantBubble& operator=(const JemaAssistantBubble&) =
      delete;
  ~JemaAssistantBubble() override;

  bool InitWebView(JemaAssistantView* owner);

  void RemoveWebView();

  // views::BubbleDialogDelegateView:
  gfx::Size CalculatePreferredSize(const views::SizeBounds& available_size) const override;

 private:
  void OnThemeChanged() override;
  // AshWebView::Observer:
  void DidStopLoading() override;

  bool OpenUrl(const GURL& url);

  std::unique_ptr<AshWebView> web_view_;
  raw_ptr<AshWebView, DanglingUntriaged> web_view_ptr_ = nullptr;
  raw_ptr<JemaAssistantView> owner_ = nullptr;
};

}

#endif // !ASH_JEMAOS_AI_JEMAOS_AI_BUBBLE_H_
