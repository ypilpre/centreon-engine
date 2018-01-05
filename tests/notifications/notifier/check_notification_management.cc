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
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::notifications;

extern configuration::state* config;

class CheckNotificationManagement : public ::testing::Test {
 public:
  void SetUp() {
    set_time(20);
    _notifier.reset(new test_notifier());
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();
    contact_map& cm(
      configuration::applier::state::instance().contacts());
    configuration::contact ctct("test");
    cm["test"] = shared_ptr<engine::contact>(new engine::contact(ctct));

    _notifier->add_contact(configuration::applier::state::instance().contacts_find("test")->second);
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

 protected:
  std::auto_ptr<test_notifier>       _notifier;
};

// Given a notifier in downtime
// When the notify method is called with PROBLEM type
// Then the filter method returns false and no notification is sent.
TEST_F(CheckNotificationManagement, ProblemWithDowntime) {

  _notifier->set_in_downtime(true);
  time_t last_notification = _notifier->get_last_notification();
  // When
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_EQ(last_notification, _notifier->get_last_notification());
}

// Given a flapping notifier
// When the notify method is called with PROBLEM type
// Then the filter method returns false and no notification is sent.
TEST_F(CheckNotificationManagement, ProblemDuringFlapping) {

  _notifier->set_flapping(true);
  time_t last_notification = _notifier->get_last_notification();
  // When the notify method is called with PROBLEM type
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  // Then the filter method returns false and no notification is sent
  ASSERT_EQ(last_notification, _notifier->get_last_notification());
}

// Given a notifier that notifies on any state.
// When the notify method is called with PROBLEM type for a given state
// Then the filter method returns false and no notification is sent.
TEST_F(CheckNotificationManagement, ProblemWithUnnotifiedState) {

  _notifier->set_current_state(1);
  time_t last_notification = _notifier->get_last_notification();
  // When
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_EQ(last_notification, _notifier->get_last_notification());
}

// Given a notifier with state 1.
// When notification is enabled on state 1
// And no contact user for notification are configured
// And the notify method is called with PROBLEM type for state 1
// Then no notification is sent.
TEST_F(CheckNotificationManagement, NoContactUser) {
  // When
  _notifier->set_current_state(1);
  time_t last_notification = _notifier->get_last_notification();
  time_t now = last_notification + 20;
  set_time(now);
  // And
  _notifier->get_contacts().clear();
  // And
  _notifier->enable_state_notification(1);
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  // Then
  ASSERT_EQ(last_notification, _notifier->get_last_notification());
}

// Given a notifier with state 1.
// When notification is enabled on state 1
// And the notify method is called with PROBLEM type for state 1
// And the notifier is not in hard state
// Then no notification is sent.
TEST_F(CheckNotificationManagement, NoHardNoNotification) {

  _notifier->set_current_state(1);
  _notifier->set_notifications_enabled(true);
  time_t last_notification = _notifier->get_last_notification();
  _notifier->enable_state_notification(1);
  // When
  time_t now = last_notification + 20;
  set_time(now);
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_FALSE(_notifier->get_last_notification() >= now);
}

// Given a notifier with state 1.
// When notification is enabled on state 1
// And the notify method is called with PROBLEM type for state 1
// And the notifier is in hard state
// Then a notification is sent.
TEST_F(CheckNotificationManagement, CheckNotificationManagement) {

  _notifier->set_current_state(1);
  _notifier->set_last_hard_state(1);
  _notifier->set_last_state_change(time(NULL));
  _notifier->set_last_hard_state_change(time(NULL));
  _notifier->set_notifications_enabled(true);
  time_t last_notification = _notifier->get_last_notification();
  _notifier->enable_state_notification(1);
  // When
  time_t now = last_notification + 20;
  set_time(now);
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_TRUE(_notifier->get_last_notification() >= now);
}

// Given a notifier with state 1.
// When notification is enabled on state 1
// And the notify method is called with PROBLEM type for state 1
// Then a notification is sent.
TEST_F(CheckNotificationManagement, TooEarlyNewNotification) {

  _notifier->set_notifications_enabled(true);
  _notifier->set_current_state(1);
  _notifier->set_last_state(1);
  _notifier->set_last_hard_state(1);
  _notifier->set_last_state_change(time(NULL));
  _notifier->set_last_hard_state_change(time(NULL));
  _notifier->add_notification_flag(notifier::PROBLEM);
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
