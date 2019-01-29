/*
** Copyright 2017-2019 Centreon
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
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::notifications;

extern configuration::state* config;

class CheckNotificationManagement : public ::testing::Test {
 public:
  void SetUp() {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();

    configuration::applier::host hst_aply;
    configuration::applier::service svc_aply;
    configuration::service csvc;

    csvc.parse("service_description", "check description");
    csvc.parse("host_name", "test_host");

    configuration::host hst;
    hst.parse("host_name", "test_host");
    hst_aply.add_object(hst);
    svc_aply.add_object(csvc);
    csvc.parse("hosts", "test_host");

    configuration::applier::command cmd_aply;
    configuration::command cmd("cmd");
    cmd.parse("command_line", "echo 1");
    csvc.parse("check_command", "cmd");
    cmd_aply.add_object(cmd);

    svc_aply.resolve_object(csvc);

    _service = configuration::applier::state::instance()
                 .services().begin()->second.get();

    set_time(20);
    contact_map& cm(configuration::applier::state::instance().contacts());
    cm["test"] = shared_ptr<engine::contact>(new engine::contact());
    cm["test"]->set_name("test");
    _service->add_contact(
      configuration::applier::state::instance().contacts_find("test").get());
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

 protected:
  service*  _service;
};

// Given a notifier in downtime
// When the notify method is called with PROBLEM type
// Then the filter method returns false and no notification is sent.
TEST_F(CheckNotificationManagement, ProblemWithDowntime) {
  _service->inc_scheduled_downtime_depth();
  time_t last_notification = _service->get_last_notification();
  // When
  _service->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_EQ(last_notification, _service->get_last_notification());
}

// Given a flapping notifier
// When the notify method is called with PROBLEM type
// Then the filter method returns false and no notification is sent.
TEST_F(CheckNotificationManagement, ProblemDuringFlapping) {

  _service->set_flapping(true);
  time_t last_notification = _service->get_last_notification();
  // When the notify method is called with PROBLEM type
  _service->notify(notifier::PROBLEM, "admin", "Test comment");
  // Then the filter method returns false and no notification is sent
  ASSERT_EQ(last_notification, _service->get_last_notification());
}

// Given a notifier that notifies on any state.
// When the notify method is called with PROBLEM type for a given state
// Then the filter method returns false and no notification is sent.
TEST_F(CheckNotificationManagement, ProblemWithUnnotifiedState) {

  _service->set_current_state(1);
  time_t last_notification = _service->get_last_notification();
  // When
  _service->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_EQ(last_notification, _service->get_last_notification());
}

// Given a notifier with state 1.
// When notification is enabled on state 1
// And no contact user for notification are configured
// And the notify method is called with PROBLEM type for state 1
// Then no notification is sent.
TEST_F(CheckNotificationManagement, NoContactUser) {
  // When
  _service->set_current_state(1);
  time_t last_notification = _service->get_last_notification();
  time_t now = last_notification + 20;
  set_time(now);
  // And
  _service->clear_contacts();
  // And
  _service->set_notify_on(notifier::ON_WARNING, true);
  _service->notify(notifier::PROBLEM, "admin", "Test comment");
  // Then
  ASSERT_EQ(last_notification, _service->get_last_notification());
}

// Given a notifier with state 1.
// When notification is enabled on state 1
// And the notify method is called with PROBLEM type for state 1
// And the notifier is not in hard state
// Then no notification is sent.
TEST_F(CheckNotificationManagement, NoHardNoNotification) {
  _service->set_current_state(1);
  _service->set_current_state_type(SOFT_STATE);
  _service->set_notifications_enabled(true);
  _service->set_last_notification(1598741230);
  _service->set_notify_on(notifier::ON_WARNING, true);
  time_t now(_service->get_last_notification() + 20);
  set_time(now);
  _service->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_LT(_service->get_last_notification(), now);
}

// Given a notifier with state 1.
// When notification is enabled on state 1
// And the notify method is called with PROBLEM type for state 1
// And the notifier is in hard state
// Then a notification is sent.
TEST_F(CheckNotificationManagement, CheckNotificationManagement) {
  _service->set_current_state(1);
  _service->set_current_state_type(HARD_STATE);
  _service->set_notifications_enabled(true);
  time_t last_notification = _service->get_last_notification();
  _service->set_notify_on(notifier::ON_WARNING, true);
  // When
  time_t now = last_notification + 20;
  set_time(now);
  _service->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_GE(_service->get_last_notification(), now);
}

// Given a notifier with state 1.
// When notification is enabled on state 1
// And the notify method is called with PROBLEM type for state 1
// Then a notification is sent.
TEST_F(CheckNotificationManagement, TooEarlyNewNotification) {

  _service->set_notifications_enabled(true);
  _service->set_current_state(1);
  _service->set_last_state(1);
  _service->set_last_hard_state(1);
  _service->set_last_state_change(time(NULL));
  _service->set_last_hard_state_change(time(NULL));
  _service->set_current_notification_number(1);
  _service->set_notify_on(notifier::ON_WARNING, true);
  time_t last_notification = 20;
  _service->set_last_notification(last_notification);
  _service->set_notification_interval(60);
  time_t now = last_notification + 10;
  set_time(now);
  // When
  _service->set_current_state(1);
  _service->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_TRUE(_service->get_last_notification() < now);
}
