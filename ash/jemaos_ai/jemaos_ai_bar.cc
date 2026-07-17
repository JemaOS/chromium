// Copyright 2026 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/jemaos_ai/jemaos_ai_bar.h"

#include <algorithm>
#include <utility>

#include "ash/jemaos_ai/jemaos_ai_view.h"
#include "ash/resources/vector_icons/vector_icons.h"
#include "ash/root_window_controller.h"
#include "ash/session/session_controller_impl.h"
#include "ash/shell.h"
#include "ash/style/icon_button.h"
#include "ash/wm/desks/desk.h"
#include "ash/wm/desks/desks_controller.h"
#include "ash/wm/desks/desks_histogram_enums.h"
#include "ash/wm/work_area_insets.h"
#include "base/check.h"
#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/timer/timer.h"
#include "cc/paint/paint_flags.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkPath.h"
#include "ui/aura/window.h"
#include "ui/chromeos/styles/cros_tokens_color_mappings.h"
#include "ui/compositor/layer.h"
#include "ui/events/event.h"
#include "ui/events/gesture_event_details.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/font.h"
#include "ui/gfx/font_list.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/paint_vector_icon.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"

namespace ash {

namespace {

constexpr int kBarHeight = 52;
constexpr int kBarVisualHeight = 52;
constexpr int kBarHorizontalPadding = 12;
constexpr int kControlSpacing = 8;
constexpr int kAgentSize = 40;
constexpr int kOrchestratorSize = 44;
constexpr int kHandleWidth = 132;
constexpr int kHandleHeight = 24;
constexpr int kHandleVisibleHeight = 5;
constexpr int kHandleTopMargin = 0;

constexpr SkColor kPurple = SkColorSetRGB(139, 92, 246);

std::unique_ptr<views::LabelButton> CreateControlButton(
    const std::u16string& text,
    const std::u16string& tooltip,
    views::Button::PressedCallback callback,
    int width = 38) {
  auto button = std::make_unique<views::LabelButton>(std::move(callback), text);
  button->SetPreferredSize(gfx::Size(width, 38));
  button->SetTooltipText(tooltip);
  button->SetEnabledTextColors(cros_tokens::kCrosSysOnSurface);
  button->SetBackground(views::CreateThemedRoundedRectBackground(
      cros_tokens::kCrosSysSystemOnBase1, 19.0f));
  return button;
}

std::unique_ptr<IconButton> CreateIconButton(
    views::Button::PressedCallback callback,
    const gfx::VectorIcon& icon,
    const std::u16string& accessible_name,
    IconButton::Type type = IconButton::Type::kSmallFloating) {
  return std::make_unique<IconButton>(
      std::move(callback), type, &icon, accessible_name,
      /*is_togglable=*/false, /*has_border=*/false);
}

class AgentButton : public views::LabelButton {
 public:
  AgentButton(const JemaAssistantAgentSummary& agent,
              base::RepeatingCallback<void(const std::string&)> activate_agent)
      : LabelButton(
            base::BindRepeating(
                [](base::RepeatingCallback<void(const std::string&)> callback,
                   std::string id) { callback.Run(id); },
                std::move(activate_agent),
                agent.id),
            agent.name.empty() ? u"?" : agent.name.substr(0, 1)),
        progress_(agent.progress),
        working_(agent.working),
        autonomous_(agent.autonomous),
        active_(agent.active),
        color_(agent.color) {
    SetPreferredSize(gfx::Size(kAgentSize, kAgentSize));
    SetTooltipText(agent.name);
    SetEnabledTextColors(SK_ColorWHITE);
    SetHorizontalAlignment(gfx::ALIGN_CENTER);
    SetImageLabelSpacing(0);
    label()->SetFontList(
        label()->font_list().DeriveWithWeight(gfx::Font::Weight::BOLD));
    SetBackground(
        views::CreateRoundedRectBackground(agent.color, kAgentSize / 2.0f));
    SetBorder(views::CreateEmptyBorder(gfx::Insets(3)));
  }

