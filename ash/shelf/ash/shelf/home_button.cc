// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/shelf/home_button.h"

#include <math.h>  // std::ceil

#include <memory>

#include "ash/app_list/app_list_controller_impl.h"
#include "ash/app_list/app_list_model_provider.h"
#include "ash/app_list/model/app_list_item.h"
#include "ash/app_list/model/app_list_model.h"
#include "ash/app_list/quick_app_access_model.h"
#include "ash/ash_element_identifiers.h"
#include "ash/constants/ash_features.h"
#include "ash/constants/ash_switches.h"
#include "ash/public/cpp/ash_typography.h"
#include "ash/public/cpp/shelf_config.h"
#include "ash/public/cpp/shelf_types.h"
#include "ash/resources/vector_icons/vector_icons.h"
#include "ash/shelf/shelf.h"
#include "ash/shelf/shelf_control_button.h"
#include "ash/shelf/shelf_focus_cycler.h"
#include "ash/shelf/shelf_navigation_widget.h"
#include "ash/shelf/shelf_view.h"
#include "ash/shell.h"
#include "ash/strings/grit/ash_strings.h"
#include "ash/style/ash_color_id.h"
#include "ash/style/ash_color_provider.h"
#include "ash/style/typography.h"
#include "ash/user_education/user_education_class_properties.h"
#include "base/check_op.h"
#include "base/i18n/rtl.h"
#include "base/memory/raw_ptr.h"
#include "base/metrics/field_trial_params.h"
#include "base/metrics/user_metrics.h"
#include "base/metrics/user_metrics_action.h"
#include "base/time/time.h"
#include "chromeos/constants/chromeos_features.h"
#include "chromeos/strings/grit/chromeos_strings.h"
#include "ui/aura/window.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/chromeos/styles/cros_tokens_color_mappings.h"
#include "ui/display/display.h"
#include "ui/display/screen.h"
#include "ui/events/ash/keyboard_capability.h"
#include "ui/events/devices/device_data_manager.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/geometry/transform_util.h"
#include "ui/gfx/scoped_canvas.h"
#include "ui/views/animation/animation_builder.h"
#include "ui/views/animation/flood_fill_ink_drop_ripple.h"
#include "ui/views/animation/ink_drop.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/button/button_controller.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/highlight_path_generator.h"
#include "ui/views/controls/label.h"
#include "ui/views/highlight_border.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/style/typography.h"
#include "ui/views/view.h"
#include "ui/views/view_class_properties.h"
#include "build/branding_buildflags.h"

namespace ash {
namespace {

// The space between the home button and quick app.
constexpr int kQuickAppStartMargin = 8;

constexpr uint8_t kAssistantVisibleAlpha = 255;    // 100% alpha
constexpr uint8_t kAssistantInvisibleAlpha = 138;  // 54% alpha

// Nudge animation constants

// The offsets that the home button moves up/down from the original home button
// position at each stage of nudge animation.
constexpr int kAnimationBounceUpOffset = 12;
constexpr int kAnimationBounceDownOffset = 3;

// Constants used on `nudge_ripple_layer_` animation.
constexpr base::TimeDelta kHomeButtonAnimationDuration =
    base::Milliseconds(250);
constexpr base::TimeDelta kRippleAnimationDuration = base::Milliseconds(2000);

// Constants used on `nudge_label_` animation.
//
// The duration of the showing/hiding animation for nudge label.
constexpr base::TimeDelta kNudgeLabelTransitionOnDuration =
    base::Milliseconds(300);
constexpr base::TimeDelta kNudgeLabelTransitionOffDuration =
    base::Milliseconds(500);

// The duration of the fade out animation that animates `nudge_label_` when
// users click on the home button while `nudge_label_` is showing.
constexpr base::TimeDelta kNudgeLabelFadeOutDuration = base::Milliseconds(100);

// The duration that the nudge label is shown.
constexpr base::TimeDelta kNudgeLabelShowingDuration = base::Seconds(6);

// The minimum space we want to keep between the `nudge_label_` and the first
// app in hotseat. Used to determine if `nudge_label_` should be shown.
constexpr int kMinSpaceBetweenNudgeLabelAndHotseat = 24;

// The durations and delay used for animating in the `quick_app_button_` and
// `expandable_container_`.
constexpr base::TimeDelta kQuickAppSlideSlideInDuration =
    base::Milliseconds(200);
constexpr base::TimeDelta kQuickAppButtonFadeInDelay = base::Milliseconds(50);
constexpr base::TimeDelta kQuickAppButtonFadeInDuration =
    base::Milliseconds(50);
constexpr base::TimeDelta kQuickAppContainerFadeInDuration =
    base::Milliseconds(100);

// The durations used for animating out the `quick_app_button_` and
// `expandable_container_`.
constexpr base::TimeDelta kQuickAppSlideOutDuration = base::Milliseconds(200);
constexpr base::TimeDelta kQuickAppFadeOutDuration = base::Milliseconds(100);

}  // namespace

class HomeButton::ButtonImageView : public views::View {
  METADATA_HEADER(ButtonImageView, views::View)

 public:
  explicit ButtonImageView(HomeButtonController* button_controller)
      : button_controller_(button_controller) {
    SetCanProcessEventsWithinSubtree(false);
    SetPaintToLayer();
    layer()->SetFillsBoundsOpaquely(false);
    UpdateBackground();
    UpdateIconImageModel();
  }

