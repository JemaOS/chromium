// Copyright 2024 The Jema Technology Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_MISC_JEMAOS_DEV_MODE_H_
#define JEMAOS_MISC_JEMAOS_DEV_MODE_H_

#include "base/functional/callback_forward.h"

namespace jemaos::misc {

bool IsDevModeSwitchSupported();
void SetDevMode(const bool enabled, base::OnceCallback<void(bool)> callback);
void GetDevMode(base::OnceCallback<void(bool)> callback);

}


#endif // !JEMAOS_MISC_JEMAOS_DEV_MODE_H_
