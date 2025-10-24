// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_MISC_SCHEDULER_H_
#define JEMAOS_MISC_SCHEDULER_H_

#include <memory>
#include "chromeos/chromeos_export.h"
#include "chromeos/ash/components/login/login_state/login_state.h"
#include "jemaos/build/config/buildflags.h"

namespace jemaos {
namespace misc {

class StatisticsCollector;
class MiscCrostiniNotifier;

class CHROMEOS_EXPORT JemaMiscScheduler : public ::ash::LoginState::Observer {
 public:
    JemaMiscScheduler();
    ~JemaMiscScheduler() override;
    static void Initialize();
    static JemaMiscScheduler* Get();
    static void Shutdown();
    void Start();
    void Stop();

 private:
    void LoggedInStateChanged() override;

    bool started_ = false;
#if BUILDFLAG(USE_JEMAOS_COM)
    std::unique_ptr<MiscCrostiniNotifier> crostini_notifier_;
#endif
    std::unique_ptr<StatisticsCollector> collector_;
};

}  // namespace misc
}  // namespace jemaos

#endif /* ifndef JEMAOS_MISC_SCHEDULER_H_ */
