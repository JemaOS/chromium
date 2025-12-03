import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import 'chrome://resources/ash/common/cr_elements/cr_dialog/cr_dialog.js';
import 'chrome://resources/ash/common/cr_elements/cr_input/cr_input.js';
import {I18nMixin} from 'chrome://resources/ash/common/cr_elements/i18n_mixin.js';
import {CrDialogElement} from 'chrome://resources/ash/common/cr_elements/cr_dialog/cr_dialog.js';

import {getTemplate} from './restore_password_dialog.html.js';

interface RestorePasswordDialog {
  $: {
    dialog: CrDialogElement,
  };
}

const RestorePasswordDialogElementBase = I18nMixin(PolymerElement);

class RestorePasswordDialog extends RestorePasswordDialogElementBase {
  static get is() {
    return 'restore-password-dialog';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      password_: {
        type: String,
        value: '',
      },
    };
  }

  private password_: string;

  override ready() {
    super.ready();
    console.log('restore password dialog ready');
  }

  override connectedCallback() {
    super.connectedCallback();
    this.$.dialog.showModal();
  }

  onCancelClick_() {
    console.log('cancel restore');
    this.dispatchEvent(new CustomEvent('cancel', {
      bubbles: true,
      composed: true,
    }));
    this.$.dialog.close();
  }

  onRestoreClick_() {
    console.log('restore with password');
    this.dispatchEvent(new CustomEvent('restore-password-obtained', {
      bubbles: true,
      composed: true,
      detail: this.password_,
    }));
    this.$.dialog.close();
  }

  onKeyPress_(e: KeyboardEvent) {
    if (e.key === 'Enter' && this.password_) {
      this.onRestoreClick_();
    }
  }
}

customElements.define(
    RestorePasswordDialog.is, RestorePasswordDialog);