  void PaintButtonContents(gfx::Canvas* canvas) override {
    views::LabelButton::PaintButtonContents(canvas);
    if (progress_ <= 0 && !working_ && !active_) {
      return;
    }

    const float stroke_width = active_ ? 3.0f : 2.0f;
    gfx::RectF bounds(GetLocalBounds());
    bounds.Inset(stroke_width / 2.0f);
    SkPath ring;
    ring.addArc(
        SkRect::MakeXYWH(bounds.x(), bounds.y(), bounds.width(),
                         bounds.height()),
        -90.0f,
        active_ && progress_ <= 0 ? 360.0f : 360.0f * progress_ / 100.0f);

    cc::PaintFlags flags;
    flags.setAntiAlias(true);
    flags.setColor(active_ ? SK_ColorWHITE : SkColorSetA(color_, 230));
    flags.setStrokeWidth(stroke_width);
    flags.setStrokeCap(cc::PaintFlags::Cap::kRound_Cap);
    flags.setStyle(cc::PaintFlags::Style::kStroke_Style);
    canvas->DrawPath(ring, flags);
    if (autonomous_) {
      cc::PaintFlags badge;
      badge.setAntiAlias(true);
      badge.setColor(SkColorSetRGB(255, 176, 32));
      canvas->DrawCircle(gfx::PointF(width() - 5.0f, 5.0f), 4.0f, badge);
    }
  }

 private:
  const int progress_;
  const bool working_;
  const bool autonomous_;
  const bool active_;
  const SkColor color_;
};

class RevealButton : public views::LabelButton {
 public:
  explicit RevealButton(views::Button::PressedCallback callback)
      : LabelButton(std::move(callback), std::u16string()) {
    SetPreferredSize(gfx::Size(kHandleWidth, kHandleHeight));
    SetTooltipText(u"Afficher l'environnement IA");
  }

  void PaintButtonContents(gfx::Canvas* canvas) override {
    views::LabelButton::PaintButtonContents(canvas);
    cc::PaintFlags flags;
    flags.setAntiAlias(true);
    flags.setColor(SkColorSetARGB(225, 90, 99, 233));
    const gfx::Rect line((width() - 92) / 2, 1, 92, kHandleVisibleHeight);
    canvas->DrawRoundRect(line, kHandleVisibleHeight / 2, flags);
  }
};

class OrchestratorButton : public IconButton {
 public:
  OrchestratorButton(base::RepeatingClosure show, base::RepeatingClosure voice)
      : IconButton(base::BindRepeating(&OrchestratorButton::OnShortPress,
                                       base::Unretained(this)),
                   IconButton::Type::kLargeProminent,
                   &kJemaAiOrbitIcon,
                   u"Orchestrateur JemaOS AI",
                   false,
                   false),
        show_(std::move(show)),
        voice_(std::move(voice)) {
    SetPreferredSize(gfx::Size(kOrchestratorSize, kOrchestratorSize));
    SetIconColor(SK_ColorWHITE);
    SetBackgroundColor(kPurple);
  }

  bool OnMousePressed(const ui::MouseEvent& event) override {
    long_press_triggered_ = false;
    long_press_timer_.Start(FROM_HERE, base::Milliseconds(650),
                            base::BindOnce(&OrchestratorButton::TriggerVoice,
                                           base::Unretained(this)));
    return IconButton::OnMousePressed(event);
  }

  void OnMouseReleased(const ui::MouseEvent& event) override {
    long_press_timer_.Stop();
    IconButton::OnMouseReleased(event);
  }

  void OnGestureEvent(ui::GestureEvent* event) override {
    if (event->type() == ui::EventType::kGestureLongPress) {
      TriggerVoice();
      event->SetHandled();
      return;
    }
    IconButton::OnGestureEvent(event);
  }

 private:
  void OnShortPress() {
    if (!long_press_triggered_) {
      show_.Run();
    }
    long_press_triggered_ = false;
  }

