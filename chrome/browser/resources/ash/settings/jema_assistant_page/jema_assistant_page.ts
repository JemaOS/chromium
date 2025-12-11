import 'chrome://resources/ash/common/cr_elements/cr_link_row/cr_link_row.js';
import '../controls/settings_toggle_button.js';
import '../os_settings_page/settings_card.js';
import '../settings_shared.css.js';

import {PrefsMixin} from '/shared/settings/prefs/prefs_mixin.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {Section} from '../mojom-webui/routes.mojom-webui.js';
import {isJemaAiHidden} from '../common/load_time_booleans.js';

import {getTemplate} from './jema_assistant_page.html.js';

const OsSettingsJemaAssistantPageElementBase = PrefsMixin(PolymerElement);

export class OsSettingsJemaAssistantPageElement extends OsSettingsJemaAssistantPageElementBase {
  static get is() {
    return 'os-settings-jema-assistant-page';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      section_: {
        type: Number,
        value: Section.kJemaAssistant,
        readOnly: true,
      },

      isJemaAiHidden_: {
        type: Boolean,
        value: () => isJemaAiHidden(),
        readOnly: true,
      },
    };
  }

  // constructor() {
  //   super();
  // }
  //
  // override ready() {
  //   super.ready();
  // }
  //
  // override connectedCallback() {
  //   super.connectedCallback();
  // }
  onSettingsClicked_() {
    let enabled = false;
    try {
       enabled = this.getPref<boolean>('jema_assistant_enabled').value;
    } catch (e) {
      console.error('Get pref jema_assistant_enabled error', e);
    }

    if (!enabled) return;
    window.open('chrome://jemaos-ai/settings');
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'os-settings-jema-assistant-page': OsSettingsJemaAssistantPageElement;
  }
}

customElements.define(OsSettingsJemaAssistantPageElement.is, OsSettingsJemaAssistantPageElement);
