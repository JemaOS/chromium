// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/ash/login/network_screen_handler.h"

#include <stddef.h>

#include <utility>

#include "base/values.h"
#include "chrome/browser/ash/login/demo_mode/demo_setup_controller.h"
#include "chrome/browser/ash/login/screens/network_screen.h"
#include "chrome/browser/ash/login/startup_utils.h"
#include "chrome/browser/ui/webui/ash/cellular_setup/cellular_setup_localized_strings_provider.h"
#include "chrome/grit/generated_resources.h"
#include "chromeos/ash/components/network/network_handler.h"
#include "chromeos/ash/components/network/technology_state_controller.h"
#include "components/login/localized_values_builder.h"
#include "ui/chromeos/strings/network/network_element_localized_strings_provider.h"

namespace ash {

NetworkScreenHandler::NetworkScreenHandler() : BaseScreenHandler(kScreenId) {}

NetworkScreenHandler::~NetworkScreenHandler() = default;

void NetworkScreenHandler::Show() {
  // In OOBE all physical network technologies should be enabled, so the user is
  // able to select any of the available networks on the device. Enabled
  // technologies should not be changed if network screen is shown outside of
  // OOBE.
  // If OOBE is not completed, we assume that the only instance of this object
  // could be OOBE itself.
  if (!StartupUtils::IsOobeCompleted()) {
    TechnologyStateController* controller =
        NetworkHandler::Get()->technology_state_controller();
    controller->SetTechnologiesEnabled(NetworkTypePattern::Physical(), true,
                                       network_handler::ErrorCallback());
  }

  base::Value::Dict data;
  data.Set("isDemoModeSetup",
           DemoSetupController::IsOobeDemoSetupFlowInProgress());
  ShowInWebUI(std::move(data));
}

void NetworkScreenHandler::ShowError(const std::u16string& message) {
  CallExternalAPI("setError", message);
}

void NetworkScreenHandler::ClearErrors() {
  CallExternalAPI("setError", std::string());
}

void NetworkScreenHandler::OnJemaosOnlineAccountSelected() {
  CallExternalAPI("onJemaosOnlineAccountSelected");
}

void NetworkScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {
  builder->Add("networkSectionTitle", IDS_NETWORK_SELECTION_TITLE);
  builder->Add("networkSectionTitleWiFi", IDS_NETWORK_SELECTION_TITLE_WIFI);
  builder->Add("networkSectionTitleEthernet", IDS_NETWORK_SELECTION_TITLE_ETHERNET);
  builder->Add("networkSectionHint", IDS_NETWORK_SELECTION_HINT);
  builder->Add("proxySettingsListItemName",
               IDS_NETWORK_PROXY_SETTINGS_LIST_ITEM_NAME);
  builder->Add("addWiFiListItemName", IDS_NETWORK_ADD_WI_FI_LIST_ITEM_NAME);
  
  // JemaOS online account network requirement strings
  builder->Add("jemaosOnlineAccountNetworkRequiredError", IDS_JEMAOS_ONLINE_ACCOUNT_NETWORK_REQUIRED_ERROR);
  builder->Add("jemaosOnlineAccountNetworkRequiredMessage", IDS_JEMAOS_ONLINE_ACCOUNT_NETWORK_REQUIRED_MESSAGE);
  builder->Add("jemaosOnlineAccountWifiPreferredMessage", IDS_JEMAOS_ONLINE_ACCOUNT_WIFI_PREFERRED_MESSAGE);
  
  ui::network_element::AddLocalizedValuesToBuilder(builder);
  cellular_setup::AddLocalizedValuesToBuilder(builder);
}

void NetworkScreenHandler::GetAdditionalParameters(base::Value::Dict* dict) {
  cellular_setup::AddNonStringLoadTimeDataToDict(dict);
}

}  // namespace ash
