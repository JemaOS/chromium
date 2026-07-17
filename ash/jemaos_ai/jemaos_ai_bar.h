// Copyright 2026 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_JEMAOS_AI_JEMAOS_AI_BAR_H_
#define ASH_JEMAOS_AI_JEMAOS_AI_BAR_H_

#include <memory>
#include <string>
#include <vector>

#include "ash/ash_export.h"
#include "ash/jemaos_ai/jemaos_ai_view.h"
#include "ash/public/cpp/session/session_observer.h"
#include "ash/wm/desks/desks_controller.h"
#include "base/memory/raw_ptr.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/aura/window_observer.h"

namespace aura {
class Window;
}

namespace views {
class Widget;
}

namespace ash {

class Desk;
class JemaAssistantBarView;

struct ASH_EXPORT JemaAssistantAgentSummary {
  std::string id;
  std::u16string name;
  SkColor color = SK_ColorGRAY;
  int progress = 0;
  bool active = false;
  bool working = false;
  bool autonomous = false;
};

// Owns the native JemaOS AI environment bar for one display. The bar reserves
// the top edge of the desktop while visible and leaves the original shelf
// untouched.
class ASH_EXPORT JemaAssistantBar : public aura::WindowObserver,
                                    public SessionObserver,
                                    public JemaAssistantViewObserver,
                                    public DesksController::Observer {
 public:
  JemaAssistantBar(aura::Window* root_window,
                   aura::Window* container,
                   JemaAssistantView* assistant_view);

  JemaAssistantBar(const JemaAssistantBar&) = delete;
  JemaAssistantBar& operator=(const JemaAssistantBar&) = delete;

  ~JemaAssistantBar() override;

  bool visible() const { return expanded_ && session_active_; }
  void SetVisible(bool visible);
  void ToggleVisibility();
  void SetAgents(std::vector<JemaAssistantAgentSummary> agents);

  // aura::WindowObserver:
  void OnWindowBoundsChanged(aura::Window* window,
                             const gfx::Rect& old_bounds,
                             const gfx::Rect& new_bounds,
                             ui::PropertyChangeReason reason) override;
  void OnWindowDestroying(aura::Window* window) override;

  // SessionObserver:
  void OnSessionStateChanged(session_manager::SessionState state) override;

  // DesksController::Observer:
  void OnDeskActivationChanged(const Desk* activated,
                               const Desk* deactivated) override;
  void OnDeskRemoved(const Desk* desk) override;

 private:
  void CreateWidgets(aura::Window* container);
  void LayoutWidgets();
  void UpdateVisibility();
  void UpdateWorkArea();
  void ShowAssistant();
  void StartVoiceInput();
  void CreateAgent();
  void ActivateAgent(const std::string& agent_id);
  void ActivateUserDesk();
  Desk* EnsureAgentDesk(const JemaAssistantAgentSummary& agent);

  raw_ptr<aura::Window> root_window_ = nullptr;
  raw_ptr<JemaAssistantView> assistant_view_ = nullptr;
  std::unique_ptr<views::Widget> bar_widget_;
  std::unique_ptr<views::Widget> show_handle_widget_;
  raw_ptr<JemaAssistantBarView> bar_view_ = nullptr;
  std::vector<JemaAssistantAgentSummary> agents_;
  raw_ptr<const Desk> user_desk_ = nullptr;
  bool expanded_ = true;
  bool session_active_ = false;
};

}  // namespace ash

#endif  // ASH_JEMAOS_AI_JEMAOS_AI_BAR_H_
