// Copyright 2023 The Jema Innovations Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/jemaos_ai/jemaos_ai_view.h"
#include "base/check.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/time/time.h"
#include "ui/views/background.h"
#include "ui/views/controls/label.h"
#include "ash/shell.h"
#include "ash/session/session_controller_impl.h"
#include "ash/jemaos_ai/jemaos_ai_bubble.h"

namespace ash {

JemaAssistantView::JemaAssistantView() {
  Shell::Get()->session_controller()->AddObserver(this);
}

JemaAssistantView::~JemaAssistantView() {
  Shell::Get()->session_controller()->RemoveObserver(this);
  if (bubble_) {
    bubble_->GetWidget()->RemoveObserver(this);
    bubble_ = nullptr;
  }
}

void JemaAssistantView::AddObserver(JemaAssistantViewObserver* observer) const {
  observers_.AddObserver(observer);
}
void JemaAssistantView::RemoveObserver(JemaAssistantViewObserver* observer) const {
  observers_.RemoveObserver(observer);
}

void JemaAssistantView::ShowBubble() {
  if (visible_) {
    return;
  }
  Show();
}

void JemaAssistantView::HideBubble() {
  if (!visible_) {
    return;
  }
  Hide();
}

void JemaAssistantView::Show() {
  if (!can_show_) {
    return;
  }
  DCHECK(bubble_);
  if (bubble_->GetWidget() == nullptr) {
    NOTREACHED();
    return;
  }
  current_anchor_point_ = display::Screen::GetScreen()->GetCursorScreenPoint();
  bubble_->SetAnchorRect(gfx::Rect(current_anchor_point_, gfx::Size()));
  bubble_->SetPreferredSize(bubble_->CalculatePreferredSize());
  bubble_->GetWidget()->Show();
  visible_ = true;

  for (auto& observer : observers_) {
    observer.OnBubbleVisibilityChanged(visible_);
    observer.OnBubbleQueryChanged(last_clipboard_item_);
  }
}

void JemaAssistantView::Hide() {
  if (!bubble_) {
    return;
  }
  bubble_->SetPreferredSize(bubble_->CalculatePreferredSize());
  bubble_->GetWidget()->Hide();
  visible_ = false;

  for (auto& observer : observers_) {
    observer.OnBubbleVisibilityChanged(visible_);
  }
}

void JemaAssistantView::CreateAssistantWidget(aura::Window* window) {
  window_ = window;
}

void JemaAssistantView::OnWidgetActivationChanged(views::Widget* widget, bool active) {
  DCHECK(widget == bubble_->GetWidget());
  if (!active) {
    Hide();
  }
}

void JemaAssistantView::OnWidgetClosing(views::Widget* widget) {
  //  not reached if set_close_on_deactivate(false)
  NOTREACHED();
  DCHECK(widget == bubble_->GetWidget());
  bubble_->GetWidget()->RemoveObserver(this);
  bubble_ = nullptr;
  visible_ = false;
}

void JemaAssistantView::OnSessionStateChanged(session_manager::SessionState state) {
  if (state == session_manager::SessionState::ACTIVE) {
    if (!bubble_) {
      bubble_ = new JemaAssistantBubble(window_,
                                        gfx::Rect(display::Screen::GetScreen()->GetCursorScreenPoint(), gfx::Size()));
      bubble_->OpenWebView(this);
      bubble_->GetWidget()->AddObserver(this);
    }
  }
}

void JemaAssistantView::UpdateLastClipboardItem(const ClipboardHistoryItem& item) {
  VLOG(3) << "clipboard update";
  last_clipboard_item_time_ = base::TimeTicks::Now();
  last_clipboard_item_.display_format = static_cast<int>(item.display_format());
  last_clipboard_item_.display_text = item.display_text();
  // copy from subsystem can be slow,  slower than the gap between pressing ctrl+c and c twice
  auto now = base::TimeTicks::Now();
  if (now - last_time_triggered_ < base::Seconds(1) && should_show_bubble_delay_) {
    VLOG(3) << "pressed twice and clipboard updated, show the bubble now";
    ShowBubble();
  }
}

bool JemaAssistantView::CanHandleToggleJemaOSAssistant() {
  if (!can_show_) return false;
  auto now = base::TimeTicks::Now();
  auto delta = now - last_time_triggered_;
  VLOG(3) << "jemaos assistant view accelerator, time delta: " << delta.InMilliseconds();
  bool triggered_before_clipboard_update = last_clipboard_item_time_ < last_time_triggered_;
  last_time_triggered_ = now;
  bool repeated = delta < base::Seconds(1);
  if (repeated && triggered_before_clipboard_update) {
    VLOG(3) << "jemaos assistant accelerator pressed twice, but clipboard not updated, so the bubble should be shown later";
    should_show_bubble_delay_ = true;
    return false;
  }
  should_show_bubble_delay_ = false;
  VLOG(3) << "can handle jemaos assistant accelerator: " << repeated;
  return repeated;
}

void JemaAssistantView::OnBubbleReady() {
  can_show_ = true;
}

void JemaAssistantView::SetBubbleRect(int x, int y, int width, int height) {
  if (bubble_) {
    auto current = bubble_->GetWidget()->GetWindowBoundsInScreen();
    auto newRect = gfx::Rect(
      x > 0 ? x : current.x(),
      y > 0 ? y : current.y(),
      width > 0 ? width : current.width(),
      height > 0 ? height : current.height()
    );
    bubble_->SetPreferredSize(gfx::Size(newRect.width(), newRect.height()));
    bubble_->SetAnchorRect(gfx::Rect(current_anchor_point_, gfx::Size()));
  }
}

} // namespace ash
