// Copyright 2020 JemaOS Authors. All rights reserved.
// Author: yudong

import {JemaDropView} from './ui/jemadrop_view.js';
import {FileManagerUI} from './ui/file_manager_ui.js';
import {DirectoryModel} from './directory_model.js';

export class JemaDropViewController {
  private jemaDropView_: JemaDropView;
  private directoryModel_: DirectoryModel;
  private ui_: FileManagerUI;
  /**
   * @param {!JemaDropView} jemaDropView JemaDrop view.
   * @param {!DirectoryModel} directoryModel Directory model.
   */
  constructor(ui: FileManagerUI, jemaDropView: JemaDropView, directoryModel: DirectoryModel) {
    console.log('JemaDropViewController constructor');
    this.ui_ = ui;
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

  hideActionButtonsAndTable_() {
    console.log('hideActionButtons_');
    this.ui_.sortButton.setAttribute('hidden', '');
    this.ui_.toggleViewButton.setAttribute('hidden', '');
    this.ui_.listContainer.table.setAttribute('hidden', '');
  }

  showActionButtonsAndTable_() {
    console.log('showActionButtons_');
    this.ui_.sortButton.removeAttribute('hidden');
    this.ui_.toggleViewButton.removeAttribute('hidden');
    this.ui_.listContainer.table.removeAttribute('hidden');
  }

  onJemaDropStarted_() {
    console.log('onJemaDropStarted_');
    this.hideActionButtonsAndTable_();
    this.jemaDropView_.show();
  }

  onJemaDropStopped_() {
    console.log('onJemaDropStopped_');
    this.jemaDropView_.hide();
    this.showActionButtonsAndTable_();
  }
}