  void TriggerVoice() {
    long_press_triggered_ = true;
    voice_.Run();
  }

  base::RepeatingClosure show_;
  base::RepeatingClosure voice_;
  base::OneShotTimer long_press_timer_;
  bool long_press_triggered_ = false;
};

}  // namespace

class JemaAssistantBarView : public views::View {
 public:
  JemaAssistantBarView(
      base::RepeatingClosure show_assistant,
      base::RepeatingClosure voice_input,
      base::RepeatingClosure create_agent,
      base::RepeatingCallback<void(const std::string&)> activate_agent,
      base::RepeatingClosure activate_user_desk,
      base::RepeatingClosure hide_bar) {
    auto* layout = SetLayoutManager(std::make_unique<views::BoxLayout>(
        views::BoxLayout::Orientation::kHorizontal,
        gfx::Insets::VH((kBarVisualHeight - kAgentSize) / 2,
                        kBarHorizontalPadding),
        kControlSpacing));
    layout->set_cross_axis_alignment(
        views::BoxLayout::CrossAxisAlignment::kCenter);

    auto home_button = CreateIconButton(
        base::BindRepeating(
            [](base::RepeatingClosure callback) { callback.Run(); },
            std::move(activate_user_desk)),
        kKsvBrowserHomeIcon, u"Revenir au bureau utilisateur");
    home_button->SetIconColor(cros_tokens::kCrosSysOnSurface);
    home_button->SetBackgroundColor(cros_tokens::kCrosSysSystemOnBase1);
    AddChildView(std::move(home_button));

    agents_container_ = AddChildView(std::make_unique<views::View>());
    auto* agents_layout =
        agents_container_->SetLayoutManager(std::make_unique<views::BoxLayout>(
            views::BoxLayout::Orientation::kHorizontal, gfx::Insets(),
            kControlSpacing));
    agents_layout->set_cross_axis_alignment(
        views::BoxLayout::CrossAxisAlignment::kCenter);
    create_agent_ = std::move(create_agent);
    activate_agent_ = std::move(activate_agent);
    RebuildAgents({});
    layout->SetFlexForView(agents_container_, 1);

    std::unique_ptr<views::View> orchestrator =
        std::make_unique<OrchestratorButton>(std::move(show_assistant),
                                             std::move(voice_input));
    AddChildView(std::move(orchestrator));

    auto* right = AddChildView(std::make_unique<views::View>());
    auto* right_layout =
        right->SetLayoutManager(std::make_unique<views::BoxLayout>(
            views::BoxLayout::Orientation::kHorizontal, gfx::Insets(),
            kControlSpacing));
    right_layout->set_main_axis_alignment(
        views::BoxLayout::MainAxisAlignment::kEnd);
    right_layout->set_cross_axis_alignment(
        views::BoxLayout::CrossAxisAlignment::kCenter);
    auto notification_button =
        CreateIconButton(views::Button::PressedCallback(),
                         kShelfNotificationsIcon, u"Notifications");
    notification_button->SetIconColor(cros_tokens::kCrosSysOnSurface);
    notification_button->SetBackgroundColor(cros_tokens::kCrosSysSystemOnBase1);
    right->AddChildView(std::move(notification_button));
    auto hide_button = CreateIconButton(
        base::BindRepeating(
            [](base::RepeatingClosure callback) { callback.Run(); },
            std::move(hide_bar)),
        kChevronUpIcon, u"Masquer l'environnement IA");
    hide_button->SetIconColor(cros_tokens::kCrosSysOnSurface);
    hide_button->SetBackgroundColor(cros_tokens::kCrosSysSystemOnBase1);
    right->AddChildView(std::move(hide_button));
    layout->SetFlexForView(right, 1);

    SetPaintToLayer();
    layer()->SetFillsBoundsOpaquely(false);
    layer()->SetRoundedCornerRadius(
        gfx::RoundedCornersF(0.0f, 0.0f, 18.0f, 18.0f));
    SetBackground(views::CreateThemedSolidBackground(
        cros_tokens::kCrosSysSystemBaseElevated));
  }

