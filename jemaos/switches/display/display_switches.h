// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_DISPLAY_SWITCHES_H_
#define CHROMEOS_JEMAOS_SWITCHES_DISPLAY_SWITCHES_H_

#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace switches {

CHROMEOS_EXPORT float GetDefaultDSF(float default_value);
CHROMEOS_EXPORT float GetDefaultScreenDpi(float default_value);

} // switches
} // jemaos

#endif /* ifndef CHROMEOS_JEMAOS_SWITCHES_DISPLAY_SWITCHES_H_ */
