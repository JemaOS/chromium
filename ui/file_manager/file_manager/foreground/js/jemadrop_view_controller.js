// Copyright 2025 Jema Technology. All rights reserved.
// Author: yudong

import {JemaDropView} from './ui/jemadrop_view.js';
import {DirectoryModel} from './directory_model.js';

/**
 * Controller for managing the JemaDrop view.
 * NOTE FOR DEVELOPERS: This class handles the interaction between the JemaDrop
 * view and the directory model, ensuring proper UI updates during JemaDrop events.
 */
export class JemaDropViewController {
  /**
   * @param {!JemaDropView} jemaDropView JemaDrop view instance.
   * @param {!DirectoryModel} directoryModel Directory model instance.
   */
  constructor(jemaDropView, directoryModel) {
    console.log('JemaDropViewController constructor');

    /**
     * @private {!JemaDropView}
     * The JemaDrop view instance managed by this controller.
     */
    this.jemaDropView_ = jemaDropView;

    /**
     * @private {!DirectoryModel}
     * The directory model instance providing data for the JemaDrop view.
     */
    this.directoryModel_ = directoryModel;

    // Register event listeners for JemaDrop events.
    // NOTE FOR DEVELOPERS: Ensure these events are properly dispatched by the directory model.
    this.directoryModel_.addEventListener(
        'jemadrop-started', this.onJemaDropStarted_.bind(this));
    this.directoryModel_.addEventListener(
        'jemadrop-stopped', this.onJemaDropStopped_.bind(this));
  }

  /**
   * Hides action buttons in the file manager UI.
   * NOTE FOR DEVELOPERS: This method modifies the visibility of UI elements
   * related to sorting and view toggling.
   * @private
   */
  hideActionButtons_() {
    console.log('hideActionButtons_');
    fileManager.ui.sortButton.setAttribute('hidden', '');
    fileManager.ui.toggleViewButton.setAttribute('hidden', '');
  }

  /**
   * Shows action buttons in the file manager UI.
   * NOTE FOR DEVELOPERS: This method restores the visibility of UI elements
   * related to sorting and view toggling.
   * @private
   */
  showActionButtons_() {
    console.log('showActionButtons_');
    fileManager.ui.sortButton.removeAttribute('hidden');
    fileManager.ui.toggleViewButton.removeAttribute('hidden');
  }

  /**
   * Handles the "jemadrop-started" event.
   * NOTE FOR DEVELOPERS: This method is triggered when a JemaDrop operation starts.
   * It hides action buttons and displays the JemaDrop view.
   * @private
   */
  onJemaDropStarted_() {
    console.log('onJemaDropStarted_');
    this.hideActionButtons_();
    this.jemaDropView_.show();
  }

  /**
   * Handles the "jemadrop-stopped" event.
   * NOTE FOR DEVELOPERS: This method is triggered when a JemaDrop operation stops.
   * It hides the JemaDrop view and restores the action buttons.
   * @private
   */
  onJemaDropStopped_() {
    console.log('onJemaDropStopped_');
    this.jemaDropView_.hide();
    this.showActionButtons_();
  }
}