  JemaAssistantBarView(const JemaAssistantBarView&) = delete;
  JemaAssistantBarView& operator=(const JemaAssistantBarView&) = delete;
  ~JemaAssistantBarView() override = default;

  void SetAgents(const std::vector<JemaAssistantAgentSummary>& agents) {
    RebuildAgents(agents);
  }

 private:
  void RebuildAgents(const std::vector<JemaAssistantAgentSummary>& agents) {
    agents_container_->RemoveAllChildViews();
    if (agents.empty()) {
      agents_container_->AddChildView(
          CreateControlButton(u"Agents", u"Aucun agent configure",
                              views::Button::PressedCallback(), 72));
    } else {
      for (const auto& agent : agents) {
        std::unique_ptr<views::View> button =
            std::make_unique<AgentButton>(agent, activate_agent_);
        agents_container_->AddChildView(std::move(button));
      }
    }
    auto add_agent = std::make_unique<views::LabelButton>(
        base::BindRepeating(
            [](base::RepeatingClosure callback) { callback.Run(); },
            create_agent_),
        u"+");
    add_agent->SetPreferredSize(gfx::Size(kAgentSize, kAgentSize));
    add_agent->SetTooltipText(u"Creer un agent");
    add_agent->SetEnabledTextColors(cros_tokens::kCrosSysOnSurface);
    add_agent->SetHorizontalAlignment(gfx::ALIGN_CENTER);
    add_agent->SetBackground(views::CreateThemedRoundedRectBackground(
        cros_tokens::kCrosSysSystemOnBase1, kAgentSize / 2.0f));
    agents_container_->AddChildView(std::move(add_agent));
    agents_container_->InvalidateLayout();
    PreferredSizeChanged();
    agents_container_->SchedulePaint();
  }

