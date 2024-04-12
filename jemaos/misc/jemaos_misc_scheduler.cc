// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/misc/jemaos_misc_scheduler.h"
#include "jemaos/misc/jemaos_stateful_update.h"
#include "jemaos/misc/jemaos_statistics_collector.h"
#include <base/logging.h>

namespace jemaos {
namespace misc {

namespace {
  JemaMiscScheduler* g_misc_scheduler = nullptr;
}

void JemaMiscScheduler::Initialize() {
  CHECK(g_misc_scheduler == nullptr);
  g_misc_scheduler = new JemaMiscScheduler();
}

JemaMiscScheduler* JemaMiscScheduler::Get() {
  return g_misc_scheduler;
}

void JemaMiscScheduler::Shutdown() {
  CHECK(g_misc_scheduler != nullptr);
  g_misc_scheduler->Stop();
  delete g_misc_scheduler;
  g_misc_scheduler = nullptr;
}

JemaMiscScheduler::JemaMiscScheduler() :
  // updater_(std::make_unique<StatefulUpdater>()),
  collector_(std::make_unique<StatisticsCollector>()) {
  if (::ash::LoginState::IsInitialized()) {
    ::ash::LoginState::Get()->AddObserver(this);
  }
}

JemaMiscScheduler::~JemaMiscScheduler() = default;

void JemaMiscScheduler::Start() {
  if (started_) return;
  VLOG(2) << "JemaMiscScheduler Start";
  started_ = true;
  collector_->Start();
  // updater_->Start();
}

void JemaMiscScheduler::Stop() {
  VLOG(2) << "JemaMiscScheduler Stop";
  collector_->Stop();
  // updater_->Stop();
  started_ = false;
}

void JemaMiscScheduler::LoggedInStateChanged() {
  if (::ash::LoginState::Get()->IsUserLoggedIn()) {
    Start();
  } else {
    Stop();
  }
}

}  // namespace misc
}  // namespace jemaos
