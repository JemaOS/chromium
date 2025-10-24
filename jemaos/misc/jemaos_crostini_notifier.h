// Copyright 2024 The Jema Technology Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_MISC_JEMAOS_CROSITINI_NOTIFIER_H_
#define JEMAOS_MISC_JEMAOS_CROSITINI_NOTIFIER_H_

#include "base/memory/weak_ptr.h"
#include "ui/message_center/public/cpp/notification_delegate.h"

class Profile;

namespace jemaos::misc {

class MiscCrostiniNotifier : public message_center::NotificationObserver {
public:
  MiscCrostiniNotifier();
  virtual ~MiscCrostiniNotifier();

  void Start();

private:
  // message_center::NotificationObserver
  void Click(const std::optional<int>& button_index,
             const std::optional<std::u16string>& reply) override;
  void Close(bool by_user) override;

  void MayShowCrostiniInstallerNotification();
  void HandleInstallNotificationClick();

  void CloseNotification();

  bool started_ = false;
  Profile* profile_ = nullptr;

  base::WeakPtrFactory<MiscCrostiniNotifier> weak_ptr_factory_{this};
};

} // namespace jemaos::misc



#endif // JEMAOS_MISC_JEMAOS_CROSITINI_NOTIFIER_H_
