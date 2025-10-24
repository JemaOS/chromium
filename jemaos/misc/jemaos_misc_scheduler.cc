// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/misc/jemaos_misc_scheduler.h"
#include "jemaos/misc/jemaos_statistics_collector.h"
#include "jemaos/build/config/buildflags.h"
#include <base/logging.h>

#if BUILDFLAG(USE_JEMAOS_COM)
#include "jemaos/misc/jemaos_crostini_notifier.h"
#endif

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
#if BUILDFLAG(USE_JEMAOS_COM)
  crostini_notifier_(std::make_unique<MiscCrostiniNotifier>()),
#endif
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
#if BUILDFLAG(USE_JEMAOS_COM)
  crostini_notifier_->Start();
#endif
}

void JemaMiscScheduler::Stop() {
  VLOG(2) << "JemaMiscScheduler Stop";
  collector_->Stop();
  started_ = false;
}

void JemaMiscScheduler::LoggedInStateChanged() {
  if (::ash::LoginState::Get()->IsUserLoggedIn() && ::ash::LoginState::Get()->IsUserAuthenticated()) {
    Start();
  } else {
    Stop();
  }
}

}  // namespace misc
}  // namespace jemaos
