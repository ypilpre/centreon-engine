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
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::notifications;

extern configuration::state* config;

class RecoveryNotification : public ::testing::Test {
 public:
  void SetUp() {
    set_time(20);
    _notifier.reset(new test_notifier());
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();
    engine::contact::add_contact("test");

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

// Given a notifier that was in state 2 with a current notification already
// sent 10 seconds ago.
// When the notify method is called with RECOVERY type
// Then the filter method returns true and the notification is sent.
TEST_F(RecoveryNotification, SimpleRecovery) {
  _notifier->set_notifications_enabled(true);
  _notifier->set_current_state(2);
  _notifier->set_current_notification_type(notifier::PROBLEM);
  _notifier->set_last_notification(10);

  long last_notification = _notifier->get_last_notification();
  // When
  _notifier->set_current_state(0);
  _notifier->notify(notifier::RECOVERY, "admin", "Test comment");
  long current_notification = _notifier->get_last_notification();
  ASSERT_GT(current_notification, last_notification);
}

// Given a notifier that was in state 2 with a current notification already
// sent 10 seconds ago.
// When the new state is OK but the notifier is in downtime
// When the notify method is called with RECOVERY type
// Then the filter method returns false and the notification is not sent.
TEST_F(RecoveryNotification, RecoveryDuringDowntime) {
  _notifier->set_current_state(2);
  _notifier->set_current_notification_type(notifier::PROBLEM);
  _notifier->set_last_notification(10);

  _notifier->set_in_downtime(true);
  long last_notification = _notifier->get_last_notification();
  // When
  _notifier->set_current_state(0);
  _notifier->notify(notifier::RECOVERY, "admin", "Test comment");
  long current_notification = _notifier->get_last_notification();
  ASSERT_EQ(current_notification, last_notification);
}

// Given a notifier that was in state 2 with a current notification already
// sent 10 seconds ago.
// When the new state has changed but is not OK
// When the notify method is called with RECOVERY type
// Then the filter method returns false and the notification is not sent.
TEST_F(RecoveryNotification, RecoveryWithNonOkState) {
  _notifier->set_current_state(2);
  _notifier->set_current_notification_type(notifier::PROBLEM);
  _notifier->set_last_notification(10);

  _notifier->set_in_downtime(true);
  long last_notification = _notifier->get_last_notification();
  // When
  _notifier->set_current_state(1);
  _notifier->notify(notifier::RECOVERY, "admin", "Test comment");
  long current_notification = _notifier->get_last_notification();
  ASSERT_EQ(current_notification, last_notification);
}
