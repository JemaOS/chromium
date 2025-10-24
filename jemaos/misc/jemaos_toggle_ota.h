#ifndef JEMAOS_TOGGLE_OTA_H_
#define JEMAOS_TOGGLE_OTA_H_

#include "base/functional/callback_forward.h"

namespace jemaos {
namespace misc {
  void EnableJemaOTA(const bool enabled, base::OnceCallback<void()> callback);
  bool GetEnabledJemaOTA();
} // misc

} // jemaos
#endif /* ifndef JEMAOS_TOGGLE_OTA_H_ */
