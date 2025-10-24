// Copyright 2025 jema technology. All rights reserved.

/** @fileoverview Definitions for chrome.appManagement API */

declare namespace chrome {
  export namespace appManagement {
    export enum AppType {
      web_app = 'web_app',
      arc_app = 'arc_app',
    }

    export type AppId = string;

    export interface App {
      appId: AppId;
      appType: AppType;
      package?: string;
    }

    export function getAppList(callback: (applist: App[]) => void): void;
  }
}