  raw_ptr<views::View> agents_container_ = nullptr;
  base::RepeatingClosure create_agent_;
  base::RepeatingCallback<void(const std::string&)> activate_agent_;
};

JemaAssistantBar::JemaAssistantBar(aura::Window* root_window,
                                   aura::Window* container,
                                   JemaAssistantView* assistant_view)
    : root_window_(root_window), assistant_view_(assistant_view) {
  CHECK(root_window_);
  CHECK(container);
  root_window_->AddObserver(this);
  Shell::Get()->session_controller()->AddObserver(this);
  if (DesksController::Get()) {
    DesksController::Get()->AddObserver(this);
    user_desk_ = DesksController::Get()->active_desk();
  }
  session_active_ = Shell::Get()->session_controller()->GetSessionState() ==
                    session_manager::SessionState::ACTIVE;
  if (assistant_view_) {
    assistant_view_->AddObserver(this);
  }
  CreateWidgets(container);
  LayoutWidgets();
  UpdateVisibility();
}

JemaAssistantBar::~JemaAssistantBar() {
  if (DesksController::Get()) {
    DesksController::Get()->RemoveObserver(this);
  }
  if (assistant_view_) {
    assistant_view_->RemoveObserver(this);
  }
  if (Shell::HasInstance()) {
    Shell::Get()->session_controller()->RemoveObserver(this);
  }
  if (root_window_) {
    RootWindowController::ForWindow(root_window_)
        ->work_area_insets()
        ->SetJemaAssistantBarHeight(0);
    root_window_->RemoveObserver(this);
  }
}

void JemaAssistantBar::SetVisible(bool visible) {
  if (expanded_ == visible) {
    return;
  }
  expanded_ = visible;
  UpdateVisibility();
}

void JemaAssistantBar::ToggleVisibility() {
  SetVisible(!expanded_);
}

void JemaAssistantBar::OnWindowBoundsChanged(aura::Window* window,
                                             const gfx::Rect& old_bounds,
                                             const gfx::Rect& new_bounds,
                                             ui::PropertyChangeReason reason) {
  if (window == root_window_ && old_bounds.size() != new_bounds.size()) {
    LayoutWidgets();
  }
}

void JemaAssistantBar::OnWindowDestroying(aura::Window* window) {
  if (window != root_window_) {
    return;
  }
  root_window_->RemoveObserver(this);
  root_window_ = nullptr;
}

void JemaAssistantBar::OnSessionStateChanged(
    session_manager::SessionState state) {
  session_active_ = state == session_manager::SessionState::ACTIVE;
  UpdateVisibility();
}

void JemaAssistantBar::OnDeskActivationChanged(const Desk* activated,
                                               const Desk* deactivated) {
  if (!activated) {
    return;
  }
  bool changed = false;
  std::string active_agent_id;
  for (auto& agent : agents_) {
    const bool active = agent.name == activated->name();
    changed |= agent.active != active;
    agent.active = active;
    if (active) {
      active_agent_id = agent.id;
    }
  }
  if (changed && bar_view_) {
    bar_view_->SetAgents(agents_);
  }
  if (!active_agent_id.empty() && assistant_view_) {
    assistant_view_->NotifyDeskAgentActivated(active_agent_id);
  }
}

void JemaAssistantBar::OnDeskRemoved(const Desk* desk) {
  if (desk != user_desk_) {
    return;
  }
  user_desk_ = nullptr;
  if (DesksController::Get() && !DesksController::Get()->desks().empty()) {
    user_desk_ = DesksController::Get()->desks().front().get();
  }
}

void JemaAssistantBar::CreateWidgets(aura::Window* container) {
  auto bar_widget = std::make_unique<views::Widget>();
  views::Widget::InitParams bar_params(
      views::Widget::InitParams::WIDGET_OWNS_NATIVE_WIDGET,
      views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
  bar_params.name = "JemaAssistantBar";
  bar_params.opacity = views::Widget::InitParams::WindowOpacity::kTranslucent;
  bar_params.parent = container;
  bar_widget->Init(std::move(bar_params));
  bar_widget->set_focus_on_creation(false);
  auto bar_view = std::make_unique<JemaAssistantBarView>(
      base::BindRepeating(&JemaAssistantBar::ShowAssistant,
                          base::Unretained(this)),
      base::BindRepeating(&JemaAssistantBar::StartVoiceInput,
                          base::Unretained(this)),
      base::BindRepeating(&JemaAssistantBar::CreateAgent,
                          base::Unretained(this)),
      base::BindRepeating(&JemaAssistantBar::ActivateAgent,
                          base::Unretained(this)),
      base::BindRepeating(&JemaAssistantBar::ActivateUserDesk,
                          base::Unretained(this)),
      base::BindRepeating(&JemaAssistantBar::SetVisible, base::Unretained(this),
                          false));
  bar_view_ = bar_view.get();
  bar_widget->SetContentsView(std::move(bar_view));
  bar_widget_ = std::move(bar_widget);
  bar_widget_->GetNativeWindow()->parent()->StackChildAtTop(
      bar_widget_->GetNativeWindow());
  bar_widget_->Hide();

  auto handle_widget = std::make_unique<views::Widget>();
  views::Widget::InitParams handle_params(
      views::Widget::InitParams::WIDGET_OWNS_NATIVE_WIDGET,
      views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
  handle_params.name = "JemaAssistantBarShowHandle";
  handle_params.opacity =
      views::Widget::InitParams::WindowOpacity::kTranslucent;
  handle_params.parent = container;
  handle_widget->Init(std::move(handle_params));
  handle_widget->set_focus_on_creation(false);
  auto handle_button = std::make_unique<RevealButton>(base::BindRepeating(
      &JemaAssistantBar::SetVisible, base::Unretained(this), true));
  std::unique_ptr<views::View> handle_contents = std::move(handle_button);
  handle_widget->SetContentsView(std::move(handle_contents));
  show_handle_widget_ = std::move(handle_widget);
  show_handle_widget_->GetNativeWindow()->parent()->StackChildAtTop(
      show_handle_widget_->GetNativeWindow());
  show_handle_widget_->Hide();
}

void JemaAssistantBar::UpdateVisibility() {
  const bool show_bar = visible();
  if (show_bar) {
    bar_widget_->Show();
  } else {
    bar_widget_->Hide();
  }
  if (session_active_ && !expanded_) {
    show_handle_widget_->Show();
  } else {
    show_handle_widget_->Hide();
  }

  if (!show_bar && assistant_view_) {
    assistant_view_->HideBubble();
  }
  UpdateWorkArea();
}

void JemaAssistantBar::LayoutWidgets() {
  if (!root_window_) {
    return;
  }
  const gfx::Rect root_bounds = root_window_->bounds();
  bar_widget_->SetBounds(
      gfx::Rect(0, 0, root_bounds.width(), kBarVisualHeight));
  show_handle_widget_->SetBounds(
      gfx::Rect((root_bounds.width() - kHandleWidth) / 2, kHandleTopMargin,
                kHandleWidth, kHandleHeight));
}

void JemaAssistantBar::UpdateWorkArea() {
  if (!root_window_) {
    return;
  }
  RootWindowController::ForWindow(root_window_)
      ->work_area_insets()
      ->SetJemaAssistantBarHeight(visible() ? kBarHeight : 0);
}

void JemaAssistantBar::ShowAssistant() {
  if (assistant_view_ && session_active_) {
    assistant_view_->ShowBubble(/*update_anchor_point=*/true);
  }
}

void JemaAssistantBar::StartVoiceInput() {
  ShowAssistant();
  if (assistant_view_) {
    assistant_view_->RequestVoiceInput();
  }
}

void JemaAssistantBar::CreateAgent() {
  ShowAssistant();
  if (assistant_view_) {
    assistant_view_->RequestCreateAgent();
  }
}

void JemaAssistantBar::ActivateAgent(const std::string& agent_id) {
  const auto it =
      std::find_if(agents_.begin(), agents_.end(),
                   [&agent_id](const JemaAssistantAgentSummary& agent) {
                     return agent.id == agent_id;
                   });
  if (it != agents_.end()) {
    Desk* target = EnsureAgentDesk(*it);
    if (target && !target->is_active()) {
      DesksController::Get()->ActivateDesk(target,
                                           DesksSwitchSource::kApiSwitch);
    }
  }
  if (assistant_view_ && assistant_view_->IsVisible()) {
    assistant_view_->ActivateAgent(agent_id);
  }
}

void JemaAssistantBar::ActivateUserDesk() {
  DesksController* controller = DesksController::Get();
  if (controller && user_desk_ && !user_desk_->is_active()) {
    controller->ActivateDesk(user_desk_, DesksSwitchSource::kApiSwitch);
  }
}

void JemaAssistantBar::SetAgents(
    std::vector<JemaAssistantAgentSummary> agents) {
  agents_ = agents;
  const auto active_agent = std::find_if(
      agents_.begin(), agents_.end(),
      [](const JemaAssistantAgentSummary& agent) { return agent.active; });
  if (active_agent != agents_.end()) {
    Desk* target = EnsureAgentDesk(*active_agent);
    if (target && !target->is_active()) {
      DesksController::Get()->ActivateDesk(target,
                                           DesksSwitchSource::kApiSwitch);
    }
  }
  if (bar_view_) {
    bar_view_->SetAgents(agents);
  }
}

Desk* JemaAssistantBar::EnsureAgentDesk(
    const JemaAssistantAgentSummary& agent) {
  DesksController* controller = DesksController::Get();
  if (!controller) {
    return nullptr;
  }

  Desk* target = nullptr;
  for (const auto& desk : controller->desks()) {
    if (desk->name() == agent.name) {
      target = desk.get();
      break;
    }
  }
  if (!target && controller->CanCreateDesks()) {
    controller->NewDesk(DesksCreationRemovalSource::kButton, agent.name);
    target = controller->desks().back().get();
  }
  return target;
}

}  // namespace ash