  ButtonImageView(const ButtonImageView&) = delete;
  ButtonImageView& operator=(const ButtonImageView&) = delete;

  ~ButtonImageView() override = default;

  // views::View:
  void OnPaint(gfx::Canvas* canvas) override {
    views::View::OnPaint(canvas);

    if (!image_.isNull()) {
      canvas->DrawImageInt(image_, (width() - image_.width()) / 2,
                           (height() - image_.height()) / 2, cc::PaintFlags());
      return;
    }

    // --- MODIFICATION : DESSIN DES 4 PÉTALES ALIGNÉS (LOOK WINDOWS) ---
    gfx::ScopedCanvas scoped_canvas(canvas);
    const float dsf = canvas->UndoDeviceScaleFactor();
    gfx::PointF center(gfx::Rect(size()).CenterPoint());
    center.Scale(dsf);

    cc::PaintFlags fg_flags;
    fg_flags.setAntiAlias(true);
    fg_flags.setStyle(cc::PaintFlags::kFill_Style);
    
    // Utilisation de la couleur système dynamique (monochrome)
    fg_flags.setColor(GetColorProvider()->GetColor(GetIconColorId()));

    // Rayon des cercles et décalage pour une grille droite
    const float petal_radius = 3.8f * dsf; 
    const float offset = 4.0f * dsf;

    // Dessin manuel des 4 pétales alignés
    canvas->DrawCircle(gfx::PointF(center.x() - offset, center.y() - offset), petal_radius, fg_flags);
    canvas->DrawCircle(gfx::PointF(center.x() + offset, center.y() - offset), petal_radius, fg_flags);
    canvas->DrawCircle(gfx::PointF(center.x() - offset, center.y() + offset), petal_radius, fg_flags);
    canvas->DrawCircle(gfx::PointF(center.x() + offset, center.y() + offset), petal_radius, fg_flags);
    // -----------------------------------------------------------------
  }

  void OnThemeChanged() override {
    views::View::OnThemeChanged();
    if (image_model_) {
      image_ = image_model_->Rasterize(GetColorProvider());
    }
    SchedulePaint();
  }

  // Updates the button image view for the new shelf config.
  void UpdateForShelfConfigChange() {
    layer()->SetBackgroundBlur(
        ShelfConfig::Get()->GetShelfControlButtonBlurRadius());
    layer()->SetBackdropFilterQuality(ColorProvider::kBackgroundBlurQuality);
    UpdateBackground();
    UpdateIconImageModel();
  }

  void SetToggled(bool toggled) {
    if (toggled_ == toggled) {
      return;
    }

    toggled_ = toggled;
    UpdateBackground();
    UpdateIconImageModel();
    SchedulePaint();
  }

 private:
  // Updates the view background to match the current shelf config.
  void UpdateBackground() {
    auto* const shelf_config = ShelfConfig::Get();

    if (shelf_config->in_tablet_mode() && shelf_config->is_in_app()) {
      SetBackground(nullptr);
      SetBorder(nullptr);
      return;
    }

    SetBackground(views::CreateThemedRoundedRectBackground(
        GetBackgroundColorId(), shelf_config->control_border_radius()));

    if (shelf_config->in_tablet_mode() && !shelf_config->is_in_app()) {
      SetBorder(std::make_unique<views::HighlightBorder>(
          shelf_config->control_border_radius(),
          views::HighlightBorder::Type::kHighlightBorderOnShadow));
    } else {
      SetBorder(nullptr);
    }
  }

  ui::ColorId GetIconColorId() {
    return toggled_ && !ShelfConfig::Get()->in_tablet_mode()
               ? cros_tokens::kCrosSysSystemOnPrimaryContainer
               : cros_tokens::kCrosSysOnSurface;
  }

  ui::ColorId GetBackgroundColorId() {
    if (ShelfConfig::Get()->in_tablet_mode()) {
      return cros_tokens::kCrosSysSystemBaseElevated;
    }

    return toggled_ ? cros_tokens::kCrosSysSystemPrimaryContainer
                    : cros_tokens::kCrosSysSystemOnBase;
  }

  void UpdateIconImageModel() {
    // --- MODIFICATION : ON FORCE À NULL POUR UTILISER ONPAINT ---
    image_model_ = std::nullopt;
    image_ = gfx::ImageSkia();
  }

  const raw_ptr<HomeButtonController> button_controller_;

  gfx::ImageSkia image_;
  std::optional<ui::ImageModel> image_model_;

