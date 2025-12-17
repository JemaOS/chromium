import { PolymerElement } from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import 'chrome://resources/ash/common/cr_elements/cr_dialog/cr_dialog.js';
import 'chrome://resources/ash/common/cr_elements/cr_input/cr_input.js';
import { I18nMixin } from 'chrome://resources/ash/common/cr_elements/i18n_mixin.js';
import { WebUiListenerMixin } from 'chrome://resources/ash/common/cr_elements/web_ui_listener_mixin.js';
import { CrDialogElement } from 'chrome://resources/ash/common/cr_elements/cr_dialog/cr_dialog.js';

import { getTemplate } from './restore_password_dialog.html.js';

interface BackupFile {
  fileKey: string;
  fileName: string;
  size: number;
  lastModified: string;
}

interface RestorePasswordDialog {
  $: {
    dialog: CrDialogElement,
  };
}

const RestorePasswordDialogElementBase = WebUiListenerMixin(I18nMixin(PolymerElement));

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
      selectedFileKey_: {
        type: String,
        value: '',
      },
      isCloudRestore: {
        type: Boolean,
        value: false,
      },
      canRestore_: {
        type: Boolean,
        value: false,
      },
      backupFiles_: {
        type: Array,
        value: () => [],
      },
      loadingFiles_: {
        type: Boolean,
        value: false,
      },
      hasBackupFiles_: {
        type: Boolean,
        computed: 'computeHasBackupFiles_(backupFiles_)',
      },
      email: {
        type: String,
        value: '',
      },
    };
  }

  static get observers() {
    return [
      'updateCanRestore_(password_, selectedFileKey_, isCloudRestore)',
    ];
  }

  private password_: string;
  private selectedFileKey_: string;
  isCloudRestore: boolean;
  private canRestore_: boolean;
  private backupFiles_: BackupFile[];
  private loadingFiles_: boolean;
  private hasBackupFiles_: boolean;
  email: string;

  override ready() {
    super.ready();
    console.log('restore password dialog ready');
  }

  override connectedCallback() {
    super.connectedCallback();
    this.$.dialog.showModal();

    // Listen for the file list result
    this.addWebUiListener('jemaos-cloud-list-files-result', this.onBackupFilesReceived_.bind(this));

    // If cloud restore, fetch the file list
    if (this.isCloudRestore && this.email) {
      this.fetchBackupFiles_();
    }
  }

  fetchBackupFiles_() {
    console.log('Fetching backup files for:', this.email);
    this.loadingFiles_ = true;
    chrome.send('jemaosCloudListBackupFiles', [this.email]);
  }

  private onBackupFilesReceived_(success: boolean, files: BackupFile[]) {
    console.log('Backup files received:', success, files);
    this.loadingFiles_ = false;
    if (success && files) {
      this.backupFiles_ = files;
    } else {
      this.backupFiles_ = [];
    }
  }

  updateCanRestore_(password: string, selectedFileKey: string, isCloudRestore: boolean) {
    console.log('updateCanRestore_', password, selectedFileKey, isCloudRestore);
    if (isCloudRestore) {
      this.canRestore_ = !!(password && selectedFileKey);
    } else {
      this.canRestore_ = !!password;
    }
    console.log('canRestore_:', this.canRestore_);
  }

  onFileSelected_(e: Event) {
    const select = e.target as HTMLSelectElement;
    this.selectedFileKey_ = select.value;
    console.log('File selected:', this.selectedFileKey_);
  }

  formatFileSize_(bytes: number): string {
    if (bytes < 1024) return bytes + ' B';
    if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
    return (bytes / (1024 * 1024)).toFixed(1) + ' MB';
  }

  formatDate_(dateStr: string): string {
    if (!dateStr) return '';
    const date = new Date(dateStr);
    return date.toLocaleString();
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
    console.log('restore with password, fileKey:', this.selectedFileKey_);
    this.dispatchEvent(new CustomEvent('restore-password-obtained', {
      bubbles: true,
      composed: true,
      detail: {
        password: this.password_,
        fileKey: this.selectedFileKey_,
      },
    }));
    this.$.dialog.close();
  }

  onKeyPress_(e: KeyboardEvent) {
    if (e.key === 'Enter' && this.canRestore_) {
      this.onRestoreClick_();
    }
  }

  computeHasBackupFiles_(backupFiles: BackupFile[]): boolean {
    return backupFiles && backupFiles.length > 0;
  }
}

customElements.define(
  RestorePasswordDialog.is, RestorePasswordDialog);

