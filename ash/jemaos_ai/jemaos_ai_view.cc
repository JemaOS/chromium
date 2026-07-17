// Copyright 2025 The jema technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/jemaos_ai/jemaos_ai_view.h"

#include "ash/constants/ash_features.h"
#include "ash/jemaos_ai/jemaos_ai_bubble.h"
#include "ash/public/cpp/ash_web_view_factory.h"
#include "ash/session/session_controller_impl.h"
#include "ash/shell.h"
#include "base/check.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/task/single_thread_task_runner.h"
#include "base/task/task_traits.h"
#include "base/time/time.h"
#include "ui/events/types/event_type.h"
#include "ui/views/background.h"
#include "ui/views/controls/label.h"
#include "ui/wm/core/coordinate_conversion.h"

namespace ash {

namespace {
const int kBubbleInitDelaySeconds = 5;
}

JemaAssistantView::JemaAssistantView(aura::Window* container)
    : window_(container) {
  Shell::Get()->session_controller()->AddObserver(this);
  Shell::Get()->AddPreTargetHandler(this);
  AssistantState::Get()->AddObserver(this);
  enabled_ = AssistantState::Get()->jema_assistant_enabled().value_or(false);
  if (enabled_) {
    ScheduleInitializeBubble();
  }
}

JemaAssistantView::~JemaAssistantView() {
  if (Shell::Get()) {
    Shell::Get()->RemovePreTargetHandler(this);
    Shell::Get()->session_controller()->RemoveObserver(this);
  }
  if (AssistantState::Get()) {
    AssistantState::Get()->RemoveObserver(this);
  }
}

bool JemaAssistantView::IsVisible() const {
  return bubble_ && bubble_->GetWidget()->IsVisible();
}

void JemaAssistantView::AddObserver(JemaAssistantViewObserver* observer) const {
  observers_.AddObserver(observer);
}
void JemaAssistantView::RemoveObserver(
    JemaAssistantViewObserver* observer) const {
  observers_.RemoveObserver(observer);
}

void JemaAssistantView::InitializeBubble() {
  if (bubble_initialized_) {
    return;
  }
  bubble_ = new JemaAssistantBubble(
      window_, gfx::Rect(display::Screen::GetScreen()->GetCursorScreenPoint(),
                         gfx::Size()));
  bubble_->InitWebView(this);
  bubble_initialized_ = true;
}

void JemaAssistantView::ScheduleInitializeBubble() {
  if (bubble_initialized_ || init_scheduled_) {
    return;
  }
  if (JemaAssistantBubble::ReadyToInit()) {
    InitializeBubble();
  } else {
    VLOG(2) << "Not ready to init bubble, schedule it, after "
            << kBubbleInitDelaySeconds << " seconds";
    base::SingleThreadTaskRunner::GetCurrentDefault()->PostDelayedTask(
        FROM_HERE,
        base::BindOnce(&JemaAssistantView::InitializeBubble,
                       weak_factory_.GetWeakPtr()),
        base::Seconds(kBubbleInitDelaySeconds));
    init_scheduled_ = true;
  }
}

void JemaAssistantView::ShowBubble(bool update_anchor_point) {
  if (!enabled_) {
    return;
  }
  if (!ready_to_show_bubble_) {
    show_when_ready_ = true;
    ScheduleInitializeBubble();
    return;
  }
  if (IsVisible()) {
    return;
  }
  Show(update_anchor_point);
}

void JemaAssistantView::ProcessPressedEvent(ui::LocatedEvent* event) {
  if (!bubble_) {
    return;
  }
  gfx::Point screen_location = event->location();
  ::wm::ConvertPointToScreen(static_cast<aura::Window*>(event->target()),
                             &screen_location);
  if (bubble_->GetBoundsInScreen().Contains(screen_location)) {
    return;
  }
  Hide();
}

void JemaAssistantView::OnTouchEvent(ui::TouchEvent* event) {
  if (event->type() == ui::EventType::kTouchPressed) {
    ProcessPressedEvent(event->AsLocatedEvent());
  }
}

void JemaAssistantView::OnMouseEvent(ui::MouseEvent* event) {
  if (event->type() == ui::EventType::kMousePressed) {
    ProcessPressedEvent(event->AsLocatedEvent());
  }
}

void JemaAssistantView::HideBubble() {
  if (!IsVisible()) {
    return;
  }
  Hide();
}

void JemaAssistantView::Show(bool update_anchor_point) {
  if (!ready_to_show_bubble_) {
    return;
  }
  if (!bubble_) {
    return;
  }
  bubble_->SetPreferredSize(bubble_->CalculatePreferredSize({}));
  bubble_->GetWidget()->CenterWindow(bubble_->GetPreferredSize());
  bubble_->GetWidget()->Show();
  for (auto& observer : observers_) {
    observer.OnBubbleVisibilityChanged(IsVisible());
    observer.OnBubbleQueryChanged(last_clipboard_item_);
  }
}

void JemaAssistantView::Hide() {
  if (!bubble_) {
    return;
  }
  bubble_->SetPreferredSize(bubble_->CalculatePreferredSize({}));
  bubble_->GetWidget()->Hide();
  for (auto& observer : observers_) {
    observer.OnBubbleVisibilityChanged(IsVisible());
  }
}

void JemaAssistantView::OnSessionStateChanged(
    session_manager::SessionState state) {
  if (enabled_ && state == session_manager::SessionState::ACTIVE) {
    ScheduleInitializeBubble();
  }
}

void JemaAssistantView::OnChromeTerminating() {
  if (bubble_) {
    bubble_->RemoveWebView();
  }
}

void JemaAssistantView::OnJemaAssistantExtraAcceleratorEnabled(bool enabled) {
  if (enabled && enabled_) {
    ScheduleInitializeBubble();
  }
}

void JemaAssistantView::OnJemaAssistantEnabled(bool enabled) {
  enabled_ = enabled;
  if (enabled_) {
    ScheduleInitializeBubble();
  } else {
    HideBubble();
  }
  for (auto& observer : observers_) {
    observer.OnJemaAssistantEnabledChanged(enabled_);
  }
}

void JemaAssistantView::HandleSendTextToAI(const gfx::Rect& anchor_rect,
                                           const std::u16string& text) {
  if (!enabled_) {
    return;
  }
  if (text.empty()) {
    return;
  }
  last_clipboard_item_.display_format =
      static_cast<int>(crosapi::mojom::ClipboardHistoryDisplayFormat::kText);
  last_clipboard_item_.display_text = text;

  current_anchor_point_ = anchor_rect.origin();
  ShowBubble(false);
}

void JemaAssistantView::UpdateLastClipboardItem(
    const ClipboardHistoryItem& item) {
  if (!enabled_) {
    return;
  }
  VLOG(3) << "clipboard update";
  last_clipboard_item_time_ = base::TimeTicks::Now();
  last_clipboard_item_.display_format = static_cast<int>(item.display_format());
  last_clipboard_item_.display_text = item.display_text();
  // copy from subsystem can be slow,  slower than the gap between pressing
  // ctrl+c and c twice
  auto now = base::TimeTicks::Now();
  if (now - last_time_triggered_ < base::Seconds(1) &&
      should_show_bubble_delay_) {
    VLOG(3) << "pressed twice and clipboard updated, show the bubble now";
    ShowBubble();
  }
}

bool JemaAssistantView::CanHandleTouchSelectionMenuAction() {
  if (!enabled_) {
    return false;
  }
  if (!ready_to_show_bubble_) {
    return false;
  }
  return true;
}

bool JemaAssistantView::CanHandleToggleJemaOSAssistant() {
  if (!enabled_) {
    return false;
  }
  if (!ready_to_show_bubble_) {
    return false;
  }
  auto now = base::TimeTicks::Now();
  auto delta = now - last_time_triggered_;
  VLOG(3) << "jemaos assistant view accelerator, time delta: "
          << delta.InMilliseconds();
  bool triggered_before_clipboard_update =
      last_clipboard_item_time_ < last_time_triggered_;
  last_time_triggered_ = now;
  bool repeated = delta < base::Seconds(1);
  if (repeated && triggered_before_clipboard_update) {
    VLOG(3) << "jemaos assistant accelerator pressed twice, but clipboard not "
               "updated, so the bubble should be shown later";
    should_show_bubble_delay_ = true;
    return false;
  }
  should_show_bubble_delay_ = false;
  VLOG(3) << "can handle jemaos assistant accelerator: " << repeated;
  return repeated;
}

void JemaAssistantView::OnBubbleReady() {
  ready_to_show_bubble_ = true;
  if (show_when_ready_) {
    show_when_ready_ = false;
    Show(/*update_anchor_point=*/true);
  }
  if (create_agent_requested_) {
    create_agent_requested_ = false;
    RequestCreateAgent();
  }
  if (voice_input_requested_) {
    voice_input_requested_ = false;
    RequestVoiceInput();
  }
  if (pending_agent_id_) {
    std::string agent_id = std::move(*pending_agent_id_);
    pending_agent_id_.reset();
    ActivateAgent(agent_id);
  }
}

void JemaAssistantView::RequestCreateAgent() {
  if (!ready_to_show_bubble_) {
    create_agent_requested_ = true;
    return;
  }
  for (auto& observer : observers_) {
    observer.OnCreateAgentRequested();
  }
}

void JemaAssistantView::ActivateAgent(const std::string& agent_id) {
  if (!ready_to_show_bubble_) {
    pending_agent_id_ = agent_id;
    return;
  }
  for (auto& observer : observers_) {
    observer.OnAgentActivated(agent_id);
  }
}

void JemaAssistantView::NotifyDeskAgentActivated(const std::string& agent_id) {
  for (auto& observer : observers_) {
    observer.OnDeskAgentActivated(agent_id);
  }
}

void JemaAssistantView::RequestVoiceInput() {
  if (!ready_to_show_bubble_) {
    voice_input_requested_ = true;
    return;
  }
  for (auto& observer : observers_) {
    observer.OnVoiceInputRequested();
  }
}

void JemaAssistantView::SetBubbleRect(int x, int y, int width, int height) {
  if (bubble_) {
    VLOG(3) << "set bubble rect: " << x << ", " << y << ", " << width << ", "
            << height;
    auto current = bubble_->GetWidget()->GetWindowBoundsInScreen();
    auto newRect = gfx::Rect(x > 0 ? x : current.x(), y > 0 ? y : current.y(),
                             width > 0 ? width : current.width(),
                             height > 0 ? height : current.height());
    bubble_->SetPreferredSize(gfx::Size(newRect.width(), newRect.height()));
    bubble_->SetAnchorRect(gfx::Rect(current_anchor_point_, gfx::Size()));
  }
}

void JemaAssistantView::CenterBubble(int width, int height) {
  if (bubble_) {
    bubble_->GetWidget()->CenterWindow(gfx::Size(width, height));
  }
}

}  // namespace ash
