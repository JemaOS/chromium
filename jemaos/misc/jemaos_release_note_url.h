// Copyright (c) 2021 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _JEMAOS_MISC_JEMAOS_RELEASE_NOTE_URL_H_
#define _JEMAOS_MISC_JEMAOS_RELEASE_NOTE_URL_H_

#include <string>

namespace jemaos {
namespace misc {

const std::string BuildJemaReleaseNoteUrlWithPath(bool staging = false);

}
}

#endif  // _JEMAOS_MISC_JEMAOS_RELEASE_NOTE_URL_H_
