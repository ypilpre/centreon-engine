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
#include "../../timeperiod/utils.hh"
#include "../test_notifier.hh"
#include "com/centreon/engine/notifications/notifier.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::notifications;

class SimpleNotification : public ::testing::Test {
 public:
  void SetUp() {
    set_time(20);
    _notifier.reset(new test_notifier());
  }

 protected:
  std::auto_ptr<test_notifier>       _notifier;
};

// Given a notifier in downtime
// When the notify method is called with PROBLEM type
// Then the filter method returns false and no notification is sent.
TEST_F(SimpleNotification, ProblemWithDowntime) {

  _notifier->set_in_downtime(true);
  long last_notification = _notifier->get_last_notification_date();
  // When
  _notifier->notify(notifier::PROBLEM);
  ASSERT_EQ(last_notification, _notifier->get_last_notification_date());
}

// Given a flapping notifier
// When the notify method is called with PROBLEM type
// Then the filter method returns false and no notification is sent.
TEST_F(SimpleNotification, ProblemDuringFlapping) {

  _notifier->set_is_flapping(true);
  long last_notification = _notifier->get_last_notification_date();
  // When
  _notifier->notify(notifier::PROBLEM);
  ASSERT_EQ(last_notification, _notifier->get_last_notification_date());
}

// Given a notifier that notifies on any state.
// When the notify method is called with PROBLEM type for a given state
// Then the filter method returns false and no notification is sent.
TEST_F(SimpleNotification, ProblemWithUnnotifiedState) {

  _notifier->set_state(1);
  long last_notification = _notifier->get_last_notification_date();
  // When
  _notifier->notify(notifier::PROBLEM);
  ASSERT_EQ(last_notification, _notifier->get_last_notification_date());
}

// Given a notifier with state 1.
// When notification is enabled on state 1
// And the notify method is called with PROBLEM type for state 1
// Then a notification is sent.
TEST_F(SimpleNotification, ProblemWithState1) {

  _notifier->set_state(1);
  long last_notification = _notifier->get_last_notification_date();
  time_t now;
  time(&now);
  ASSERT_TRUE(now > 0);
  std::cout << "time: " << now << std::endl;
  // When
  _notifier->enable_state_notification(1);
  _notifier->notify(notifier::PROBLEM);
  ASSERT_TRUE(_notifier->get_last_notification_date() >= now);
}
