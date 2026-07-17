#ifndef ASH_WEBUI_JEMA_ASSISTANT_APP_UI_JEMA_ASSISTANT_UI_DELEGATE_H_
#define ASH_WEBUI_JEMA_ASSISTANT_APP_UI_JEMA_ASSISTANT_UI_DELEGATE_H_

#include <string>

#include "base/functional/callback.h"
#include "base/types/expected.h"
#include "base/values.h"
#include "third_party/skia/include/core/SkColor.h"

namespace content {
class WebUIDataSource;
}

namespace ash {

class JemaAssistantAppUIDelegate {
 public:
  struct ColorInfo {
    SkColor base;
    SkColor shaded;
    SkColor header;
    SkColor primary;
  };
  virtual ~JemaAssistantAppUIDelegate() = default;
  virtual void PopulateLoadTimeData(content::WebUIDataSource* source) = 0;

  virtual ColorInfo GetSystemColorInfo() = 0;

  using ToolResult = base::expected<base::Value::Dict, std::string>;
  using ToolCallback = base::OnceCallback<void(ToolResult)>;
  virtual void ExecuteSystemTool(const base::Value::Dict& request,
                                 ToolCallback callback) = 0;
  virtual void SaveApiKey(const std::string& provider,
                          const std::string& api_key,
                          ToolCallback callback) = 0;
  virtual void LoadApiKey(const std::string& provider,
                          ToolCallback callback) = 0;
};

}  // namespace ash

#endif  // !ASH_WEBUI_JEMA_ASSISTANT_APP_UI_JEMA_ASSISTANT_UI_DELEGATE_H_
