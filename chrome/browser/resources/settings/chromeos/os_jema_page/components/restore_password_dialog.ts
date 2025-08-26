import '//resources/cr_elements/cr_button/cr_button.js';
import '//resources/cr_elements/cr_dialog/cr_dialog.js';
import '//resources/cr_elements/cr_input/cr_input.js';
import '//resources/cr_elements/cr_shared_style.css.js';
import '../../../settings_shared.css.js';

import {PolymerElement, mixinBehaviors} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {CrDialogElement} from '//resources/cr_elements/cr_dialog/cr_dialog.js';
import {CrInputElement} from '//resources/cr_elements/cr_input/cr_input.js';

import {getTemplate} from './restore_password_dialog.html.js';

interface RestorePasswordDialog {
  $: {
    dialog: CrDialogElement,
    passwordInput: CrInputElement,
    showPasswordCheckbox: HTMLInputElement,
    noPasswordCheckbox: HTMLInputElement,
  };
}

const RestorePasswordDialogElementBase = mixinBehaviors(
    [], PolymerElement);

class RestorePasswordDialog extends RestorePasswordDialogElementBase {
  static get is() {
    return 'restore-password-dialog';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      restoreFile: {
        type: String,
        value: '',
      },

      password_: {
        type: String,
        value: '',
        notify: true,
      },

      showPassword_: {
        type: Boolean,
        value: false,
      },

      noPassword_: {
        type: Boolean,
        value: false,
      },

      isRestoreEnabled_: {
        type: Boolean,
        computed: 'computeIsRestoreEnabled_(password_, noPassword_)',
      },
    };
  }

  restoreFile: string;
  private password_: string;
  private showPassword_: boolean;
  private noPassword_: boolean;
  private isRestoreEnabled_: boolean;

  override connectedCallback() {
    super.connectedCallback();
    this.$.dialog.showModal();
    // Focus the password input after the dialog opens
    window.setTimeout(() => {
      this.$.passwordInput.focus();
    }, 1);
  }

  onCancelClick_() {
    this.$.dialog.cancel();
  }

  onRestoreClick_() {
    const password = this.noPassword_ ? '' : this.password_;
    
    // Dispatch event with the password
    this.dispatchEvent(new CustomEvent(
        'restore-password-entered',
        {bubbles: true, composed: true, detail: {password}}));

    this.$.dialog.close();
  }

  onNoPasswordChanged_() {
    if (this.noPassword_) {
      this.password_ = '';
      this.$.passwordInput.value = '';
    }
    this.$.passwordInput.disabled = this.noPassword_;
  }

  getPasswordInputType_(showPassword: boolean): string {
    return showPassword ? 'text' : 'password';
  }

  getFileName_(filePath: string): string {
    if (!filePath) return '';
    const parts = filePath.split('/');
    return parts[parts.length - 1];
  }

  computeIsRestoreEnabled_(password: string, noPassword: boolean): boolean {
    return noPassword || Boolean(password && password.length > 0);
  }
}

customElements.define(
    RestorePasswordDialog.is, RestorePasswordDialog as any);
