// Copyright 2020 JemaOS Authors. All rights reserved.
// Author: yudong

import {loadTimeData} from 'chrome://resources/ash/common/load_time_data.m.js';

export class JemaDropView {
  constructor(jemaDropView) {
    /**
     * @private {!HTMLElement}
     */
    this.jemaDropView_ = jemaDropView;

    // Download permission is denied by default in a webview
    this.jemaDropView_.addEventListener('permissionrequest', function(e) {
      if (e.permission === 'download') {
        e.request.allow();
      }
    });
  }

  show() {
    const webview = this.jemaDropView_.getElementsByTagName('webview')[0];
    if (!webview.src) {
      webview.src = loadTimeData.getString('JEMA_DROP_URL');
    }
    this.jemaDropView_.hidden = false;
  }

  hide() {
    this.jemaDropView_.hidden = true;
  }
}
