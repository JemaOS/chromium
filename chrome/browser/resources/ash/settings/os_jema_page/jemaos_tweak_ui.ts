// os-settings-jemaos-tweak-ui
import { PolymerElement, mixinBehaviors } from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import { sendWithPromise } from 'chrome://resources/js/cr.js';
import { I18nMixin } from 'chrome://resources/ash/common/cr_elements/i18n_mixin.js';
import { WebUiListenerMixin } from 'chrome://resources/ash/common/cr_elements/web_ui_listener_mixin.js';
import { CrDialogElement } from 'chrome://resources/ash/common/cr_elements/cr_dialog/cr_dialog.js';
import { LifetimeBrowserProxyImpl } from '/shared/settings/lifetime_browser_proxy.js';
import 'chrome://resources/ash/common/cr_elements/cr_toggle/cr_toggle.js';
import 'chrome://resources/ash/common/cr_elements/cr_dialog/cr_dialog.js';
import 'chrome://resources/ash/common/cr_elements/cr_shared_style.css.js';
import '../settings_shared.css.js';
import '../common/password_prompt_dialog/password_prompt_dialog.js';
import './components/backup_intro_dialog.js';
import './components/restore_password_dialog.js';
import './components/restore_result_dialog.js';
import { ShellClient } from './shell_client.js';

import { getTemplate } from './jemaos_tweak_ui.html.js';

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

interface JemaSettingsTweakUiPageElement {
  $: {
    widevineErrorDialog: CrDialogElement,
  };
}

const JemaSettingsTweakUIPageElementBase =
  WebUiListenerMixin(I18nMixin(PolymerElement));

