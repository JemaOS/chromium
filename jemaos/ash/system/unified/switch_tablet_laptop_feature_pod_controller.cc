// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/ash/system/unified/switch_tablet_laptop_feature_pod_controller.h"
#include "ash/resources/vector_icons/vector_icons.h"
#include "ash/system/unified/feature_pod_button.h"
#include "ash/system/unified/feature_tile.h"
#include "ash/shell.h"
#include "ash/strings/grit/ash_strings.h"
#include "ui/base/l10n/l10n_util.h"
#include "ash/wm/tablet_mode/tablet_mode_controller.h"
#include "ash/system/unified/unified_system_tray_controller.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "jemaos/prefs/jemaos_pref_names.h"

namespace ash {

// Constructor for SwitchTabletLabtopFeaturePodController
SwitchTabletLabtopFeaturePodController::SwitchTabletLabtopFeaturePodController(
    UnifiedSystemTrayController* tray_controller)
    : tray_controller_(tray_controller) {}

// Destructor for SwitchTabletLabtopFeaturePodController
SwitchTabletLabtopFeaturePodController::~SwitchTabletLabtopFeaturePodController() = default;

// Creates the feature pod button
FeaturePodButton* SwitchTabletLabtopFeaturePodController::CreateButton() {
  DCHECK(!button_);
  DCHECK(!features::IsQsRevampEnabled());
  button_ = new FeaturePodButton(this, /*is_togglable=*/false);
  UpdateButton();
  return button_;
}

// Creates the feature tile
std::unique_ptr<FeatureTile> SwitchTabletLabtopFeaturePodController::CreateTile(
    bool compact) {
  DCHECK(!tile_);
  DCHECK(features::IsQsRevampEnabled());
  auto tile = std::make_unique<FeatureTile>(
      base::BindRepeating(
        &SwitchTabletLabtopFeaturePodController::OnIconPressed,
        weak_factory_.GetWeakPtr()));
  tile_ = tile.get();
  tile_->SetSubLabelVisibility(false);
  UpdateButton();
  return tile;
}

// Handles the icon press event
void SwitchTabletLabtopFeaturePodController::OnIconPressed() {
  tray_controller_->CloseBubble();
  TabletModeController* controller = Shell::Get()->tablet_mode_controller();
  const bool force_on = controller->IsInJemaForceOnMode();
  const bool force_off = controller->IsInJemaForceOffMode();
  if (!force_on && !force_off && !IsInTabletMode()) {
    // -> default & laptop -> force_on
    controller->SetEnabledByJema(true);
  } else if (IsInTabletMode()) {
    // -> ((default && tablet) || force_on) -> force_off
    controller->SetEnabledByJema(false);
  } else if (force_off) {
    // -> force_off -> default
    controller->SetDefaultBehaviorByJema();
  }
}

// Returns the catalog name for the feature
QsFeatureCatalogName SwitchTabletLabtopFeaturePodController::GetCatalogName() {
  return QsFeatureCatalogName::kSwitchLaptopTablet;
}

// Registers local state preferences
void SwitchTabletLabtopFeaturePodController::RegisterLocalStatePrefs(
    PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(
      jemaos::prefs::kShowSwitchTabletLaptopButton, false);
}

// Updates the visibility of the button or tile
void SwitchTabletLabtopFeaturePodController::UpdateButton() {
  TabletModeController* controller = Shell::Get()->tablet_mode_controller();
  const bool force_on = controller->IsInJemaForceOnMode();
  const bool force_off = controller->IsInJemaForceOffMode();
  std::u16string label_text;
  const gfx::VectorIcon* icon;
  if (!force_on && !force_off && !IsInTabletMode()) {
    icon = &kUnifiedMenuTabletModeIcon;
    label_text = l10n_util::GetStringUTF16(
        IDS_ASH_JEMAOS_UNIFIED_MENU_LABEL_SWITCH_TO_TABLET_MODE);
  } else if (IsInTabletMode()) {
    icon = &kUnifiedMenuDesktopModeIcon;
    label_text = l10n_util::GetStringUTF16(
        IDS_ASH_JEMAOS_UNIFIED_MENU_LABEL_SWITCH_TO_LAPTOP_MODE);
  } else if (force_off) {
    icon = &kUnifiedMenuDefaultModeIcon;
    label_text = l10n_util::GetStringUTF16(
        IDS_ASH_JEMAOS_UNIFIED_MENU_LABEL_SWITCH_TO_DEFAULT_MODE);
  }
  PrefService* prefs = Shell::Get()->local_state();
  bool visible = prefs->GetBoolean(
      jemaos::prefs::kShowSwitchTabletLaptopButton);
  const bool is_qs_revamp_enabled = features::IsQsRevampEnabled();
  if (is_qs_revamp_enabled) {
    if (!tile_) {
      return;
    }
    tile_->SetVectorIcon(*icon);
    tile_->SetLabel(label_text);
    tile_->SetTooltipText(label_text);
    tile_->SetVisible(visible);
  } else {
    if (!button_) {
      return;
    }
    button_->SetVectorIcon(*icon);
    button_->SetLabel(label_text);
    button_->SetIconAndLabelTooltips(label_text);
    button_->SetVisible(visible);
  }
}

// Checks if the system is in tablet mode
bool SwitchTabletLabtopFeaturePodController::IsInTabletMode() {
  TabletModeController* controller = Shell::Get()->tablet_mode_controller();
  const bool in_dev_mode = controller->IsInDevTabletMode();
  const bool force_on = controller->IsInJemaForceOnMode();
  const bool physical_on = controller->is_in_tablet_physical_state();
  return in_dev_mode || force_on || physical_on;
}

} // namespace ash