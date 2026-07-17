#ifndef CHROME_BROWSER_ASH_WEB_APPLICATIONS_JEMA_ASSISTANT_APP_UI_DELEGATE_H_
#define CHROME_BROWSER_ASH_WEB_APPLICATIONS_JEMA_ASSISTANT_APP_UI_DELEGATE_H_

#include <map>

#include "ash/webui/jema_assistant_app_ui/jema_assistant_app_ui_delegate.h"
#include "base/files/file_path.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/ref_counted_memory.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "content/public/browser/weak_document_ptr.h"
#include "ui/accessibility/ax_action_data.h"
#include "ui/accessibility/ax_tree_update.h"
#include "ui/aura/window_tracker.h"
#include "url/gurl.h"

namespace content {
class WebUI;
class WebContents;
}  // namespace content

class ChromeJemaAssistantAppUIDelegate
    : public ash::JemaAssistantAppUIDelegate {
 public:
  explicit ChromeJemaAssistantAppUIDelegate(content::WebUI* web_ui);

  ChromeJemaAssistantAppUIDelegate(const ChromeJemaAssistantAppUIDelegate&) =
      delete;
  ChromeJemaAssistantAppUIDelegate& operator=(
      const ChromeJemaAssistantAppUIDelegate&) = delete;
  ~ChromeJemaAssistantAppUIDelegate() override;

  // JemaAssistantAppUIDelegate:
  void PopulateLoadTimeData(content::WebUIDataSource* source) override;

  ash::JemaAssistantAppUIDelegate::ColorInfo GetSystemColorInfo() override;
  void ExecuteSystemTool(const base::Value::Dict& request,
                         ToolCallback callback) override;
  void SaveApiKey(const std::string& provider,
                  const std::string& api_key,
                  ToolCallback callback) override;
  void LoadApiKey(const std::string& provider, ToolCallback callback) override;

 private:
  void ListApps(ToolCallback callback);
  void LaunchApp(const base::Value::Dict& request, ToolCallback callback);
  void OpenQuickText(const base::Value::Dict& request, ToolCallback callback);
  void GetLocale(ToolCallback callback);
  void ChangeLocale(const base::Value::Dict& request, ToolCallback callback);
  void ListDisplayModes(ToolCallback callback);
  void SetDisplayMode(const base::Value::Dict& request, ToolCallback callback);
  void SearchFiles(const base::Value::Dict& request, ToolCallback callback);
  void ReadFile(const base::Value::Dict& request, ToolCallback callback);
  void WriteFile(const base::Value::Dict& request, ToolCallback callback);
  void GetAudioState(ToolCallback callback);
  void SetAudioState(const base::Value::Dict& request, ToolCallback callback);
  void GetPowerState(ToolCallback callback);
  void ShowNotification(const base::Value::Dict& request,
                        ToolCallback callback);
  void ComposeEmail(const base::Value::Dict& request, ToolCallback callback);
  void GetWifiState(ToolCallback callback);
  void SetWifiState(const base::Value::Dict& request, ToolCallback callback);
  void GetBluetoothState(ToolCallback callback);
  void SetBluetoothState(const base::Value::Dict& request,
                         ToolCallback callback);
  void GetActiveWindowContext(ToolCallback callback);
  void OpenBrowserUrl(const base::Value::Dict& request, ToolCallback callback);
  void CaptureActiveWindow(const base::Value::Dict& request,
                           ToolCallback callback);
  void GetActiveAccessibilityTree(const base::Value::Dict& request,
                                  ToolCallback callback);
  void PerformAccessibilityAction(const base::Value::Dict& request,
                                  ToolCallback callback);
  void OnActiveWindowCaptured(ToolCallback callback,
                              std::u16string title,
                              scoped_refptr<base::RefCountedMemory> png);
  struct AccessibilitySnapshotRequest;
  void RequestFrameAccessibilitySnapshot(
      std::shared_ptr<AccessibilitySnapshotRequest> request,
      content::RenderFrameHost* frame);
  void OnFrameAccessibilityTreeSnapshot(
      std::shared_ptr<AccessibilitySnapshotRequest> request,
      content::WeakDocumentPtr document,
      ui::AXTreeID tree_id,
      ui::AXTreeUpdate& update);
  void CompleteAccessibilitySnapshot(
      std::shared_ptr<AccessibilitySnapshotRequest> request);
  void GetNativeAccessibilityTree(aura::Window* window, ToolCallback callback);
  aura::Window* GetTargetUserWindow() const;
  content::WebContents* GetWebContentsForWindow(aura::Window* window) const;

  std::optional<base::FilePath> ResolveUserPath(
      const base::Value::Dict& request,
      std::string* error) const;
  void AppendAuditEntry(const base::Value::Dict& request,
                        bool success,
                        std::string_view detail);
  std::string GetPermission(const base::Value::Dict& request) const;

  std::optional<std::string> ResolveAppId(const std::string& query,
                                          std::string* error) const;

  raw_ptr<content::WebUI> web_ui_;  // Owns |this|.
  base::WeakPtrFactory<ChromeJemaAssistantAppUIDelegate> weak_ptr_factory_{
      this};

  struct ActionToken {
    int32_t node_id = ui::kInvalidAXNodeID;
    ui::AXTreeID tree_id;
    content::WeakDocumentPtr document;
    base::TimeTicks expires_at;
    bool can_focus = false;
    bool can_activate = false;
    bool can_set_value = false;
    bool native_views = false;
    raw_ptr<aura::Window> native_window = nullptr;
  };
  std::map<std::string, ActionToken> action_tokens_;
};

#endif  // !#ifndef
        // CHROME_BROWSER_ASH_WEB_APPLICATIONS_JEMA_ASSISTANT_APP_UI_DELEGATE_H_
