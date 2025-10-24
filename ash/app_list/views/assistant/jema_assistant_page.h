#ifndef ASH_APP_LIST_VIEWS_ASSISTANT_JEMA_ASSISTANT_PAGE_H_
#define ASH_APP_LIST_VIEWS_ASSISTANT_JEMA_ASSISTANT_PAGE_H_

#include "ash/ash_export.h"
#include "base/memory/raw_ptr.h"
#include "ui/views/view.h"
#include "ash/public/cpp/ash_web_view.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "base/scoped_observation.h"
#include "ash/public/cpp/assistant/controller/assistant_controller.h"
#include "ash/public/cpp/assistant/controller/assistant_controller_observer.h"
#include "ash/assistant/model/assistant_ui_model_observer.h"

namespace ash {

class ASH_EXPORT JemaAssistantPage : public views::View,
                                     public AssistantControllerObserver,
                                     public AssistantUiModelObserver,
                                     public AshWebView::Observer {
  METADATA_HEADER(JemaAssistantPage, views::View)
public:
  JemaAssistantPage();
  JemaAssistantPage(const JemaAssistantPage&) = delete;
  JemaAssistantPage& operator=(const JemaAssistantPage&) =
      delete;
  ~JemaAssistantPage() override;

  // views::View:
  gfx::Size CalculatePreferredSize(
      const views::SizeBounds& available_size) const override;
  void RequestFocus() override;
  void OnBoundsChanged(const gfx::Rect& previous_bounds) override;

  // AssistantWebView::Observer:
  void DidStopLoading() override;

  // AssistantControllerObserver:
  void OnDeepLinkReceived(
      assistant::util::DeepLinkType type,
      const std::map<std::string, std::string>& params) override;

  // ash::AssistantUiModelObserver:
  void OnUiVisibilityChanged(
      AssistantVisibility new_visibility,
      AssistantVisibility old_visibility,
      std::optional<AssistantEntryPoint> entry_point,
      std::optional<AssistantExitPoint> exit_point) override;

  void InitializeUIForBubbleView();

private:
  void InitLayout();
  AshWebView* WebView();
  void OpenUrl(const GURL& url);
  void RemoveContents();

  std::unique_ptr<AshWebView> web_view_;
  raw_ptr<AshWebView> web_view_ptr_ = nullptr;

  base::ScopedObservation<AssistantController, AssistantControllerObserver>
      assistant_controller_observation_{this};
};

} // namespace ash


#endif  // #ifndef ASH_APP_LIST_VIEWS_ASSISTANT_JEMA_ASSISTANT_PAGE_H_
