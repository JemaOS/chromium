// Copyright 2025 jema technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {html, mixinBehaviors, PolymerElement} from '//resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {OobeI18nMixin} from '../../components/mixins/oobe_i18n_mixin.js';
import {OobeDialogHostMixin} from '../../components/mixins/oobe_dialog_host_mixin.js';
import '//resources/ash/common/cr_elements/cr_radio_button/cr_card_radio_button.js';
import '//resources/ash/common/cr_elements/cr_radio_group/cr_radio_group.js';
import '../../components/buttons/oobe_back_button.js';
import '../../components/buttons/oobe_next_button.js';
import '../../components/common_styles/oobe_common_styles.css.js';
import '../../components/common_styles/cr_card_radio_group_styles.css.js';
import '../../components/common_styles/oobe_dialog_host_styles.css.js';
import '../../components/dialogs/oobe_adaptive_dialog.js';
import {loadTimeData} from '//resources/ash/common/load_time_data.m.js';
import {getTemplate} from './account_type_selection.html.js';


const AccountTypeSelectionScreenElementBase =
  OobeDialogHostMixin(OobeI18nMixin(PolymerElement));

const AccountTypeToSelect = {
  GOOGLE: 'google',
  JEMA: 'jema',
  JEMA_LOCAL: 'jema-local',
};

class AccountTypeSelection extends AccountTypeSelectionScreenElementBase {
  static get is() {
    return 'account-type-selection-element';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      selectedAccountType_: {
        type: String,
      },
      hideBackButton_: {
        type: Boolean,
        value: () => {
          return loadTimeData.getBoolean('isOobeFlow');
        },
      },
      isOnline_: {
        type: Boolean,
        value: true,
      },
      hideLocalButton_: {
        type: Boolean,
        computed: 'computeHideLocalButton_(isOnline_)',
      },
    }
  }

  private selectedAccountType_: string;
  private hideBackButton_: boolean;
  private isOnline_: boolean;
  private hideLocalButton_: boolean;


  override ready() {
    super.ready();
    this.checkInternetConnectivity_();
    // Default to online JEMA account when online, local when offline
    this.selectedAccountType_ = this.isOnline_ ? 
        AccountTypeToSelect.JEMA : AccountTypeToSelect.JEMA_LOCAL;
  }

  private computeHideLocalButton_(isOnline: boolean): boolean {
    // Hide local button when online, show when offline
    return isOnline;
  }

  private checkInternetConnectivity_() {
    // Check if browser reports online status
    this.isOnline_ = navigator.onLine;
    
    // Listen for online/offline events
    window.addEventListener('online', () => {
      this.isOnline_ = true;
      // Switch to online account if currently on local
      if (this.selectedAccountType_ === AccountTypeToSelect.JEMA_LOCAL) {
        this.selectedAccountType_ = AccountTypeToSelect.JEMA;
      }
    });
    
    window.addEventListener('offline', () => {
      this.isOnline_ = false;
      // Switch to local account if currently on online
      if (this.selectedAccountType_ === AccountTypeToSelect.JEMA) {
        this.selectedAccountType_ = AccountTypeToSelect.JEMA_LOCAL;
      }
    });
  }


  onBackButtonClicked_() {
    this.dispatchEvent(new CustomEvent('account-type-selection-back'));
  }

  onNextClicked_() {
    this.dispatchEvent(new CustomEvent('account-type-selected', { detail: this.selectedAccountType_ }));
  }

}

customElements.define(AccountTypeSelection.is, AccountTypeSelection);
