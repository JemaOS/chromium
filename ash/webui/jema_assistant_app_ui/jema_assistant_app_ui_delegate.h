#ifndef ASH_WEBUI_JEMA_ASSISTANT_APP_UI_JEMA_ASSISTANT_UI_DELEGATE_H_
#define ASH_WEBUI_JEMA_ASSISTANT_APP_UI_JEMA_ASSISTANT_UI_DELEGATE_H_

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
};

} // namespace ash

#endif // !ASH_WEBUI_JEMA_ASSISTANT_APP_UI_JEMA_ASSISTANT_UI_DELEGATE_H_
