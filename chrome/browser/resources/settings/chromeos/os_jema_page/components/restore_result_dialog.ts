import '//resources/cr_elements/cr_button/cr_button.js';
import '//resources/cr_elements/cr_dialog/cr_dialog.js';
import '//resources/cr_elements/cr_shared_style.css.js';
import '//resources/cr_elements/icons.html.js';
import '//resources/polymer/v3_0/iron-icon/iron-icon.js';
import '../../../settings_shared.css.js';

import {PolymerElement, mixinBehaviors} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {CrDialogElement} from '//resources/cr_elements/cr_dialog/cr_dialog.js';

import {getTemplate} from './restore_result_dialog.html.js';

interface RestoreResultDialog {
  $: {
    dialog: CrDialogElement,
  };
}

const RestoreResultDialogElementBase = mixinBehaviors(
    [], PolymerElement);

class RestoreResultDialog extends RestoreResultDialogElementBase {
  static get is() {
    return 'restore-result-dialog';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      message: {
        type: String,
        value: '',
      },

      isSuccess: {
        type: Boolean,
        value: false,
      },
    };
  }

  message: string;
  isSuccess: boolean;

  override connectedCallback() {
    super.connectedCallback();
    this.$.dialog.showModal();
  }

  onOkClick_() {
    this.$.dialog.close();
  }

  getTitle_(isSuccess: boolean): string {
    return isSuccess ? 'Restore Successful' : 'Restore Failed';
  }
}

customElements.define(
    RestoreResultDialog.is, RestoreResultDialog as any);
