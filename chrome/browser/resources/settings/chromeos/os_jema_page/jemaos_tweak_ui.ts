// os-settings-jemaos-tweak-ui
import {PolymerElement, mixinBehaviors} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {assert} from 'chrome://resources/js/assert_ts.js';
import {sendWithPromise} from 'chrome://resources/js/cr.js';
import {I18nMixin} from 'chrome://resources/cr_elements/i18n_mixin.js';
import {RouteObserverMixin} from '../route_observer_mixin.js'
import {WebUiListenerMixin} from 'chrome://resources/cr_elements/web_ui_listener_mixin.js';
import {CrDialogElement} from 'chrome://resources/cr_elements/cr_dialog/cr_dialog.js';
import {LifetimeBrowserProxyImpl} from '/shared/settings/lifetime_browser_proxy.js';
import 'chrome://resources/cr_components/localized_link/localized_link.js';
import 'chrome://resources/cr_elements/cr_toggle/cr_toggle.js';
import 'chrome://resources/cr_elements/cr_shared_style.css.js';
import 'chrome://resources/cr_elements/cr_dialog/cr_dialog.js';
import '../../settings_shared.css.js';
import '../../controls/password_prompt_dialog.js';
import './components/backup_intro_dialog.js';
import './components/restore_password_dialog.js';
import './components/restore_result_dialog.js';
import {ShellClient} from './shell_client.js';

import {getTemplate} from './jemaos_tweak_ui.html.js';

class WidevineHelper {
  private element_: HTMLElement;
  private shellClient_: ShellClient;

  constructor(ele: HTMLElement) {
    this.element_ = ele;
    this.shellClient_ = new ShellClient(ele);
  }

  static get Command() {
    return '/usr/bin/enable_libwidevine';
  }

  static get RunningStateParam() {
    return '--state';
  }

  static get StatusParam() {
    return '--status';
  }

  static get EnableParam() {
    return '--file';
  }

  static get DisableParam() {
    return '--disable';
  }

  async isSupported() {
    const ret = await this.shellClient_.IsFileExist(WidevineHelper.Command);
    return ret;
  }

  async getRunningState() {
    const result = await this.shellClient_.SafeExecForResult(`${WidevineHelper.Command} ${WidevineHelper.RunningStateParam}`);
    return result;
  }

  async getWidevineStatus() {
    const result = await this.shellClient_.SafeExecForResult(`${WidevineHelper.Command} ${WidevineHelper.StatusParam}`);
    return result;
  }

  async toggle(file: string | null) {
    if (file) {
      await this.enable(file);
    } else {
      await this.disable();
    }
  }

  async install(file: string) {
    await this.shellClient_.ExecForResult(
      `${WidevineHelper.Command} ${WidevineHelper.EnableParam} ${file}`);
  }

  async enable(file: string) {
    try {
      await this.install(file);
    } catch (err) {
      console.log('enable widevine failed', err);
      throw err;
    }
  }

  async disable() {
    await this.shellClient_.SafeExecForResult(`${WidevineHelper.Command} ${WidevineHelper.DisableParam}`);
  }
}

const WIDEVINE_STATUS_KEY = 'jemaos_libwidevine_enabled';

const JemaSettingsTweakUIPageElementBase =
    RouteObserverMixin(WebUiListenerMixin(I18nMixin(PolymerElement)));

interface JemaSettingsTweakUiPageElement {
  $: {
    widevineErrorDialog: CrDialogElement,
  };
}

