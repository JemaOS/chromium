// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/shell_state.h"

#include "base/format_macros.h"
#include "base/strings/stringprintf.h"

namespace jemaos {

namespace ash {

// Constructor for ShellState
ShellState::ShellState()
    : code(0) {
}

// Converts the ShellState object to a string representation
std::string ShellState::ToString() const {
  std::string result_s;
  base::StringAppendF(&result_s,
                      "code = %d ",
                      code);
  base::StringAppendF(&result_s,
                      "result = %s ",
                      result.c_str());
  return result_s;
}

}  // namespace ash

}  // namespace jemaos