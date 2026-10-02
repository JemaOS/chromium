// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_BASE_AUTHENTICATION_METHOD_H_
#define REMOTING_BASE_AUTHENTICATION_METHOD_H_

#include <string>
#include <string_view>

namespace remoting {

// Method represents an authentication algorithm.
enum class AuthenticationMethod {
  INVALID,

  // SPAKE2 PIN or access code hashed with host_id using HMAC-SHA256.
  SHARED_SECRET_SPAKE2_CURVE25519,

  // SPAKE2 using shared pairing secret.
  PAIRED_SPAKE2_CURVE25519,

  // Authentication using the SessionAuthz service, which generates the
  // shared secret for SPAKE2 key exchange. This authz mode is used for Cloud
  // machines and is incompatible with other forms of SessionAuthz.
  CLOUD_SESSION_AUTHZ_SPAKE2_CURVE25519,

  // Authentication using the SessionAuthz service, which generates the
  // shared secret for SPAKE2 key exchange. This authz mode is used for Corp
  // machines and is incompatible with other forms of SessionAuthz.
  CORP_SESSION_AUTHZ_SPAKE2_CURVE25519,

  // JEMAOS: authentication using the access-code hash directly, without the
  // SPAKE2 key exchange. Enables the JemaOS web client (which cannot run
  // BoringSSL SPAKE2) to connect to the native host. Both peers derive the
  // same key from HMAC-SHA256(support_id, access_code); the channel is then
  // secured by SslHmacChannelAuthenticator.
  JEMAOS_HMAC_SHA256,

  // JEMAOS: SPAKE2 over Curve25519 for the JemaOS web client. Uses exactly the
  // same key derivation as SHARED_SECRET_SPAKE2_CURVE25519 (shared_secret_hash
  // = HMAC-SHA256(support_id, access_code)); the JemaOS web client performs the
  // SPAKE2 exchange in JavaScript. This is preferred over JEMAOS_HMAC_SHA256
  // when the client supports it, because it prevents offline guessing of the
  // low-entropy access code from a captured handshake.
  JEMAOS_SPAKE2_CURVE25519,
};

// Parses a string that defines an authentication method. Returns
// Method::INVALID if the string is invalid.
extern AuthenticationMethod ParseAuthenticationMethodString(
    std::string_view value);

// Returns string representation of |method|.
extern std::string AuthenticationMethodToString(AuthenticationMethod method);

}  // namespace remoting

#endif  // REMOTING_BASE_AUTHENTICATION_METHOD_H_