class JemaSettingsTweakUiPageElement extends JemaSettingsTweakUIPageElementBase {
  static get is() {
    return 'os-settings-jemaos-tweak-ui' as const;
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      showRebootButton_: Boolean,
      showRotateScreenButton_: Boolean,
      isInTabletPhysicalState_: Boolean,
      showSwitchTabletLaptopButton_: Boolean,
      canToggleRotateScreenButton_: {
        type: Boolean,
        computed: 'computeCanToggleRotateScreenButton_(isInTabletPhysicalState_)',
      },
      showToggleWidevine_: {
        type: Boolean,
        value: false,
      },
      libwidevineEnabled_: {
        type: Boolean,
        value: false,
      },
      togglingWidevine_: {
        type: Boolean,
        value: false,
      },
      showWidevineErrorDialog_: {
        type: Boolean,
        value: false,
      },
      rebootRequiredForWidevine_: {
        type: Boolean,
        value: false,
      },
      showBackup_: {
        type: Boolean,
        value: false,
      },
      backupRunning_: {
        type: Boolean,
        value: false,
      },
      showPasswordPromptDialog_: {
        type: Boolean,
        value: false,
      },
      showBackupIntroDialog_: {
        type: Boolean,
        value: false,
      },
      showRestore_: {
        type: Boolean,
        value: false,
      },
      restoreRunning_: {
        type: Boolean,
        value: false,
      },
      showRestoreFileDialog_: {
        type: Boolean,
        value: false,
      },
      showRestorePasswordDialog_: {
        type: Boolean,
        value: false,
      },
      selectedRestoreFile_: {
        type: String,
        value: '',
      },
    };
  }

  private showRebootButton_: boolean;
  private showRotateScreenButton_: boolean;
  private isInTabletPhysicalState_: boolean;
  private showSwitchTabletLaptopButton_: boolean;
  private canToggleRotateScreenButton_: boolean;

  private showToggleWidevine_: boolean;
  private libwidevineEnabled_: boolean;
  private togglingWidevine_: boolean;

  private showBackup_: boolean;
  private backupRunning_: boolean;
  private showPasswordPromptDialog_: boolean;
  private showBackupIntroDialog_: boolean;

  private showRestore_: boolean;
  private restoreRunning_: boolean;
  private showRestoreFileDialog_: boolean;
  private showRestorePasswordDialog_: boolean;
  private selectedRestoreFile_: string;

  private client_: WidevineHelper;

  private backupEmail_: string;
  private backupFilePassword_: string;
  private backupCanceled_: boolean;

  private showWidevineErrorDialog_: boolean;
  private rebootRequiredForWidevine_: boolean;

  constructor() {
    super();
    this.client_ =  new WidevineHelper(this);
    this.backupEmail_ = '';
    this.backupFilePassword_ = '';
  }

  override connectedCallback() {
    this.getShowRebootButtonInTray();
    this.getShowRotateScreenButton();
    this.getIsInTabletPhysicalState();
    this.getShowSwitchTabletLaptopButton();
    this.addWebUiListener('show-reboot-button-in-tray-changed', this.onShowRebootButtonInTrayChanged_.bind(this));
    this.addWebUiListener('show-rotate-screen-button-changed', this.onShowRotateScreenButtonChanged_.bind(this));
    this.addWebUiListener('is-in-tablet-physical-state-changed', this.onIsInTabletPhysicalStateChanged_.bind(this));
    this.addWebUiListener('show-switch-tablet-laptop-button-changed', this.onShowSwitchTabletLaptopButtonChanged_.bind(this));
    this.addWebUiListener('jemaos-libwidevine-file-selected', this.onLibwidevineFileSelected_.bind(this));

    this.addWebUiListener('jemaos-backup-file-selected', this.onBackupFileSelected_.bind(this));
    this.addWebUiListener('jemaos-backup-task-finished', this.onBackupDone_.bind(this));
    this.addWebUiListener('jemaos-restore-file-selected', this.onRestoreFileSelected_.bind(this));

    this.checkLibwidevineStatus_();
    this.createBackupScript_().then(() => {
      this.checkBackupSupported_();
      this.checkRestoreSupported_();
    });
  }

  getShowRotateScreenButton() {
    sendWithPromise('getShowRotateScreenButton').then((visible) => {
      this.showRotateScreenButton_ = visible;
    });
  }

  getShowRebootButtonInTray() {
    sendWithPromise('getShowRebootButtonInTray').then((visible) => {
      this.showRebootButton_ = visible;
    });
  }

  getIsInTabletPhysicalState() {
    sendWithPromise('getIsInTabletPhysicalState').then((enabled) => {
      this.isInTabletPhysicalState_ = enabled;
    });
  }

  getShowSwitchTabletLaptopButton() {
    sendWithPromise('getShowSwitchTabletLaptopButton').then((enabled) => {
      console.log('getShowSwitchTabletLaptopButton', enabled);
      this.showSwitchTabletLaptopButton_ = enabled;
    });
  }

  onToggleShowSwitchTabletLaptopButton_() {
    this.showSwitchTabletLaptopButton_ = !this.showSwitchTabletLaptopButton_;
    chrome.send('setShowSwitchTabletLaptopButton', [this.showSwitchTabletLaptopButton_]);
  }

  onShowSwitchTabletLaptopButtonChanged_(visible: boolean) {
    console.log('onShowSwitchTabletLaptopButtonChanged_', visible);
    this.showSwitchTabletLaptopButton_ = visible;
  }

  onShowRebootButtonInTrayChanged_(visible: boolean) {
    this.showRebootButton_ = visible;
  }

  onShowRotateScreenButtonChanged_(visible: boolean) {
    this.showRotateScreenButton_ = visible;
  }

  onIsInTabletPhysicalStateChanged_(enabled: boolean) {
    this.isInTabletPhysicalState_ = enabled;
  }

  computeCanToggleRotateScreenButton_(isInTabletPhysicalState: boolean) {
    const canToggleRotateScreenButton = isInTabletPhysicalState;
    return canToggleRotateScreenButton;
  }

  onToggleShowRotateScreenButton_() {
    this.showRotateScreenButton_ = !this.showRotateScreenButton_;
    chrome.send('setShowRotateScreenButton', [this.showRotateScreenButton_]);
  }

  onToggleShowRebootButton_() {
    this.showRebootButton_ = !this.showRebootButton_;
    chrome.send('setShowRebootButtonInTray', [this.showRebootButton_]);
  }

  shouldShowRotateScreenButton_() {
    return this.showRotateScreenButton_ && this.canToggleRotateScreenButton_;
  }

  getShowRebootButtonMessage_() {
    return this.i18nAdvanced('displayJemaOsRebootButtonInTray')
  }

  getShowRotateScreenButtonMessage_() {
    if (!this.isInTabletPhysicalState_) {
      return this.i18nAdvanced('notTabletPhysicalStateDisableJemaOsRotateScreen');
    }
    return this.i18nAdvanced('displayJemaOsRotateScreenButton');
  }

  async checkLibwidevineStatus_() {
    const support = await this.client_.isSupported();
    if (support) {
      this.getLibwidevineScriptRunningState_();
      const result = await this.getLibwidevineEnabled_();
      // if we can't get 'yes' or 'no' throught the script, do not show libwidevine settings block
      this.showToggleWidevine_ = !!result;
    }
    if (this.showToggleWidevine_) {
      this.getRebootRequiredForWidevine_();
    }
  }

  async getLibwidevineEnabled_() {
    const result = await this.client_.getWidevineStatus();
    const enabled = (result === 'yes');
    this.libwidevineEnabled_ = enabled;
    return result;
  }

  async getLibwidevineScriptRunningState_() {
    const result = await this.client_.getRunningState();
    this.togglingWidevine_ = (result === 'yes');
  }

  async onToggleLibwidevine_() {
    if (this.togglingWidevine_) {
      return;
    }
    const enabled = !this.libwidevineEnabled_;
    this.libwidevineEnabled_ = enabled;
    if (enabled) {
      chrome.send('selectLibwidevineFile', []);
    } else {
      await this.disableLibwidevine_();
    }
  }

  async enableLibwidevine_(file: string) {
    this.togglingWidevine_ = true;
    try {
      await this.client_.toggle(file);
    } catch (e) {
      this.showWidevineErrorDialog_ = true;
    }
    this.togglingWidevine_ = false;
    const result = await this.getLibwidevineEnabled_();
    if (result === 'yes') {
      this.toggleRebootRequiredForWidevine_(true);
    }
  }

  async disableLibwidevine_() {
    this.togglingWidevine_ = true;
    await this.client_.toggle(null);
    this.togglingWidevine_ = false;
    const result = await this.getLibwidevineEnabled_();
    if (result === 'no') {
      this.toggleRebootRequiredForWidevine_(false);
    }
  }

  getRebootRequiredForWidevine_() {
    sendWithPromise('getRebootRequiredForWidevine').then((required: boolean) => {
      this.rebootRequiredForWidevine_ = required;
    });
  }

  toggleRebootRequiredForWidevine_(force: boolean) {
    sendWithPromise('toggleRebootRequiredForWidevine', force).then((required: boolean) => {
      this.rebootRequiredForWidevine_ = required;
    });
  }

  shouldShowRebootButtonForWidevine_(toggling: boolean, required: boolean) {
    return !toggling && required;
  }

  onRestartForWidevineTap_() {
    LifetimeBrowserProxyImpl.getInstance().signOutAndRestart();
  }

  toggleWidevineHelpMsg_() {
    return this.i18nAdvanced('toggleWidevineHelpMessage');
  }

  onWidevineErrorDialogClose_() {
    this.showWidevineErrorDialog_ = false;
  }

  async onLibwidevineFileSelected_(file: string | null) {
    if (file) {
      await this.enableLibwidevine_(file);
    } else {
      this.libwidevineEnabled_ = false;
    }
  }

  async createBackupScript_() {
    const shellClient = new ShellClient(this);
    
    // Check if backup script already exists
    const exists = await shellClient.IsFileExist('/usr/bin/jemaos-backup');
    if (exists) {
      console.log('Backup script already exists');
      return;
    }

    console.log('Creating backup script...');
    
    // Use sendWithPromise to request script creation from backend
    try {
      await sendWithPromise('createJemaosBackupScript');
      console.log('Backup script created successfully');
    } catch (error) {
      console.error('Failed to create backup script:', error);
    }
  }

  async checkBackupSupported_() {
    sendWithPromise('jemaosBackupSupported').then((supported) => {
      if (!supported) {
        this.showBackup_ = false;
        return;
      }
      chrome.usersPrivate.getCurrentUser().then((user) => {
        this.backupEmail_ = user && user.email;
        if (this.backupEmail_) {
          this.showBackup_ = true;
          this.getJemaosBackupState_();
        } else {
          console.error('failed to get current user, backup is disabled');
          this.showBackup_ = false;
        }
      });
    })
  }

  getJemaosBackupState_() {
    sendWithPromise('getJemaosBackupState').then((state) => {
      this.backupRunning_ = (state === 'running');
    });
  }

  generateDefaultFilename(email: string) {
    const date = new Date();
    const year = date.getFullYear().toString().padStart(4, '0');
    const month = (date.getMonth() + 1).toString().padStart(2, '0');
    const day = date.getDate().toString().padStart(2, '0');
    const hour = date.getHours().toString().padStart(2, '0');
    const minute = date.getMinutes().toString().padStart(2, '0');

    const regex = /[^A-Za-z0-9]/g;
    const name = email.split('@')[0];
    const filename = `jemaos_${name.replace(regex, '_')}_${year}${month}${day}_${hour}${minute}.bak`;
    return filename;
  }

  onBackupClick_() {
    // this.showPasswordPromptDialog_ = true;
    this.backupCanceled_ = false;
    this.showBackupIntroDialog_ = true;
  }


  openSelectBackupFileDialog_() {
    const defaultFilename = this.generateDefaultFilename(this.backupEmail_);
    console.log('defaultFilename', defaultFilename);
    chrome.send('jemaosBackupSelectFile', [defaultFilename]);
  }

  onBackupFileSelected_(canceled: boolean) {
    if (canceled) {
      this.backupFilePassword_ = '';
      return;
    }
    this.startBackup_();
  }

  startBackup_() {
    this.backupRunning_ = true;
    chrome.send('jemaosBackupStarted', [this.backupEmail_, this.backupFilePassword_]);
  }

  onBackupDone_(success: boolean) {
    console.log('onBackupDone_, result', success);
    this.backupRunning_ = false;
  }

  onBackupIntroDialogClosed_(e: Event) {
    console.log('intro dialog closed', e);
    this.showBackupIntroDialog_ = false;
    if (!this.backupCanceled_) {
      console.log('prompt for password');
      this.backupFilePassword_ = '';
      this.showPasswordPromptDialog_ = true;
    }
  }

  onBackupIntroDialogCanceled_(e: Event) {
    console.log('intro dialog canceled', e);
    this.backupCanceled_ = true;
  }

  onBackupPasswordObtained_(e: Event) {
    console.log('onBackupPasswordObtained_', e);
    const { detail } = e as CustomEvent;
    this.backupFilePassword_ = detail;
  }

  onPasswordPromptClosed_(e: Event) {
    this.showPasswordPromptDialog_ = false;
    console.log('password prompt closed', e);
    if (this.backupFilePassword_) {
      console.log('continue to select file');
      this.openSelectBackupFileDialog_();
    }
  }

  onPasswordPromptCanceled_(e: Event) {
    console.log('password prompt cancel', e);
  }

  // Restore functionality methods
  async checkRestoreSupported_() {
    sendWithPromise('jemaosRestoreSupported').then((supported) => {
      if (!supported) {
        this.showRestore_ = false;
        return;
      }
      this.showRestore_ = true;
    });
  }

  onRestoreClick_() {
    console.log('Restore button clicked, opening file dialog...');
    // Use Chrome's file picker for restore file selection
    chrome.send('jemaosRestoreSelectFile');
  }

  onRestoreFileSelected_(canceled: boolean) {
    console.log('onRestoreFileSelected_ called with canceled:', canceled);
    if (canceled) {
      console.log('Restore file selection canceled');
      return;
    }
    
    // Show password dialog instead of system prompt
    this.openRestorePasswordDialog_();
  }

  private openRestorePasswordDialog_() {
    console.log('Showing restore password dialog');
    
    // Create and show the password dialog
    const dialog = document.createElement('restore-password-dialog') as any;
    dialog.restoreFile = this.selectedRestoreFile_ || 'Selected backup file';
    document.body.appendChild(dialog);
    
    // Listen for password entry
    dialog.addEventListener('restore-password-entered', (event: any) => {
      console.log('Password entered from dialog');
      document.body.removeChild(dialog);
      this.startRestoreWithPassword_(event.detail.password);
    });
    
    // Listen for dialog cancel
    dialog.addEventListener('cr-dialog-cancel', () => {
      console.log('Restore password dialog canceled');
      document.body.removeChild(dialog);
    });
  }

  private startRestoreWithPassword_(password: string) {
    console.log('Starting restore operation with password, length:', password.length);
    
    this.restoreRunning_ = true;
    
    // Call the restore handler with the restore file path that was set in the backend
    sendWithPromise('restoreJemaOSBackup', password).then(
      (success: boolean) => {
        console.log('Restore operation completed with success:', success);
        this.restoreRunning_ = false;
        if (success) {
          this.showRestoreSuccessToast_();
        } else {
          this.showRestoreErrorToast_();
        }
      }
    ).catch((error: any) => {
      console.error('Restore operation failed with error:', error);
      this.restoreRunning_ = false;
      this.showRestoreErrorToast_();
    });
  }

  showRestoreSuccessToast_() {
    // Show success message
    console.log('Restore completed successfully');
    this.showRestoreResultDialog_(true, 'Backup restored successfully! Please restart Chrome for changes to take effect.');
  }

  showRestoreErrorToast_() {
    // Show error message
    console.log('Restore failed');
    this.showRestoreResultDialog_(false, 'Restore failed. Please check the backup file and password, then try again.');
  }

  private showRestoreResultDialog_(isSuccess: boolean, message: string) {
    const dialog = document.createElement('restore-result-dialog') as any;
    dialog.isSuccess = isSuccess;
    dialog.message = message;
    document.body.appendChild(dialog);
  }
}

customElements.define(
    JemaSettingsTweakUiPageElement.is, JemaSettingsTweakUiPageElement);
