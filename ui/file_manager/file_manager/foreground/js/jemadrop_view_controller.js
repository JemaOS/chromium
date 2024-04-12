// Copyright 2020 JemaOS Authors. All rights reserved.
// Author: yudong

import {JemaDropView} from './ui/jemadrop_view.js';
import {DirectoryModel} from './directory_model.js';

export class JemaDropViewController {
  /**
   * @param {!JemaDropView} jemaDropView JemaDrop view.
   * @param {!DirectoryModel} directoryModel Directory model.
   */
  constructor(jemaDropView, directoryModel) {
    console.log('JemaDropViewController constructor');
    /**
     * @private {!JemaDropView}
     */
    this.jemaDropView_ = jemaDropView;

    /**
     * @private {!DirectoryModel}
     */
    this.directoryModel_ = directoryModel;

    this.directoryModel_.addEventListener(
        'jemadrop-started', this.onJemaDropStarted_.bind(this));
    this.directoryModel_.addEventListener(
        'jemadrop-stopped', this.onJemaDropStopped_.bind(this));
  }

  hideActionButtons_() {
    console.log('hideActionButtons_');
    fileManager.ui.sortButton.setAttribute('hidden', '');
    fileManager.ui.toggleViewButton.setAttribute('hidden', '');
  }

  showActionButtons_() {
    console.log('showActionButtons_');
    fileManager.ui.sortButton.removeAttribute('hidden');
    fileManager.ui.toggleViewButton.removeAttribute('hidden');
  }

  onJemaDropStarted_() {
    console.log('onJemaDropStarted_');
    this.hideActionButtons_();
    this.jemaDropView_.show();
  }

  onJemaDropStopped_() {
    console.log('onJemaDropStopped_');
    this.jemaDropView_.hide();
    this.showActionButtons_();
  }
}