class JemaSettingsTweakUiPageElement extends JemaSettingsTweakUIPageElementBase {
  static get is() {
    return 'os-settings-jemaos-tweak-ui' as const;
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
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
      showRestorePasswordDialog_: {
        type: Boolean,
        value: false,
      },
      showRestoreResultDialog_: {
        type: Boolean,
        value: false,
      },
      restoreRunning_: {
        type: Boolean,
        value: false,
      },
      restoreSuccess_: {
        type: Boolean,
        value: false,
      },
      restoreMessage_: {
        type: String,
        value: '',
      },
      cloudBackupRunning_: {
        type: Boolean,
        value: false,
      },
      cloudRestoreRunning_: {
        type: Boolean,
        value: false,
      },
      showCloudBackupResultDialog_: {
        type: Boolean,
        value: false,
      },
      cloudBackupSuccess_: {
        type: Boolean,
        value: false,
      },
      cloudBackupMessage_: {
        type: String,
        value: '',
      },
      authFactorHasPassword: Boolean,
      arcMediaAutoScanEnabled_: {
        type: Boolean,
        value: true,
      },
      savedArcMediaAutoScanEnabled_: {
        type: Number,
        value: -1,
      },
      rebootRequiredForToggleArcMediaAutoScan_: {
        type: Boolean,
        computed: 'computeRebootRequiredForToggleArcMediaAutoScan_(arcMediaAutoScanEnabled_, savedArcMediaAutoScanEnabled_)',
      },
    };
  }

  private showRotateScreenButton_: boolean;
  private isInTabletPhysicalState_: boolean;
  private showSwitchTabletLaptopButton_: boolean;
  private canToggleRotateScreenButton_: boolean;

  private arcMediaAutoScanEnabled_: boolean;
  private savedArcMediaAutoScanEnabled_: number;
  private rebootRequiredForToggleArcMediaAutoScan_: boolean;
  private togglingArcMediaAutoScan_: boolean;

  private showToggleWidevine_: boolean;
  private libwidevineEnabled_: boolean;
  private togglingWidevine_: boolean;

  private showBackup_: boolean;
  private backupRunning_: boolean;
  private showPasswordPromptDialog_: boolean;
  private showBackupIntroDialog_: boolean;
  private showRestorePasswordDialog_: boolean;
  private showRestoreResultDialog_: boolean;
  private restoreRunning_: boolean;
  private restoreSuccess_: boolean;
  private restoreMessage_: string;
  private cloudBackupRunning_: boolean;
  private cloudRestoreRunning_: boolean;
  private showCloudBackupResultDialog_: boolean;
  private cloudBackupSuccess_: boolean;
  private cloudBackupMessage_: string;
  private isCloudBackup_: boolean = false;
  private isCloudRestore_: boolean = false;
  private authFactorHasPassword: boolean;

  private client_: WidevineHelper;

  private backupEmail_: string;
  private backupFilePassword_: string;
  private backupCanceled_: boolean;
  private restoreFilePath_: string;
  private restorePassword_: string;
  private cloudRestoreFileKey_: string;

  private showWidevineErrorDialog_: boolean;
  private rebootRequiredForWidevine_: boolean;

  constructor() {
    super();
    this.client_ = new WidevineHelper(this);
    this.backupEmail_ = '';
    this.backupFilePassword_ = '';
    this.restoreFilePath_ = '';
    this.restorePassword_ = '';
    this.cloudRestoreFileKey_ = '';
    this.togglingArcMediaAutoScan_ = false;
  }

  override connectedCallback() {
    super.connectedCallback();
    this.getShowRotateScreenButton();
    this.getIsInTabletPhysicalState();
    this.getShowSwitchTabletLaptopButton();
    this.getArcMediaAutoScanEnabled();
    this.addWebUiListener('show-rotate-screen-button-changed', this.onShowRotateScreenButtonChanged_.bind(this));
    this.addWebUiListener('is-in-tablet-physical-state-changed', this.onIsInTabletPhysicalStateChanged_.bind(this));
    this.addWebUiListener('show-switch-tablet-laptop-button-changed', this.onShowSwitchTabletLaptopButtonChanged_.bind(this));
    this.addWebUiListener('jemaos-libwidevine-file-selected', this.onLibwidevineFileSelected_.bind(this));

    this.addWebUiListener('jemaos-backup-file-selected', this.onBackupFileSelected_.bind(this));
    this.addWebUiListener('jemaos-backup-task-finished', this.onBackupDone_.bind(this));

    this.addWebUiListener('jemaos-restore-file-selected', this.onRestoreFileSelected_.bind(this));
    this.addWebUiListener('jemaos-restore-task-finished', this.onRestoreDone_.bind(this));

    this.addWebUiListener('jemaos-cloud-backup-task-finished', this.onCloudBackupDone_.bind(this));
    this.addWebUiListener('jemaos-cloud-restore-task-finished', this.onCloudRestoreDone_.bind(this));

    this.addWebUiListener('jemaos-arc-media-auto-scan-changed', this.onArcMediaAutoScanStateChanged_.bind(this));

    this.checkLibwidevineStatus_();
    this.checkBackupSupported_();
  }

  getShowRotateScreenButton() {
    sendWithPromise('getShowRotateScreenButton').then((visible) => {
      this.showRotateScreenButton_ = visible;
    });
  }

  getIsInTabletPhysicalState() {
    sendWithPromise('getIsInTabletPhysicalState').then((enabled) => {
      this.isInTabletPhysicalState_ = enabled;
    });
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

  shouldShowRotateScreenButton_() {
    return this.showRotateScreenButton_ && this.canToggleRotateScreenButton_;
  }

  getShowRotateScreenButtonMessage_() {
    if (!this.isInTabletPhysicalState_) {
      return this.i18nAdvanced('notTabletPhysicalStateDisableJemaOsRotateScreen');
    }
    return this.i18nAdvanced('displayJemaOsRotateScreenButton');
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

  backupDisabled_() {
    return this.backupRunning_ || !this.authFactorHasPassword;
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
      if (this.isCloudBackup_) {
        console.log('starting cloud backup');
        this.isCloudBackup_ = false;
        this.startCloudBackup_();
      } else {
        console.log('continue to select file');
        this.openSelectBackupFileDialog_();
      }
    } else {
      this.isCloudBackup_ = false;
    }
  }

  onPasswordPromptCanceled_(e: Event) {
    console.log('password prompt cancel', e);
    this.isCloudBackup_ = false;
  }

  restoreDisabled_() {
    return this.restoreRunning_ || !this.authFactorHasPassword;
  }

  onRestoreClick_() {
    console.log('restore click');
    this.showRestorePasswordDialog_ = true;
  }

  onRestorePasswordObtained_(e: Event) {
    console.log('onRestorePasswordObtained_', e);
    const { detail } = e as CustomEvent;
    // Handle both old format (string) and new format (object with password and fileKey)
    if (typeof detail === 'string') {
      this.restorePassword_ = detail;
      this.cloudRestoreFileKey_ = '';
    } else {
      this.restorePassword_ = detail.password;
      this.cloudRestoreFileKey_ = detail.fileKey || '';
    }
  }

  onRestorePasswordDialogClosed_(e: Event) {
    this.showRestorePasswordDialog_ = false;
    console.log('restore password dialog closed', e);
    if (this.restorePassword_) {
      if (this.isCloudRestore_) {
        console.log('starting cloud restore');
        this.isCloudRestore_ = false;
        this.startCloudRestore_();
      } else {
        console.log('continue to select restore file');
        this.openSelectRestoreFileDialog_();
      }
    } else {
      this.isCloudRestore_ = false;
    }
  }

  onRestorePasswordDialogCanceled_(e: Event) {
    console.log('restore password dialog cancel', e);
    this.restorePassword_ = '';
    this.isCloudRestore_ = false;
  }

  openSelectRestoreFileDialog_() {
    chrome.send('jemaosRestoreSelectFile', []);
  }

  onRestoreFileSelected_(canceled: boolean, filePath: string) {
    if (canceled) {
      this.restorePassword_ = '';
      return;
    }
    this.restoreFilePath_ = filePath;
    this.startRestore_();
  }

  startRestore_() {
    this.restoreRunning_ = true;
    chrome.send('jemaosRestoreStarted', [this.backupEmail_, this.restorePassword_, this.restoreFilePath_]);
  }

  onRestoreDone_(success: boolean, message: string) {
    console.log('onRestoreDone_, result', success, message);
    this.restoreRunning_ = false;
    this.restoreSuccess_ = success;
    this.restoreMessage_ = message;
    this.showRestoreResultDialog_ = true;
  }

  onRestoreResultDialogClosed_(e: Event) {
    console.log('restore result dialog closed', e);
    this.showRestoreResultDialog_ = false;
    this.restorePassword_ = '';
    this.restoreFilePath_ = '';
  }

  // Cloud Backup handlers
  cloudBackupDisabled_() {
    return this.cloudBackupRunning_ || !this.authFactorHasPassword;
  }

  cloudRestoreDisabled_() {
    return this.cloudRestoreRunning_ || !this.authFactorHasPassword;
  }

  onCloudBackupClick_() {
    console.log('cloud backup click');
    this.isCloudBackup_ = true;
    this.showPasswordPromptDialog_ = true;
  }

  onCloudRestoreClick_() {
    console.log('cloud restore click');
    this.isCloudRestore_ = true;
    this.showRestorePasswordDialog_ = true;
  }

  startCloudBackup_() {
    this.cloudBackupRunning_ = true;
    chrome.send('jemaosCloudBackupStarted', [this.backupEmail_, this.backupFilePassword_]);
  }

  onCloudBackupDone_(success: boolean, message: string) {
    console.log('onCloudBackupDone_, result', success, message);
    this.cloudBackupRunning_ = false;
    this.cloudBackupSuccess_ = success;
    this.cloudBackupMessage_ = message;
    this.showCloudBackupResultDialog_ = true;
  }

  onCloudBackupResultDialogClosed_(e: Event) {
    console.log('cloud backup result dialog closed', e);
    this.showCloudBackupResultDialog_ = false;
    this.backupFilePassword_ = '';
  }

  startCloudRestore_() {
    this.cloudRestoreRunning_ = true;
    // Send the fileKey directly to the backend (it already contains the full path)
    chrome.send('jemaosCloudRestoreStarted', [this.backupEmail_, this.restorePassword_, this.cloudRestoreFileKey_]);
  }

  onCloudRestoreDone_(success: boolean, message: string) {
    console.log('onCloudRestoreDone_, result', success, message);
    this.cloudRestoreRunning_ = false;
    this.restoreSuccess_ = success;
    this.restoreMessage_ = message;
    this.showRestoreResultDialog_ = true;
  }

  getArcMediaAutoScanEnabled() {
    sendWithPromise('getArcMediaAutoScanState').then((result: { enabled: boolean, saved: number }) => {
      const { enabled, saved } = result;
      console.log('getArcMediaAutoScanState', result);
      this.arcMediaAutoScanEnabled_ = enabled;
      this.savedArcMediaAutoScanEnabled_ = saved;
      if (saved === -1) {
        chrome.send('setArcMediaAutoScanStateForCurrentSession', [enabled]);
      }
    });
  }

  async onToggleArcMediaAutoScan_() {
    if (this.togglingArcMediaAutoScan_) {
      return;
    }
    this.togglingArcMediaAutoScan_ = true;
    this.arcMediaAutoScanEnabled_ = !this.arcMediaAutoScanEnabled_;
    chrome.send('setArcMediaAutoScanState', [this.arcMediaAutoScanEnabled_]);
    this.togglingArcMediaAutoScan_ = false;
  }

  onArcMediaAutoScanStateChanged_(result: { enabled: boolean, saved: number }) {
    const { enabled, saved } = result;
    console.log('onArcMediaAutoScanStateChanged_', result);
    this.arcMediaAutoScanEnabled_ = enabled;
    this.savedArcMediaAutoScanEnabled_ = saved;
  }

  computeRebootRequiredForToggleArcMediaAutoScan_(enabled: boolean, saved: number) {
    if (saved === -1) return false;
    return enabled !== (!!saved);
  }

  onRestartForToggleArcMediaAutoScanTap_() {
    LifetimeBrowserProxyImpl.getInstance().relaunch();
  }
}

customElements.define(
  JemaSettingsTweakUiPageElement.is, JemaSettingsTweakUiPageElement);
