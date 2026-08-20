// Copyright (c) 2026 The Jema OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/misc/jemaos_device_uid.h"

#include <openssl/sha.h>

#include "base/files/file_util.h"
#include "base/guid.h"
#include "base/hash/hash.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "chromeos/ash/components/system/statistics_provider.h"
#include "components/prefs/pref_service.h"
#include "jemaos/prefs/jemaos_pref_names.h"

namespace jemaos {
namespace {

// Sel de domaine : l UID ne peut pas etre recoupe avec le machine-id ou le
// numero de serie brut utilises par une autre application.
constexpr char kUidSalt[] = "JemaOS-DeviceUID-v1:";

std::string HashDeviceUid(const std::string& raw) {
  const std::string input = kUidSalt + raw;
  uint8_t digest[SHA256_DIGEST_LENGTH];
  SHA256(reinterpret_cast<const uint8_t*>(input.data()), input.size(), digest);
  return base::HexEncode(digest, sizeof(digest));
}

}  // namespace

std::string GetDeviceUid(PrefService* local_state) {
  std::string raw;

  // 1. ChromeOS VPD (vrais Chromebooks).
  if (auto* provider = ash::system::StatisticsProvider::GetInstance()) {
    if (auto id = provider->GetMachineID()) {
      raw = *id;
    }
  }

  // 2. UID firmware (DMI) pre-hache au boot par le service root
  //    (/etc/init/jemaos-device-uid.conf) : survit a la reinstallation,
  //    car le DMI vit dans le firmware. Le fichier contient deja le hash
  //    final : le retourner tel quel, sans re-hacher.
  if (raw.empty()) {
    std::string content;
    if (base::ReadFileToString(base::FilePath("/var/lib/jemaos/device_uid"),
                               &content)) {
      std::string prehashed =
          std::string(base::TrimWhitespaceASCII(content, base::TRIM_ALL));
      if (prehashed.size() == 64) {
        return prehashed;
      }
    }
  }

  // 3. machine-id D-Bus/systeme : stable par installation, lisible par
  //    l utilisateur chrome, disponible sur tout PC generique.
  if (raw.empty()) {
    for (const char* path : {"/var/lib/dbus/machine-id", "/etc/machine-id"}) {
      std::string content;
      if (base::ReadFileToString(base::FilePath(path), &content)) {
        raw = std::string(base::TrimWhitespaceASCII(content, base::TRIM_ALL));
        if (!raw.empty()) {
          break;
        }
      }
    }
  }

  // 4. Dernier recours : UUID aleatoire persiste dans local state (survit
  //    aux reboots).
  if (raw.empty() && local_state) {
    raw = local_state->GetString(prefs::kJemaOsDeviceUid);
    if (raw.empty()) {
      raw = base::GenerateGUID();
      local_state->SetString(prefs::kJemaOsDeviceUid, raw);
      local_state->CommitPendingWrite();
    }
  }

  if (raw.empty()) {
    LOG(ERROR) << "GetDeviceUid: aucune source d identite disponible";
    return std::string();
  }
  return HashDeviceUid(raw);
}

}  // namespace jemaos