  bool toggled_ = false;
};

BEGIN_METADATA(HomeButton, ButtonImageView)
END_METADATA

// HomeButton::ScopedNoClipRect ------------------------------------------------

HomeButton::ScopedNoClipRect::ScopedNoClipRect(
    ShelfNavigationWidget* shelf_navigation_widget)
    : shelf_navigation_widget_(shelf_navigation_widget),
      clip_rect_(shelf_navigation_widget_->GetLayer()->clip_rect()) {
  shelf_navigation_widget_->GetLayer()->SetClipRect(gfx::Rect());
}

HomeButton::ScopedNoClipRect::~ScopedNoClipRect() {
  if (shelf_navigation_widget_->GetLayer())
    shelf_navigation_widget_->GetLayer()->SetClipRect(clip_rect_);
}

// HomeButton Implementation --------------------------------------------------

HomeButton::HomeButton(Shelf* shelf)
    : ShelfControlButton(shelf, this),
      shelf_(shelf),
      controller_(this) {
  GetViewAccessibility().SetName(
      l10n_util::GetStringUTF16(IDS_ASH_SHELF_APP_LIST_LAUNCHER_TITLE));
  button_controller()->set_notify_action(
      views::ButtonController::NotifyAction::kOnPress);

  views::InkDrop::Get(this)->SetMode(views::InkDropHost::InkDropMode::ON);

  SetEventTargeter(std::make_unique<views::ViewTargeter>(this));
  layer()->SetName("shelf/Homebutton");

  button_image_view_ =
      AddChildViewAt(std::make_unique<ButtonImageView>(&controller_), 0);

  if (features::IsHomeButtonWithTextEnabled()) {
    CreateNudgeLabel();
    expandable_container_->SetVisible(true);
    shelf_->shelf_layout_manager()->LayoutShelf(false);
  }

  if (features::IsHomeButtonQuickAppAccessEnabled() &&
      !features::IsHomeButtonWithTextEnabled()) {
    shell_observation_.Observe(Shell::Get());
    app_list_model_observation_.Observe(AppListModelProvider::Get());
    quick_app_model_observation_.Observe(
        AppListModelProvider::Get()->quick_app_access_model());
  }

  if (features::IsUserEducationEnabled()) {
    SetProperty(kHelpBubbleContextKey, HelpBubbleContext::kAsh);
  }
  SetProperty(views::kElementIdentifierKey, kHomeButtonElementId);

  ui::DeviceDataManager::GetInstance()->AddObserver(this);
  ShelfConfig::Get()->AddObserver(this);
}

HomeButton::~HomeButton() {
  ui::DeviceDataManager::GetInstance()->RemoveObserver(this);
  ShelfConfig::Get()->RemoveObserver(this);
}

gfx::Size HomeButton::CalculatePreferredSize(
    const views::SizeBounds& available_size) const {
  const gfx::Size control_button_size =
      ShelfControlButton::CalculatePreferredSize(available_size);

  if (expandable_container_ && expandable_container_->GetVisible()) {
    const gfx::Size container_size = expandable_container_->GetPreferredSize();
    return gfx::Size(
        std::max(control_button_size.width(), container_size.width()),
        std::max(control_button_size.height(), container_size.height()));
  }

  return control_button_size;
}

void HomeButton::Layout(PassKey) {
  LayoutSuperclass<ShelfControlButton>(this);

  button_image_view_->SetBoundsRect(
      gfx::Rect(ShelfControlButton::CalculatePreferredSize({})));

  if (expandable_container_) {
    if (shelf_->IsHorizontalAlignment()) {
      expandable_container_->SetSize(gfx::Size(
          expandable_container_->GetPreferredSize().width(), height()));
    } else {
      expandable_container_->SetSize(gfx::Size(
          width(), expandable_container_->GetPreferredSize().height()));
    }

    if (quick_app_button_) {
      if (shelf_->IsHorizontalAlignment()) {
        expandable_container_->SetBorder(
            views::CreateEmptyBorder(gfx::Insets::TLBR(
                0,
                ShelfControlButton::CalculatePreferredSize({}).width() +
                    kQuickAppStartMargin,
                0, 0)));
      } else {
        expandable_container_->SetBorder(
            views::CreateEmptyBorder(gfx::Insets::TLBR(
                ShelfControlButton::CalculatePreferredSize({}).height() +
                    kQuickAppStartMargin,
                0, 0, 0)));
      }
      expandable_container_->layer()->SetClipRect(
          gfx::Rect(expandable_container_->size()));
    }
  }
}

void HomeButton::OnGestureEvent(ui::GestureEvent* event) {
  if (!controller_.MaybeHandleGestureEvent(event))
    Button::OnGestureEvent(event);
}

std::u16string HomeButton::GetTooltipText(const gfx::Point& p) const {
  return IsShowingAppList() ? std::u16string()
                            : GetViewAccessibility().GetCachedName();
}

void HomeButton::OnShelfButtonAboutToRequestFocusFromTabTraversal(
    ShelfButton* button,
    bool reverse) {
  DCHECK_EQ(button, this);
  const bool quick_app_focused =
      quick_app_button_ &&
      (GetFocusManager()->GetFocusedView() == quick_app_button_);

  if (GetFocusManager()->GetFocusedView() == this ||
      (quick_app_focused && !reverse) ||
      (reverse && shelf()->navigation_widget()->GetBackButton())) {
    shelf()->shelf_focus_cycler()->FocusOut(reverse,
                                            SourceView::kShelfNavigationView);
  }
}

void HomeButton::ButtonPressed(views::Button* sender,
                               const ui::Event& event,
                               views::InkDrop* ink_drop) {
  if (display::Screen::GetScreen()->InTabletMode()) {
    base::RecordAction(
        base::UserMetricsAction("AppList_HomeButtonPressedTablet"));
  } else {
    base::RecordAction(
        base::UserMetricsAction("AppList_HomeButtonPressedClamshell"));
  }

  Shell::Get()->app_list_controller()->ToggleAppList(
      GetDisplayId(), AppListShowSource::kShelfButton, event.time_stamp());

  if (expandable_container_ && !quick_app_button_) {
    if (features::IsHomeButtonWithTextEnabled())
      return;

    if (!expandable_container_->GetVisible()) {
      RemoveNudgeLabel();
      return;
    }

    if (label_nudge_timer_.IsRunning())
      label_nudge_timer_.Stop();
    AnimateNudgeLabelFadeOut();
  }
}

void HomeButton::OnShelfConfigUpdated() {
  button_image_view_->UpdateForShelfConfigChange();
}

void HomeButton::OnAssistantAvailabilityChanged() {
  if (button_image_view_) {
    button_image_view_->SchedulePaint();
  }
}

bool HomeButton::IsShowingAppList() const {
  auto* controller = Shell::Get()->app_list_controller();
  return controller && controller->GetTargetVisibility(GetDisplayId());
}

void HomeButton::HandleLocaleChange() {
  GetViewAccessibility().SetName(
      l10n_util::GetStringUTF16(IDS_ASH_SHELF_APP_LIST_LAUNCHER_TITLE));
  TooltipTextChanged();
  SetBoundsRect(gfx::Rect());
}

int64_t HomeButton::GetDisplayId() const {
  aura::Window* window = GetWidget()->GetNativeWindow();
  return display::Screen::GetScreen()->GetDisplayNearestWindow(window).id();
}

std::unique_ptr<HomeButton::ScopedNoClipRect>
HomeButton::CreateScopedNoClipRect() {
  return std::make_unique<HomeButton::ScopedNoClipRect>(
      shelf()->navigation_widget());
}

bool HomeButton::CanShowNudgeLabel() const {
  if (!shelf_->IsHorizontalAlignment())
    return false;

  if (quick_app_button_) {
    return false;
  }

  ShelfView* shelf_view = shelf_->hotseat_widget()->GetShelfView();
  int view_size = shelf_view->view_model()->view_size();
  if (view_size == 0)
    return true;

  DCHECK(nudge_label_);

  gfx::Rect first_app_bounds =
      shelf_view->view_model()->view_at(0)->GetMirroredBounds();
  first_app_bounds = shelf_view->ConvertRectToWidget(first_app_bounds);
  aura::Window* shelf_native_window =
      shelf_view->GetWidget()->GetNativeWindow();
  aura::Window::ConvertRectToTarget(shelf_native_window,
                                    shelf_native_window->GetRootWindow(),
                                    &first_app_bounds);

  gfx::Rect label_rect =
      ConvertRectToWidget(expandable_container_->GetMirroredBounds());
  aura::Window* native_window = GetWidget()->GetNativeWindow();
  DCHECK_EQ(shelf_native_window->GetRootWindow(),
            native_window->GetRootWindow());
  aura::Window::ConvertRectToTarget(
      native_window, native_window->GetRootWindow(), &label_rect);

  int space = label_rect.ManhattanInternalDistance(first_app_bounds);
  return space >= kMinSpaceBetweenNudgeLabelAndHotseat;
}

void HomeButton::StartNudgeAnimation() {
  if (features::IsHomeButtonWithTextEnabled())
    return;

  nudge_ripple_layer_.ReleaseLayer();
  if (nudge_label_)
    nudge_label_->layer()->GetAnimator()->AbortAllAnimations();
  if (expandable_container_) {
    expandable_container_->layer()->GetAnimator()->AbortAllAnimations();
  }

  if (!nudge_label_)
    CreateNudgeLabel();

  const bool can_show_nudge_label = CanShowNudgeLabel();

  views::AnimationBuilder builder;
  builder
      .SetPreemptionStrategy(
          ui::LayerAnimator::IMMEDIATELY_ANIMATE_TO_NEW_TARGET)
      .OnStarted(base::BindOnce(&HomeButton::OnNudgeAnimationStarted,
                                weak_ptr_factory_.GetWeakPtr()))
      .OnEnded(base::BindOnce(can_show_nudge_label
                                  ? &HomeButton::OnLabelSlideInAnimationEnded
                                  : &HomeButton::OnNudgeAnimationEnded,
                              weak_ptr_factory_.GetWeakPtr()))
      .OnAborted(base::BindOnce(&HomeButton::OnNudgeAnimationEnded,
                                weak_ptr_factory_.GetWeakPtr()))
      .Once();

  if (can_show_nudge_label) {
    AnimateNudgeLabelSlideIn(builder);
  } else {
    AnimateNudgeBounce(builder);
  }

  scoped_no_clip_rect_ = CreateScopedNoClipRect();
  AnimateNudgeRipple(builder);
}

void HomeButton::SetToggled(bool toggled) {
  button_image_view_->SetToggled(toggled);
}

void HomeButton::AddNudgeAnimationObserverForTest(
    NudgeAnimationObserver* observer) {
  observers_.AddObserver(observer);
}
void HomeButton::RemoveNudgeAnimationObserverForTest(
    NudgeAnimationObserver* observer) {
  observers_.RemoveObserver(observer);
}

void HomeButton::OnThemeChanged() {
  ShelfControlButton::OnThemeChanged();

  if (ripple_layer_delegate_) {
    ripple_layer_delegate_->set_color(GetColorProvider()->GetColor(
        cros_tokens::kCrosSysRippleNeutralOnSubtle));
  }
  if (expandable_container_) {
    expandable_container_->layer()->SetColor(
        GetColorProvider()->GetColor(cros_tokens::kCrosSysSystemOnBase));
  }
}

void HomeButton::CreateExpandableContainer() {
  const int home_button_width =
      ShelfControlButton::CalculatePreferredSize({}).width();

  expandable_container_ = AddChildViewAt(std::make_unique<views::View>(), 0);
  expandable_container_->SetLayoutManager(
      std::make_unique<views::FillLayout>());
  expandable_container_->SetPaintToLayer(ui::LAYER_SOLID_COLOR);
  expandable_container_->layer()->SetMasksToBounds(true);
  if (GetColorProvider()) {
    expandable_container_->layer()->SetColor(
        GetColorProvider()->GetColor(cros_tokens::kCrosSysSystemOnBase));
  }
  expandable_container_->layer()->SetRoundedCornerRadius(
      gfx::RoundedCornersF(home_button_width / 2.f));
  expandable_container_->layer()->SetName("NudgeLabelContainer");
}

void HomeButton::CreateNudgeLabel() {
  DCHECK(!expandable_container_);

  CreateExpandableContainer();
  expandable_container_->SetBorder(views::CreateEmptyBorder(gfx::Insets::TLBR(
      0, ShelfControlButton::CalculatePreferredSize({}).width(), 0, 16)));

  auto* label_mask =
      expandable_container_->AddChildView(std::make_unique<views::View>());
  label_mask->SetLayoutManager(std::make_unique<views::FillLayout>());
  label_mask->SetBorder(
      views::CreateEmptyBorder(gfx::Insets::TLBR(0, 12, 0, 0)));
  label_mask->SetPaintToLayer(ui::LAYER_NOT_DRAWN);
  label_mask->layer()->SetMasksToBounds(true);
  label_mask->layer()->SetName("NudgeLabelMask");

  nudge_label_ = label_mask->AddChildView(std::make_unique<views::Label>(
      l10n_util::GetStringUTF16(IDS_SHELF_LAUNCHER_NUDGE_TEXT)));
  nudge_label_->SetAutoColorReadabilityEnabled(false);
  nudge_label_->SetPaintToLayer();
  nudge_label_->layer()->SetFillsBoundsOpaquely(false);
  nudge_label_->SetTextContext(CONTEXT_LAUNCHER_NUDGE_LABEL);
  nudge_label_->SetTextStyle(views::style::STYLE_EMPHASIZED);
  nudge_label_->SetEnabledColorId(cros_tokens::kCrosSysOnSurface);
  TypographyProvider::Get()->StyleLabel(TypographyToken::kCrosButton2,
                                        *nudge_label_);
  expandable_container_->SetVisible(false);
  DeprecatedLayoutImmediately();
}

void HomeButton::CreateQuickAppButton() {
  CreateExpandableContainer();
  if (shelf_->IsHorizontalAlignment()) {
    expandable_container_->SetBorder(views::CreateEmptyBorder(gfx::Insets::TLBR(
        0,
        ShelfControlButton::CalculatePreferredSize({}).width() +
            kQuickAppStartMargin,
        0, 0)));
  } else {
    expandable_container_->SetBorder(views::CreateEmptyBorder(gfx::Insets::TLBR(
        ShelfControlButton::CalculatePreferredSize({}).height() +
            kQuickAppStartMargin,
        0, 0, 0)));
  }

  quick_app_button_ = expandable_container_->AddChildView(
      std::make_unique<views::ImageButton>(base::BindRepeating(
          &HomeButton::QuickAppButtonPressed, base::Unretained(this))));
  quick_app_button_->GetViewAccessibility().SetName(
      AppListModelProvider::Get()->quick_app_access_model()->GetAppName());

  const int control_size =
      ShelfControlButton::CalculatePreferredSize({}).width();

  const gfx::Size preferred_size = gfx::Size(control_size, control_size);

  quick_app_button_->SetPaintToLayer();
  quick_app_button_->layer()->SetFillsBoundsOpaquely(false);
  quick_app_button_->SetImageModel(
      views::Button::STATE_NORMAL,
      ui::ImageModel::FromImageSkia(
          AppListModelProvider::Get()->quick_app_access_model()->GetAppIcon(
              preferred_size)));
  views::HighlightPathGenerator::Install(
      quick_app_button_,
      std::make_unique<views::RoundRectHighlightPathGenerator>(
          gfx::Insets(views::FocusRing::kDefaultHaloThickness / 2),
          ShelfConfig::Get()->control_border_radius()));
  views::FocusRing::Get(quick_app_button_)
      ->SetColorId(cros_tokens::kCrosSysFocusRing);
  quick_app_button_->SetSize(preferred_size);

  shelf_->shelf_layout_manager()->LayoutShelf(false);
}

void HomeButton::QuickAppButtonPressed() {
  ash::Shell::Get()->app_list_controller()->ActivateItem(
      AppListModelProvider::Get()->quick_app_access_model()->quick_app_id(),
      /*event_flags=*/0, ash::AppListLaunchedFrom::kLaunchedFromQuickAppAccess,
      /*is_above_the_fold=*/false);
  AppListModelProvider::Get()->quick_app_access_model()->SetQuickAppActivated();
}

void HomeButton::AnimateNudgeRipple(views::AnimationBuilder& builder) {
  nudge_ripple_layer_.Reset(std::make_unique<ui::Layer>());
  ui::Layer* ripple_layer = nudge_ripple_layer_.layer();

  float ripple_diameter =
      ShelfControlButton::CalculatePreferredSize({}).width();
  auto* color_provider = GetColorProvider();
  DCHECK(color_provider);
  ripple_layer_delegate_ = std::make_unique<views::CircleLayerDelegate>(
      color_provider->GetColor(cros_tokens::kCrosSysRippleNeutralOnSubtle),
      /*radius=*/ripple_diameter / 2);

  ripple_layer->SetBounds(
      gfx::Rect(layer()->parent()->bounds().x() + layer()->bounds().x(),
                layer()->parent()->bounds().y() + layer()->bounds().y(),
                ripple_diameter, ripple_diameter));

  ripple_layer->set_delegate(ripple_layer_delegate_.get());
  ripple_layer->SetMasksToBounds(true);
  ripple_layer->SetFillsBoundsOpaquely(false);

  ui::Layer* shelf_container_layer = GetWidget()->GetLayer()->parent();
  shelf_container_layer->Add(ripple_layer);
  shelf_container_layer->StackBelow(ripple_layer, layer()->parent());

  const gfx::PointF ripple_center =
      gfx::RectF(gfx::SizeF(ripple_layer->size())).CenterPoint();

  gfx::Transform initial_disc_scale;
  initial_disc_scale.Scale(0.1f, 0.1f);
  gfx::Transform initial_state =
      gfx::TransformAboutPivot(ripple_center, initial_disc_scale);

  gfx::Transform final_disc_scale;
  final_disc_scale.Scale(3.0f, 3.0f);
  gfx::Transform scale_about_pivot =
      gfx::TransformAboutPivot(ripple_center, final_disc_scale);

  builder.GetCurrentSequence()
      .At(base::TimeDelta())
      .SetDuration(base::TimeDelta())
      .SetTransform(ripple_layer, initial_state)
      .SetOpacity(ripple_layer, 0.5f)
      .Then()
      .SetDuration(kRippleAnimationDuration)
      .SetTransform(ripple_layer, scale_about_pivot,
                    gfx::Tween::ACCEL_0_40_DECEL_100)
      .SetOpacity(ripple_layer, 0.0f, gfx::Tween::ACCEL_0_80_DECEL_80);
}

void HomeButton::AnimateNudgeBounce(views::AnimationBuilder& builder) {
  gfx::PointF bounce_up_point = shelf()->SelectValueForShelfAlignment(
      gfx::PointF(0, -kAnimationBounceUpOffset),
      gfx::PointF(kAnimationBounceUpOffset, 0),
      gfx::PointF(-kAnimationBounceUpOffset, 0));
  gfx::PointF bounce_down_point = shelf()->SelectValueForShelfAlignment(
      gfx::PointF(0, kAnimationBounceDownOffset),
      gfx::PointF(-kAnimationBounceDownOffset, 0),
      gfx::PointF(kAnimationBounceDownOffset, 0));

  gfx::Transform move_up;
  move_up.Translate(bounce_up_point.x(), bounce_up_point.y());
  gfx::Transform move_down;
  move_down.Translate(bounce_down_point.x(), bounce_down_point.y());

  ui::Layer* widget_layer = GetWidget()->GetLayer();

  builder.GetCurrentSequence()
      .At(base::TimeDelta())
      .SetDuration(kHomeButtonAnimationDuration)
      .SetTransform(widget_layer, move_up, gfx::Tween::FAST_OUT_SLOW_IN_3)
      .Then()
      .SetDuration(kHomeButtonAnimationDuration)
      .SetTransform(widget_layer, move_down, gfx::Tween::ACCEL_80_DECEL_20)
      .Then()
      .SetDuration(kHomeButtonAnimationDuration)
      .SetTransform(widget_layer, gfx::Transform(),
                    gfx::Tween::FAST_OUT_SLOW_IN_3);
}

void HomeButton::AnimateNudgeLabelSlideIn(views::AnimationBuilder& builder) {
  DCHECK(expandable_container_ && nudge_label_);
  expandable_container_->SetVisible(true);
  shelf_->shelf_layout_manager()->LayoutShelf(false);

  const gfx::Rect initial_container_clip_rect =
      GetExpandableContainerClipRectToHomeButton();
  const gfx::Transform initial_transform =
      GetTransformForContainerChildBehindHomeButton();

  const gfx::Rect container_target_clip_rect =
      gfx::Rect(expandable_container_->size());

  builder.GetCurrentSequence()
      .At(base::TimeDelta())
      .SetDuration(base::TimeDelta())
      .SetTransform(nudge_label_->layer(), initial_transform)
      .SetClipRect(expandable_container_->layer(), initial_container_clip_rect)
      .SetOpacity(expandable_container_->layer(), 0)
      .Then()
      .SetDuration(kNudgeLabelTransitionOnDuration)
      .SetTransform(nudge_label_->layer(), gfx::Transform(),
                    gfx::Tween::ACCEL_5_70_DECEL_90)
      .SetClipRect(expandable_container_->layer(), container_target_clip_rect,
                    gfx::Tween::ACCEL_5_70_DECEL_90)
      .SetOpacity(expandable_container_->layer(), 1,
                  gfx::Tween::ACCEL_5_70_DECEL_90);
}

void HomeButton::AnimateNudgeLabelSlideOut() {
  const gfx::Transform target_transform =
      GetTransformForContainerChildBehindHomeButton();
  const gfx::Rect container_target_clip_rect =
      GetExpandableContainerClipRectToHomeButton();

  views::AnimationBuilder()
      .SetPreemptionStrategy(
          ui::LayerAnimator::IMMEDIATELY_ANIMATE_TO_NEW_TARGET)
      .OnEnded(base::BindOnce(&HomeButton::OnNudgeAnimationEnded,
                              weak_ptr_factory_.GetWeakPtr()))
      .OnAborted(base::BindOnce(&HomeButton::OnNudgeAnimationEnded,
                                weak_ptr_factory_.GetWeakPtr()))
      .Once()
      .SetDuration(kNudgeLabelTransitionOffDuration)
      .SetTransform(nudge_label_->layer(), target_transform,
                    gfx::Tween::ACCEL_40_DECEL_100_3)
      .SetClipRect(expandable_container_->layer(), container_target_clip_rect,
                    gfx::Tween::ACCEL_40_DECEL_100_3)
      .SetOpacity(expandable_container_->layer(), 0,
                  gfx::Tween::ACCEL_40_DECEL_100_3);
}

void HomeButton::AnimateNudgeLabelFadeOut() {
  views::AnimationBuilder()
      .SetPreemptionStrategy(
          ui::LayerAnimator::IMMEDIATELY_ANIMATE_TO_NEW_TARGET)
      .OnEnded(base::BindOnce(&HomeButton::OnLabelFadeOutAnimationEnded,
                              weak_ptr_factory_.GetWeakPtr()))
      .OnAborted(base::BindOnce(&HomeButton::OnLabelFadeOutAnimationEnded,
                                weak_ptr_factory_.GetWeakPtr()))
      .Once()
      .SetDuration(kNudgeLabelFadeOutDuration)
      .SetOpacity(expandable_container_->layer(), 0, gfx::Tween::LINEAR);
}

void HomeButton::OnNudgeAnimationStarted() {
  for (auto& observer : observers_)
    observer.NudgeAnimationStarted(this);
}

void HomeButton::OnNudgeAnimationEnded() {
  nudge_ripple_layer_.ReleaseLayer();
  ripple_layer_delegate_.reset();

  if (expandable_container_) {
    expandable_container_->SetVisible(false);
    shelf_->shelf_layout_manager()->LayoutShelf(false);
  }

  scoped_no_clip_rect_.reset();

  for (auto& observer : observers_)
    observer.NudgeAnimationEnded(this);
}

void HomeButton::OnLabelSlideInAnimationEnded() {
  for (auto& observer : observers_)
    observer.NudgeLabelShown(this);

  label_nudge_timer_.Start(
      FROM_HERE, kNudgeLabelShowingDuration,
      base::BindOnce(&HomeButton::AnimateNudgeLabelSlideOut,
                      base::Unretained(this)));
}

void HomeButton::OnLabelFadeOutAnimationEnded() {
  OnNudgeAnimationEnded();
  RemoveNudgeLabel();
}

void HomeButton::RemoveNudgeLabel() {
  nudge_label_ = nullptr;
  RemoveChildViewT(expandable_container_.ExtractAsDangling());
}

void HomeButton::RemoveQuickAppButton() {
  quick_app_button_ = nullptr;
  RemoveChildViewT(expandable_container_.ExtractAsDangling());
}

bool HomeButton::DoesIntersectRect(const views::View* target,
                                   const gfx::Rect& rect) const {
  DCHECK_EQ(target, this);
  gfx::Rect button_bounds = target->GetLocalBounds();

  if (expandable_container_ && expandable_container_->GetVisible()) {
    button_bounds = expandable_container_->layer()->bounds();
  }

  button_bounds.Inset(
      gfx::Insets::VH(-ShelfConfig::Get()->control_button_edge_spacing(
                          !shelf()->IsHorizontalAlignment()),
                      -ShelfConfig::Get()->control_button_edge_spacing(
                          shelf()->IsHorizontalAlignment())));
  return button_bounds.Intersects(rect);
}

void HomeButton::OnInputDeviceConfigurationChanged(uint8_t input_device_types) {
  if (input_device_types & InputDeviceEventObserver::kKeyboard) {
    button_image_view_->UpdateForShelfConfigChange();
  }
}

void HomeButton::OnDeviceListsComplete() {
  button_image_view_->UpdateForShelfConfigChange();
}

void HomeButton::OnShellDestroying() {
  shell_observation_.Reset();
  app_list_model_observation_.Reset();
  quick_app_model_observation_.Reset();
}

void HomeButton::OnActiveAppListModelsChanged(AppListModel* model,
                                              SearchModel* search_model) {
  QuickAppAccessModel* quick_model =
      AppListModelProvider::Get()->quick_app_access_model();
  quick_app_model_observation_.Reset();
  quick_app_model_observation_.Observe(quick_model);

  OnQuickAppShouldShowChanged(quick_model->quick_app_should_show_state());
}

void HomeButton::OnQuickAppShouldShowChanged(bool show_quick_app) {
  if (!show_quick_app && quick_app_button_) {
    AnimateQuickAppButtonOut();
  } else if (show_quick_app && !quick_app_button_) {
    if (nudge_label_) {
      RemoveNudgeLabel();
    }
    AnimateQuickAppButtonIn();
  }
}

void HomeButton::OnQuickAppIconChanged() {
  if (!quick_app_button_) {
    return;
  }

  const int control_size =
      ShelfControlButton::CalculatePreferredSize({}).width();
  quick_app_button_->SetImageModel(
      views::Button::STATE_NORMAL,
      ui::ImageModel::FromImageSkia(
          AppListModelProvider::Get()->quick_app_access_model()->GetAppIcon(
              gfx::Size(control_size, control_size))));
}

void HomeButton::AnimateQuickAppButtonIn() {
  CreateQuickAppButton();

  CHECK(quick_app_button_ && expandable_container_ && !nudge_label_);

  const gfx::Rect initial_container_clip_rect =
      GetExpandableContainerClipRectToHomeButton();
  const gfx::Transform initial_transform =
      GetTransformForContainerChildBehindHomeButton();

  const gfx::Rect container_target_clip_rect =
      gfx::Rect(expandable_container_->size());

  views::AnimationBuilder()
      .SetPreemptionStrategy(
          ui::LayerAnimator::IMMEDIATELY_ANIMATE_TO_NEW_TARGET)
      .Once()
      .SetDuration(base::TimeDelta())
      .SetTransform(quick_app_button_->layer(), initial_transform)
      .SetClipRect(expandable_container_->layer(), initial_container_clip_rect)
      .SetOpacity(quick_app_button_->layer(), 0)
      .SetOpacity(expandable_container_->layer(), 0)
      .Then()
      .SetDuration(kQuickAppSlideSlideInDuration)
      .SetClipRect(expandable_container_->layer(), container_target_clip_rect,
                    gfx::Tween::ACCEL_20_DECEL_100)
      .SetTransform(quick_app_button_->layer(), gfx::Transform(),
                    gfx::Tween::ACCEL_20_DECEL_100)
      .At(kQuickAppButtonFadeInDelay)
      .SetDuration(kQuickAppButtonFadeInDuration)
      .SetOpacity(quick_app_button_->layer(), 1)
      .At(base::TimeDelta())
      .SetDuration(kQuickAppContainerFadeInDuration)
      .SetOpacity(expandable_container_->layer(), 1);
}

void HomeButton::AnimateQuickAppButtonOut() {
  const gfx::Transform target_transform =
      GetTransformForContainerChildBehindHomeButton();
  const gfx::Rect container_target_clip_rect =
      GetExpandableContainerClipRectToHomeButton();

  views::AnimationBuilder()
      .SetPreemptionStrategy(
          ui::LayerAnimator::IMMEDIATELY_ANIMATE_TO_NEW_TARGET)
      .OnEnded(base::BindOnce(&HomeButton::OnQuickAppButtonSlideOutDone,
                              weak_ptr_factory_.GetWeakPtr()))
      .OnAborted(base::BindOnce(&HomeButton::OnQuickAppButtonSlideOutDone,
                                weak_ptr_factory_.GetWeakPtr()))
      .Once()
      .SetDuration(kQuickAppSlideOutDuration)
      .SetTransform(quick_app_button_->layer(), target_transform,
                    gfx::Tween::ACCEL_20_DECEL_100)
      .SetClipRect(expandable_container_->layer(), container_target_clip_rect,
                    gfx::Tween::ACCEL_20_DECEL_100)
      .At(base::TimeDelta())
      .SetDuration(kQuickAppFadeOutDuration)
      .SetOpacity(expandable_container_->layer(), 0)
      .SetOpacity(quick_app_button_->layer(), 0);
}

void HomeButton::OnQuickAppButtonSlideOutDone() {
  RemoveQuickAppButton();
  shelf_->shelf_layout_manager()->LayoutShelf(true);
}

gfx::Transform HomeButton::GetTransformForContainerChildBehindHomeButton() {
  const int home_button_width =
      ShelfControlButton::CalculatePreferredSize({}).width();

  const int container_visible_width =
      expandable_container_->width() - home_button_width;

  gfx::Transform target_transform;
  if (shelf_->IsHorizontalAlignment()) {
    target_transform.Translate(base::i18n::IsRTL() ? container_visible_width
                                                   : -container_visible_width,
                               0);
  } else {
    target_transform.Translate(
        0, home_button_width - expandable_container_->height());
  }
  return target_transform;
}

gfx::Rect HomeButton::GetExpandableContainerClipRectToHomeButton() {
  const int home_button_width =
      ShelfControlButton::CalculatePreferredSize({}).width();
  const int container_visible_width =
      expandable_container_->width() - home_button_width;

  return gfx::Rect(base::i18n::IsRTL() ? container_visible_width : 0, 0,
                    home_button_width, home_button_width);
}

BEGIN_METADATA(HomeButton)
END_METADATA

}  // namespace ash
