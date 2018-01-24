/*
** Copyright 2017-2018 Centreon
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
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::notifications;

extern configuration::state* config;

class AcknowledgementNotification : public ::testing::Test {
 public:
  void SetUp() {
    set_time(20);
    _notifier.reset(new test_notifier());
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();
    contact_map& cm = configuration::applier::state::instance().contacts();
    configuration::contact ctct("test");
    cm["test"] = shared_ptr<engine::contact>(new engine::contact(ctct));
    _notifier->add_contact(configuration::applier::state::instance().contacts_find("test").get());
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

 protected:
  std::auto_ptr<test_notifier>       _notifier;
};

// Given a notifier in state 0. Let's assume notification is enabled on
// states 1, 2 and on recovery.
// When the service is set in hard WARNING,
// Then a notification is sent.
// When the service is acknowledged (normally),
// Then no more notification is sent while the state is WARNING.
// When the service is set to hard CRITICAL,
// Then the acknowledgement is removed and a notification is sent.
// When the service is set to OK,
// Then a recovery notification is sent.
TEST_F(AcknowledgementNotification, AcknowledgementNotification) {

  _notifier->set_current_state(0);
  _notifier->set_last_state_change(time(NULL));
  _notifier->set_notifications_enabled(true);
  time_t last_notification = _notifier->get_last_notification();
  _notifier->enable_state_notification(0);
  _notifier->enable_state_notification(1);
  _notifier->enable_state_notification(2);
  // When the service is set in hard WARNING
  time_t now = time(NULL) + 20;
  set_time(now);
  _notifier->set_current_state(1);
  _notifier->set_last_hard_state(1);
  _notifier->set_last_state_change(now);
  _notifier->set_last_hard_state_change(now);
  _notifier->set_last_check(now);
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  // Then a notification is sent
  ASSERT_TRUE(_notifier->get_last_notification() >= now);

  // When the service is acknowledged (normally)
  now += 20;
  set_time(now);
  _notifier->set_acknowledged(notifier::ACKNOWLEDGEMENT_NORMAL);
  _notifier->set_last_acknowledgement(now);
  // Then no more notification is sent while the state is WARNING
  _notifier->notify(notifier::ACKNOWLEDGEMENT, "admin", "Test comment");
  ASSERT_TRUE(_notifier->get_last_notification() >= now);
  now += 20;
  set_time(now);
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_FALSE(_notifier->get_last_notification() >= now);

  // When the service is set to hard CRITICAL
  now += _notifier->get_notification_interval() + 1;
  set_time(now);
  _notifier->set_current_state(1);
  _notifier->set_last_state(1);
  _notifier->set_last_hard_state(1);
  _notifier->set_last_state_change(now);
  _notifier->set_last_hard_state_change(now);
  _notifier->set_last_check(now);
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_FALSE(_notifier->get_last_notification() >= now);

  // When the service is set to hard CRITICAL
  now += 20;
  set_time(now);
  _notifier->set_current_state(2);
  _notifier->set_last_hard_state(2);
  _notifier->set_last_state_change(now);
  _notifier->set_last_hard_state_change(now);
  _notifier->set_last_check(now);
  // Then the acknowledgement is removed and a notification is sent.
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_FALSE(_notifier->is_acknowledged());
  ASSERT_TRUE(_notifier->get_last_notification() >= now);

  // When the service is set to OK,
  now += 20;
  set_time(now);
  _notifier->set_current_state(0);
  _notifier->set_last_state_change(now);
  _notifier->set_last_check(now);
  // Then a recovery notification is sent.
  _notifier->notify(notifier::RECOVERY, "admin", "Test comment");
  ASSERT_TRUE(_notifier->get_last_notification() >= now);
}

// Given a notifier in state 0. Let's assume notification is enabled on
// states 1, 2 and on recovery.
// When the service is set in hard WARNING,
// Then a notification is sent.
// When the service is sticky acknowledged,
// Then no more notification is sent while the state is WARNING.
// When the service is set to hard CRITICAL,
// Then no notification is sent.
// When the service is set to OK,
// Then the acknowledgement is removed and a recovery notification is sent.
TEST_F(AcknowledgementNotification, StickyAcknowledgementNotification) {

  _notifier->set_current_state(0);
  _notifier->set_last_state_change(time(NULL));
  _notifier->set_notifications_enabled(true);
  time_t last_notification = _notifier->get_last_notification();
  _notifier->enable_state_notification(0);
  _notifier->enable_state_notification(1);
  _notifier->enable_state_notification(2);
  // When the service is set in hard WARNING
  time_t now = time(NULL) + 20;
  set_time(now);
  _notifier->set_current_state(1);
  _notifier->set_last_hard_state(1);
  _notifier->set_last_state_change(now);
  _notifier->set_last_hard_state_change(now);
  _notifier->set_last_check(now);
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  // Then a notification is sent
  ASSERT_TRUE(_notifier->get_last_notification() >= now);

  // When the service is acknowledged (normally)
  now += 20;
  set_time(now);
  _notifier->set_acknowledged(notifier::ACKNOWLEDGEMENT_STICKY);
  _notifier->set_last_acknowledgement(now);
  // Then no more notification is sent while the state is WARNING
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_FALSE(_notifier->get_last_notification() >= now);

  now += _notifier->get_notification_interval() + 1;
  set_time(now);
  _notifier->set_current_state(1);
  _notifier->set_last_state(1);
  _notifier->set_last_hard_state(1);
  _notifier->set_last_state_change(now);
  _notifier->set_last_hard_state_change(now);
  _notifier->set_last_check(now);
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_FALSE(_notifier->get_last_notification() >= now);

  // When the service is set to hard CRITICAL
  now += 20;
  set_time(now);
  _notifier->set_current_state(2);
  _notifier->set_last_hard_state(2);
  _notifier->set_last_state_change(now);
  _notifier->set_last_hard_state_change(now);
  _notifier->set_last_check(now);
  // Then no more notification is sent while the state is WARNING
  _notifier->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_FALSE(_notifier->get_last_notification() >= now);

  // When the service is set to OK,
  now += 20;
  set_time(now);
  _notifier->set_current_state(0);
  _notifier->set_last_state_change(now);
  _notifier->set_last_check(now);
  // Then the acknowledgement is removed and a recovery notification is sent.
  _notifier->notify(notifier::RECOVERY, "admin", "Test comment");
  ASSERT_FALSE(_notifier->is_acknowledged());
  ASSERT_TRUE(_notifier->get_last_notification() >= now);
}
