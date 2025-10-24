// os-settings-jema-page

import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import 'chrome://resources/ash/common/cr_elements/cr_shared_style.css.js';

import {I18nMixin} from 'chrome://resources/ash/common/cr_elements/i18n_mixin.js';
import {Router, routes} from '../router.js';
import {RouteOriginMixin} from '../common/route_origin_mixin.js';
import {Section} from '../mojom-webui/routes.mojom-webui.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import '../os_settings_page/settings_card.js';
import '../os_settings_page/os_settings_subpage.js';
import '../settings_shared.css.js';
import './jemaos_account.js';
// import './jemaos_drivers.js';
import './jemaos_remoting.js';
import './jemaos_tweak_ui.js';
import './jemaos_experiment.js';
import './jemaos_more_info.js';
import './jemaos_dev_mode.js';
// <if expr="use_jemaos_license">
import './jemaos_license_info.js';
// </if>

import {getTemplate} from './os_jema_page.html.js';

const OsSettingsJemaPageElementBase = 
  RouteOriginMixin(I18nMixin(PolymerElement));

/** @polymer */
class OsSettingsJemaPageElement extends OsSettingsJemaPageElementBase {
  static get is() {
    return 'os-jema-page' as const;
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      section_: {
        type: Number,
        value: Section.kJemaOs,
        readOnly: true,
      },
      isJemaAccount_: {
        type: Boolean,
        value() {
          return loadTimeData.getBoolean('isJemaProfile');
        },
      },
      isGuest_: {
        type: Boolean,
        value() {
          return loadTimeData.getBoolean('isGuest');
        },
      },

// <if expr="use_jemaos_license">
      showJemaOsLicense_: {
        type: Boolean,
        value() {
          return loadTimeData.valueExists('showJemaOsLicense') && loadTimeData.getBoolean('showJemaOsLicense');
        },
      },
// </if>

      showToggleRebootButtonInTray: {
        type: Boolean,
        value() {
          // some specified board cannot reboot
          const ret = loadTimeData.getBoolean('showToggleRebootButtonInTray');
          console.log('showToggleRebootButtonInTray', ret);
          return ret;
        },
      },
      showToggleRotateScreenButton: {
        type: Boolean,
        value() {
          // only amd64-jemaos/amd64-generic shouldShowToggleShowRotateScreenButton is true
          return loadTimeData.getBoolean('showToggleRotateScreenButton');
        },
      },
      showToggleSwitchTabletLaptopButton: {
        type: Boolean,
        value() {
          return loadTimeData.getBoolean('showToggleSwitchTabletLaptopButton');
        },
      },

      authFactorHasPassword_: Boolean,
    };
  }

  private isJemaAccount_: boolean;
  private isGuest_: boolean;
  private showJemaOsLicense_: boolean;
  private showToggleRebootButtonInTray: boolean;
  private showToggleRotateScreenButton: boolean;
  private showToggleSwitchTabletLaptopButton: boolean;
  private authFactorHasPassword_: boolean;

  constructor() {
    super();
    this.route = routes.JEMAOS;
    console.log('os-settings-jema-page ready');
    // <if expr="_google_chrome">
    console.log('this is a test for build tools')
    // </if>
  }

  override connectedCallback() {
    super.connectedCallback();
    console.log('os-settings-jema-page attached');
  }

  showJemaOsTweakUi_() {
    return this.showToggleRotateScreenButton || this.showToggleRebootButtonInTray || this.showToggleSwitchTabletLaptopButton;
  }

  onAuthFactorHasPasswordChanged_(e: CustomEvent) {
    this.authFactorHasPassword_ = e.detail;
  }

// <if expr="use_jemaos_license">
  onLicenseInfoClick_() {
    if (!this.showJemaOsLicense_) return;
    Router.getInstance().navigateTo(routes.JEMAOS_LICENSE_INFO);
  }
// </if>
}

customElements.define(
    OsSettingsJemaPageElement.is, OsSettingsJemaPageElement);
