// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_EMAIL_SUBSCRIPTION_H
#define JEMAOS_EMAIL_SUBSCRIPTION_H

class Profile;

namespace jemaos {

namespace misc {

void Subscribe(Profile* profile, bool email_opt_in, bool improve_plan_opt_in);

} // misc

} // jemaos


#endif
