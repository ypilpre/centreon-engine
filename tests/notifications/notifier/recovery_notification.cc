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
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
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

class RecoveryNotification : public ::testing::Test {
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
    configuration::contact ctct("test");
    cm["test"] = shared_ptr<engine::contact>(new engine::contact(ctct));
    _service->add_contact(
      configuration::applier::state::instance().contacts_find("test").get());
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

 protected:
  engine::service*  _service;
};

// Given a notifier that was in state 2 with a current notification already
// sent 10 seconds ago.
// When the notify method is called with RECOVERY type
// Then the filter method returns true and the notification is sent.
TEST_F(RecoveryNotification, SimpleRecovery) {
  _service->set_notifications_enabled(true);
  _service->set_current_state(2);
  _service->set_last_hard_state(2);
  _service->add_notification_flag(notifier::PROBLEM);
  _service->set_last_notification(10);

  long last_notification = _service->get_last_notification();
  // When
  set_time(last_notification + 10);

  _service->set_last_state(2);
  _service->set_last_state_change(time(NULL));
  _service->set_current_state(0);
  _service->notify(notifier::RECOVERY, "admin", "Test comment");
  long current_notification = _service->get_last_notification();
  ASSERT_GT(current_notification, last_notification);
}

// Given a notifier that was in state 2 with a current notification already
// sent 10 seconds ago.
// When the new state is OK but the notifier is in downtime
// When the notify method is called with RECOVERY type
// Then the filter method returns false and the notification is not sent.
TEST_F(RecoveryNotification, RecoveryDuringDowntime) {
  _service->set_current_state(2);
  _service->add_notification_flag(notifier::PROBLEM);
  _service->set_last_notification(10);

  _service->set_in_downtime(true);
  long last_notification = _service->get_last_notification();
  // When
  _service->set_current_state(0);
  _service->notify(notifier::RECOVERY, "admin", "Test comment");
  long current_notification = _service->get_last_notification();
  ASSERT_EQ(current_notification, last_notification);
}

// Given a notifier that was in state 2 with a current notification already
// sent 10 seconds ago.
// When the new state has changed but is not OK
// When the notify method is called with RECOVERY type
// Then the filter method returns false and the notification is not sent.
TEST_F(RecoveryNotification, RecoveryWithNonOkState) {
  _service->set_current_state(2);
  _service->add_notification_flag(notifier::PROBLEM);
  _service->set_last_notification(10);

  _service->set_in_downtime(true);
  long last_notification = _service->get_last_notification();
  // When
  _service->set_current_state(1);
  _service->notify(notifier::RECOVERY, "admin", "Test comment");
  long current_notification = _service->get_last_notification();
  ASSERT_EQ(current_notification, last_notification);
}
