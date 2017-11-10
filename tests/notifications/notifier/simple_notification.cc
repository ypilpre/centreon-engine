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
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::notifications;
using namespace com::centreon::engine::configuration;

extern configuration::state* config;

class SimpleNotification : public ::testing::Test {
 public:
  void SetUp() {
    set_time(20);
    _notifier.reset(new test_notifier());
    if (config == NULL) {
      config = new configuration::state;
    }
    shared_ptr<configuration::contact> user(new configuration::contact);
    _notifier->add_contact(user);
  }

 protected:
  std::auto_ptr<test_notifier>       _notifier;
};

// Given a notifier in downtime
// When the notify method is called with PROBLEM type
// Then the filter method returns false and no notification is sent.
TEST_F(SimpleNotification, ProblemWithDowntime) {

  _notifier->set_in_downtime(true);
  long last_notification = _notifier->get_last_notification();
  // When
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_EQ(last_notification, _notifier->get_last_notification());
}

// Given a flapping notifier
// When the notify method is called with PROBLEM type
// Then the filter method returns false and no notification is sent.
TEST_F(SimpleNotification, ProblemDuringFlapping) {

  _notifier->set_is_flapping(true);
  long last_notification = _notifier->get_last_notification();
  // When
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_EQ(last_notification, _notifier->get_last_notification());
}

// Given a notifier that notifies on any state.
// When the notify method is called with PROBLEM type for a given state
// Then the filter method returns false and no notification is sent.
TEST_F(SimpleNotification, ProblemWithUnnotifiedState) {

  _notifier->set_current_state(1);
  long last_notification = _notifier->get_last_notification();
  // When
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_EQ(last_notification, _notifier->get_last_notification());
}

// Given a notifier with state 1.
// When notification is enabled on state 1
// And no contact user for notification are configured
// And the notify method is called with PROBLEM type for state 1
// Then no notification is sent.
TEST_F(SimpleNotification, NoContactUser) {
  // When
  _notifier->set_current_state(1);
  time_t last_notification = _notifier->get_last_notification();
  time_t now = last_notification + 20;
  set_time(now);
  // And
  _notifier->clear_contacts();
  // And
  _notifier->enable_state_notification(1);
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  // Then
  ASSERT_EQ(last_notification, _notifier->get_last_notification());
}

// Given a notifier with state 1.
// When notification is enabled on state 1
// And the notify method is called with PROBLEM type for state 1
// Then a notification is sent.
TEST_F(SimpleNotification, SimpleNotification) {

  _notifier->set_current_state(1);
  time_t last_notification = _notifier->get_last_notification();
  // When
  time_t now = last_notification + 20;
  set_time(now);
  _notifier->enable_state_notification(1);
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_TRUE(_notifier->get_last_notification() >= now);
}

// Given a notifier with state 1.
// When notification is enabled on state 1
// And the notify method is called with PROBLEM type for state 1
// Then a notification is sent.
TEST_F(SimpleNotification, TooEarlyNewNotification) {

  _notifier->set_current_state(1);
  _notifier->set_current_notification_type(notifier::PROBLEM);
  _notifier->set_current_notification_number(1);
  _notifier->enable_state_notification(1);
  time_t last_notification = 20;
  _notifier->set_last_notification(last_notification);
  _notifier->set_notification_interval(60);
  time_t now = last_notification + 10;
  set_time(now);
  // When
  _notifier->set_current_state(1);
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_TRUE(_notifier->get_last_notification() < now);
}
