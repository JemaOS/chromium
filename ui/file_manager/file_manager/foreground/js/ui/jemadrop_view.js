// Copyright 2025 Jema Technology. All rights reserved.

import {loadTimeData} from 'chrome://resources/ash/common/load_time_data.m.js';

/**
 * Represents the JemaDrop view.
 * NOTE FOR DEVELOPERS: This class manages the JemaDrop UI component, handling
 * visibility and permissions for the embedded webview.
 */
export class JemaDropView {
  /**
   * @param {!HTMLElement} jemaDropView The root element of the JemaDrop view.
   */
  constructor(jemaDropView) {
    /**
     * @private {!HTMLElement}
     * The root element of the JemaDrop view.
     */
    this.jemaDropView_ = jemaDropView;

    // Download permission is denied by default in a webview.
    // NOTE FOR DEVELOPERS: This listener explicitly allows download requests
    // made within the webview.
    this.jemaDropView_.addEventListener('permissionrequest', function(e) {
      if (e.permission === 'download') {
        e.request.allow();
      }
    });
  }

  /**
   * Shows the JemaDrop view.
   * NOTE FOR DEVELOPERS: If the webview source is not set, it initializes the
   * webview with the JemaDrop URL from loadTimeData.
   */
  show() {
    const webview = this.jemaDropView_.getElementsByTagName('webview')[0];
    if (!webview.src) {
      webview.src = loadTimeData.getString('JEMA_DROP_URL');
    }
    this.jemaDropView_.hidden = false;
  }

  /**
   * Hides the JemaDrop view.
   * NOTE FOR DEVELOPERS: This method simply sets the hidden property to true.
   */
  hide() {
    this.jemaDropView_.hidden = true;
  }
}