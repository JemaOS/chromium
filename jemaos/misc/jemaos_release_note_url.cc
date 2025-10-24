// Copyright (c) 2021 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "jemaos/misc/jemaos_release_note_url.h"
#include "base/strings/strcat.h"
#include "base/system/sys_info.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/browser_process.h"
#include "jemaos/switches/urls/urls_constants.h"

namespace jemaos::misc {

namespace {
/*
const std::string trim_suffix(const std::string& str) {
  if (str.size() < 4) {
    return str;
  }
  if (str.substr(str.size() - 4) == "-com") {
    return str.substr(0, str.size() - 4);
  }
  if (str.substr(str.size() - 3) == "-io") {
    return str.substr(0, str.size() - 3);
  }
  return str;
}

const std::string ReleaseNoteUrlWithVersionAndBoard() {
  const std::string version = base::SysInfo::GetLsbJemaReleaseVersion();
  const std::string board_name = base::SysInfo::GetLsbReleaseBoard();
  const std::string language = g_browser_process->GetApplicationLocale();
  const std::string url = base::StrCat(
      {jemaos::constants::kJemaOSReleaseNotesURL,
      "/", version, "/", trim_suffix(board_name)});
  return url;
}
*/

const std::string JemaOSNewsURL() {
  return jemaos::constants::kJemaOSNewsURL;
}

}  // namespace

const std::string BuildJemaReleaseNoteUrlWithPath(bool staging) {
  return JemaOSNewsURL();
}

}  // namespace jemaos::misc
