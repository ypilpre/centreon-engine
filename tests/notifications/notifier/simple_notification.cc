/*
** Copyright 2017 Centreon
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <memory>
#include <gtest/gtest.h>
#include "../test_notifier.hh"
#include "com/centreon/engine/notifications/notifier.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::notifications;

class SimpleNotification : public ::testing::Test {
 public:
  void SetUp() {
    _notifier.reset(new test_notifier());
  }

 protected:
  std::auto_ptr<test_notifier>       _notifier;
};

// Given a notifier
// When ...
// Then ...
TEST_F(SimpleNotification, ProblemFilter) {

  // Then
  ASSERT_TRUE(_notifier.get());
}
