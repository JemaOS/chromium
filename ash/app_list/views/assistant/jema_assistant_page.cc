#include "ash/app_list/views/assistant/jema_assistant_page.h"
#include "ash/public/cpp/assistant/controller/assistant_ui_controller.h"
#include "ash/assistant/model/assistant_ui_model.h"
#include "ui/views/border.h"
#include "ui/views/layout/fill_layout.h"
#include "ash/public/cpp/ash_web_view_factory.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ash/assistant/util/deep_link_util.h"

namespace ash {

namespace {
const char kJemaAssistantExtensionUrl[] = "chrome://jemaos-ai/?source=launcher";
constexpr int kHeightDip = 440;
} // namespace

JemaAssistantPage::JemaAssistantPage() {
  InitLayout();
  assistant_controller_observation_.Observe(AssistantController::Get());
  AssistantUiController::Get()->GetModel()->AddObserver(this);
}

JemaAssistantPage::~JemaAssistantPage() {
  if (AssistantUiController::Get())
    AssistantUiController::Get()->GetModel()->RemoveObserver(this);
}

void JemaAssistantPage::OnBoundsChanged(
    const gfx::Rect& previous_bounds) {
  if (AssistantUiController::Get())
    AssistantUiController::Get()->SetAppListBubbleWidth(size().width());
}

void JemaAssistantPage::RequestFocus() {
  VLOG(2) << "JemaAssistantPage::RequestFocus()";
  if (web_view_ptr_) {
    web_view_ptr_->GetInitiallyFocusedView()->RequestFocus();
  }
}

void JemaAssistantPage::InitLayout() {
  SetLayoutManager(std::make_unique<views::FillLayout>());
}

AshWebView* JemaAssistantPage::WebView() {
  return web_view_ptr_ ? web_view_ptr_.get() : web_view_.get();
}

void JemaAssistantPage::OnDeepLinkReceived(
    assistant::util::DeepLinkType type,
    const std::map<std::string, std::string>& params) {
  if (type == assistant::util::DeepLinkType::kQuery) {
    const absl::optional<std::string>& query =
        GetDeepLinkParam(params, assistant::util::DeepLinkParam::kQuery);

    if (!query.has_value())
      return;
    AssistantUiController::Get()->ShowUi(assistant::AssistantEntryPoint::kDeepLink);
    OpenUrl(GURL(base::StringPrintf("%s&initQuery=%s", kJemaAssistantExtensionUrl, query->c_str())));
  }
}

void JemaAssistantPage::OnUiVisibilityChanged(
      AssistantVisibility new_visibility,
      AssistantVisibility old_visibility,
      absl::optional<AssistantEntryPoint> entry_point,
      absl::optional<AssistantExitPoint> exit_point) {
  if (new_visibility == AssistantVisibility::kVisible && entry_point.has_value() && (entry_point == AssistantEntryPoint::kHotkey || entry_point == AssistantEntryPoint::kLauncherSearchBoxIcon)) {
    OpenUrl(GURL(kJemaAssistantExtensionUrl));
  }
}

void JemaAssistantPage::InitializeUIForBubbleView() {
  // called from AppListBubbleView::InitializeUIForBubbleView
  VLOG(2) << "JemaAssistantPage::InitializeUIForBubbleView()";
}

gfx::Size JemaAssistantPage::CalculatePreferredSize() const {
  return gfx::Size(INT_MAX, kHeightDip);
}

void JemaAssistantPage::OpenUrl(const GURL& url) {
  if (web_view_ptr_ || web_view_) {
    return;
  }
  // RemoveContents();
  web_view_ = AshWebViewFactory::Get()->Create(AshWebView::InitParams());
  WebView()->AddObserver(this);
  WebView()->Navigate(url);
}

void JemaAssistantPage::DidStopLoading() {
  if (!web_view_) {
    return;
  }

  web_view_->SetPreferredSize(GetPreferredSize());
  web_view_ptr_ = AddChildView(std::move(web_view_));
  web_view_ptr_->SetBorder(views::CreateEmptyBorder(0));
  web_view_ptr_->GetInitiallyFocusedView()->RequestFocus();
}

void JemaAssistantPage::RemoveContents() {
  if (!web_view_ptr_) {
    return;
  }
  RemoveChildViewT(web_view_ptr_.get())->RemoveObserver(this);
  web_view_ptr_ = nullptr;
}

BEGIN_METADATA(JemaAssistantPage, views::View)
END_METADATA

} // namespace ash
