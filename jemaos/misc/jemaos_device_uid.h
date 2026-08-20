// Copyright (c) 2026 The Jema OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_MISC_JEMAOS_DEVICE_UID_H_
#define JEMAOS_MISC_JEMAOS_DEVICE_UID_H_

#include <string>

class PrefService;

namespace jemaos {

// Retourne un identifiant unique et stable par appareil, respectueux de la
// vie privee : le resultat est un hash SHA-256, le numero de serie brut ne
// quitte jamais la machine. Ordre de priorite :
//   1. machine-id VPD ChromeOS (vrais Chromebooks / appareils avec VPD) ;
//   2. /var/lib/dbus/machine-id ou /etc/machine-id (stable par installation,
//      lisible par l utilisateur chrome) ;
//   3. UUID aleatoire persiste dans local state (pref jemaos.device_uid).
std::string GetDeviceUid(PrefService* local_state);

}  // namespace jemaos

#endif  // JEMAOS_MISC_JEMAOS_DEVICE_UID_H_