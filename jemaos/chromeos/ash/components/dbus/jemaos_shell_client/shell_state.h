// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_DBUS_SHELL_STATE_H_
#define JEMAOS_DBUS_SHELL_STATE_H_

#include <stdint.h>

#include <string>

#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace ash {

// Represents the state of the JemaOS Shell
struct CHROMEOS_EXPORT ShellState {
  int32_t code;           // The status code of the shell operation
  std::string result;     // The result of the shell operation

  // Constructor
  ShellState();

  // Converts the ShellState object to a string representation
  std::string ToString() const;
};

}  // namespace ash
}  // namespace jemaos

#endif  // JEMAOS_DBUS_SHELL_STATE_H_