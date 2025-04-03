// Copyright (c) 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _JEMAOS_MISC_JEMAOS_RELEASE_NOTE_URL_H_
#define _JEMAOS_MISC_JEMAOS_RELEASE_NOTE_URL_H_

#include <string>

class Profile;

namespace jemaos {
namespace misc {

const std::string BuildJemaReleaseNoteUrlWithPath(Profile* profile);

}
}

#endif  // _JEMAOS_MISC_JEMAOS_RELEASE_NOTE_URL_H_
