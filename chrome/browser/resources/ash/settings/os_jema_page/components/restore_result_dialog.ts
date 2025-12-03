import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import 'chrome://resources/ash/common/cr_elements/cr_dialog/cr_dialog.js';
import 'chrome://resources/ash/common/cr_elements/icons.html.js';
import 'chrome://resources/polymer/v3_0/iron-icon/iron-icon.js';
import {I18nMixin} from 'chrome://resources/ash/common/cr_elements/i18n_mixin.js';
import {CrDialogElement} from 'chrome://resources/ash/common/cr_elements/cr_dialog/cr_dialog.js';

import {getTemplate} from './restore_result_dialog.html.js';

interface RestoreResultDialog {
  $: {
    dialog: CrDialogElement,
  };
}

const RestoreResultDialogElementBase = I18nMixin(PolymerElement);

class RestoreResultDialog extends RestoreResultDialogElementBase {
  static get is() {
    return 'restore-result-dialog';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      isSuccess: {
        type: Boolean,
        value: false,
      },
      message: {
        type: String,
        value: '',
      },
    };
  }

  isSuccess: boolean;
  message: string;

  override ready() {
    super.ready();
    console.log('restore result dialog ready');
  }

  override connectedCallback() {
    super.connectedCallback();
    this.$.dialog.showModal();
  }

  getTitle_(isSuccess: boolean): string {
    return isSuccess 
      ? this.i18n('jemaosSettingsRestoreSuccessTitle')
      : this.i18n('jemaosSettingsRestoreErrorTitle');
  }

  onOkClick_() {
    console.log('ok clicked');
    this.$.dialog.close();
  }
}

customElements.define(
    RestoreResultDialog.is, RestoreResultDialog);

