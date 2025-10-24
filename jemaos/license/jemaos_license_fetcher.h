// Copyright 2020 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_LICENSE_FETCHER_H_
#define JEMAOS_LICENSE_FETCHER_H_
#include <memory>
#include <string>
#include "base/values.h"
/*
#include "base/timer/timer.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_fetcher_delegate.h"
#include "net/url_request/url_request_context_getter.h"
*/
#include "services/network/public/cpp/simple_url_loader.h"
#include "jemaos/license/jemaos_callback_status.h"

namespace jemaos::license {

enum FetchMode {
  OnlineMode,
  OfflineMode,
};

class LicenseOnlineFetcher /*:  public net::URLFetcherDelegate*/ {
 public:
  LicenseOnlineFetcher();
  ~LicenseOnlineFetcher();
  void StartFetch(const std::string& id,
                  const std::string& serial_number,
                  const bool is_new_license,
                  const std::string& oem_token,
                  SuccessCallback<std::optional<std::string>> success_callback,
                  ErrorCallback err_callback);
  FetchMode GetFetchMode();
 private:
 /*
  scoped_refptr<net::URLRequestContextGetter> request_context_getter_;
  std::unique_ptr<net::URLRequestContext> url_request_context_;
  std::unique_ptr<base::OneShotTimer> timeout_;
  */
  SuccessCallback<std::optional<std::string>> success_callback_;
  ErrorCallback err_callback_;
  std::unique_ptr<network::SimpleURLLoader> simple_loader_;
  void OnURLFetchComplete(std::unique_ptr<std::string> response_body);
  void OnTimeout();
};

}  // namespace jemaos::license


#endif  // JEMAOS_LICENSE_FETCHER_H_
