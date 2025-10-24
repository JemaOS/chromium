// @ts-nocheck
// Copyright 2020 JemaOS Authors. All rights reserved.
// Author: yudong

import {loadTimeData} from 'chrome://resources/ash/common/load_time_data.m.js';

export class JemaDropView {
  /**
  * @param {!HTMLElement} element
  */
  constructor(element) {
    /**
     * @private {!HTMLElement}
     */
    this.jemaDropView_ = element;

    // Download permission is denied by default in a webview
    // @ts-ignore TS7006: Parameter 'element' implicitly has an 'any' type.
    this.jemaDropView_.addEventListener('permissionrequest', function(e) {
      if (e.permission === 'download') {
        e.request.allow();
      }
    });

    this.loaded_ = false;
    this.setUrl();
  }

  setUrl() {
    const webview = this.jemaDropView_.getElementsByTagName('webview')[0];
    if (webview && !webview.src) {
      this.jemaDropView_.addEventListener(
        "contentload",
        this.onContentLoad_.bind(this),
      );
      webview.src = loadTimeData.getString("JEMA_DROP_URL");
    }
  }

  show() {
    if (!this.loaded_) {
      this.hideSpinnerCallback_ =
        window.fileManager.spinnerController.showWithDelay(
          100,
          this.onSpinnerShow_.bind(this),
        );
    }
    this.setUrl();
    this.jemaDropView_.hidden = false;
  }

  hide() {
    this.jemaDropView_.hidden = true;
    this.hideSpinner_();
  }

  onContentLoad_() {
    this.loaded_ = true;
    this.hideSpinner_();
  }

  onSpinnerShow_() {}

  hideSpinner_() {
    if (this.hideSpinnerCallback_) {
      this.hideSpinnerCallback_();
      this.hideSpinnerCallback_ = null;
    }
  }
}
