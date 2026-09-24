// Copyright 2026 jema technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/ash/login/jema_password_validator.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/values.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/net/system_network_context_manager.h"
#include "components/prefs/pref_service.h"
#include "jemaos/prefs/jemaos_pref_names.h"
#include "net/base/net_errors.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"

namespace ash {
namespace {

constexpr char kJemaOsConnectApiBase[] = "https://connect-api.jematech.fr";
// Pure password check (no hardware/license resolution), safe from the pod.
constexpr char kJemaOsVerifyPasswordPath[] = "/v1/connect/verify-password";
// Same public app key as the in-session token refresh (user_session_manager).
constexpr char kJemaOsConnectApiKey[] = "e58492a3-b452-4197-9f4a-deb7915b9446";

const net::NetworkTrafficAnnotationTag kJemaPasswordValidatorAnnotation =
    net::DefineNetworkTrafficAnnotation("jema_password_validator", R"jema(
        semantics {
          sender: "JemaOS login"
          description:
            "Validates the password typed on the login screen against the Jema
            Connect API, so a password changed in the SaaS is applied without
            changing the login flow. No data is stored."
          trigger: "User types their password on the Jema login screen."
          data: "Account email, password and this device's stable UID."
          destination: OTHER
          destination_other: "Jema Connect API (connect-api.jematech.fr)"
        }
        policy {
          cookies_allowed: NO
          setting: "This feature cannot be disabled."
        })jema");

void OnResponse(std::unique_ptr<network::SimpleURLLoader> loader,
                base::OnceCallback<void(bool)> callback,
                std::unique_ptr<std::string> response_body) {
  const int net_error = loader ? loader->NetError() : net::ERR_FAILED;
  const std::string body = response_body ? *response_body : std::string();
  bool ok = false;
  std::optional<base::Value> value = base::JSONReader::Read(body);
  if (net_error == net::OK && value && value->is_dict()) {
    ok = value->GetDict().FindBool("valid").value_or(false);
  }
  LOG(WARNING) << "[JEMAOS] Password verification online: "
               << (ok ? "OK" : "refused") << " (net_error=" << net_error
               << ", body=" << body << ")";
  std::move(callback).Run(ok);
}

}  // namespace

// static
void JemaPasswordValidator::Validate(const std::string& email,
                                     const std::string& password,
                                     base::OnceCallback<void(bool)> callback) {
  if (email.empty() || password.empty() || !g_browser_process ||
      !g_browser_process->system_network_context_manager()) {
    std::move(callback).Run(false);
    return;
  }

  base::Value::Dict body;
  body.Set("email", email);
  body.Set("password", password);
  std::string body_json;
  base::JSONWriter::Write(body, &body_json);

  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url =
      GURL(std::string(kJemaOsConnectApiBase) + kJemaOsVerifyPasswordPath);
  resource_request->method = "POST";
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
  resource_request->headers.SetHeader("Content-Type", "application/json");
  resource_request->headers.SetHeader("x-api-key", kJemaOsConnectApiKey);

  auto loader = network::SimpleURLLoader::Create(
      std::move(resource_request), kJemaPasswordValidatorAnnotation);
  loader->SetAllowHttpErrorResults(true);
  loader->AttachStringForUpload(body_json, "application/json");
  auto* loader_ptr = loader.get();
  loader_ptr->DownloadToString(
      g_browser_process->system_network_context_manager()
          ->GetSharedURLLoaderFactory()
          .get(),
      base::BindOnce(&OnResponse, std::move(loader), std::move(callback)),
      256 * 1024);
}

}  // namespace ash
