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
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::notifications;

extern configuration::state* config;

class FlappingNotification : public ::testing::Test {
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
    cm["test"]->set_alias("test");

    _service->add_contact(
      configuration::applier::state::instance().contacts_find("test").get());
    _service->set_notify_on(notifier::ON_RECOVERY, true);
    _service->set_notify_on(notifier::ON_FLAPPING, true);
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

 protected:
  engine::service*  _service;
};

// Given a notifier not in downtime with notifications enabled
// Then The filter on FLAPPINGSTART notification is OK to send notification
// When a FLAPPINGSTART notification is already sent
// Then the FLAPPINGSTOP notification can be sent when the flapping is finished.
TEST_F(FlappingNotification, StartStopFlapping) {
  time_t last_notification = _service->get_last_notification();
  _service->set_notifications_enabled(true);
  _service->set_flapping(true);
  _service->notify(notifier::FLAPPINGSTART, "admin", "Test start flapping");
  time_t current_notification = _service->get_last_notification();
  ASSERT_GT(current_notification, last_notification);
  _service->set_flapping(false);
  time_t now = last_notification + 10;
  set_time(now);
  _service->notify(notifier::FLAPPINGSTOP, "admin", "Test stop flapping");
  last_notification = _service->get_last_notification();
  ASSERT_EQ(now, last_notification);
}

// Given a notifier not in downtime with notifications enabled
// Then The filter on FLAPPINGSTART notification is OK to send notification
// When a FLAPPINGSTART notification is already sent
// Then the FLAPPINGDISABLED notification can be sent when the flapping is
// finished.
TEST_F(FlappingNotification, StartDisableFlapping) {
  time_t last_notification = _service->get_last_notification();
  _service->set_notifications_enabled(true);
  _service->set_flapping(true);
  _service->notify(notifier::FLAPPINGSTART, "admin", "Test start flapping");
  time_t current_notification = _service->get_last_notification();
  ASSERT_GT(current_notification, last_notification);
  _service->set_flapping(false);
  time_t now = last_notification + 10;
  set_time(now);
  _service->notify(notifier::FLAPPINGDISABLED, "admin", "Test disable flapping");
  last_notification = _service->get_last_notification();
  ASSERT_EQ(now, last_notification);
}

// Given a notifier not in downtime with notifications enabled
// Then The filter on FLAPPINGSTART notification is OK to send notification
// When a FLAPPINGSTART notification is already sent
// Then the FLAPPINGSTART notification can not be resent.
TEST_F(FlappingNotification, StartStartFlapping) {
  time_t last_notification = _service->get_last_notification();
  _service->set_notifications_enabled(true);
  _service->set_flapping(true);
  _service->notify(notifier::FLAPPINGSTART, "admin", "Test start flapping");
  time_t current_notification = _service->get_last_notification();
  ASSERT_GT(current_notification, last_notification);
  _service->set_flapping(false);
  time_t now = last_notification + 10;
  set_time(now);
  _service->notify(notifier::FLAPPINGSTOP, "admin", "Test stop flapping");
  last_notification = _service->get_last_notification();
  ASSERT_EQ(now, last_notification);
}

// Given a notifier not in downtime with notifications enabled
// When the notifier is not OK
// And the filter on FLAPPINGSTART notification is OK to send notification
// And a FLAPPINGSTART notification is already sent
// And the flapping is stopped
// Then the FLAPPINGSTOP notification can be sent
// And the RECOVERY notification can also be sent
TEST_F(FlappingNotification, StartStopFlappingWithRecovery) {
  time_t last_notification = _service->get_last_notification();
  _service->set_notifications_enabled(true);
  _service->set_current_state(2);
  _service->set_current_state_type(HARD_STATE);
  _service->notify(notifier::PROBLEM, "admin", "Test with a critical problem");
  _service->set_flapping(true);
  _service->notify(notifier::FLAPPINGSTART, "admin", "Test start flapping");
  time_t current_notification = _service->get_last_notification();
  ASSERT_GT(current_notification, last_notification);
  _service->set_flapping(false);
  _service->set_current_state(0);
  time_t now = last_notification + 10;
  set_time(now);
  _service->notify(notifier::FLAPPINGSTOP, "admin", "Test stop flapping");
  last_notification = _service->get_last_notification();
  ASSERT_EQ(now, last_notification);
  ++now;
  set_time(now);
  _service->notify(notifier::RECOVERY, "admin", "Test recovery");
  last_notification = _service->get_last_notification();
  ASSERT_EQ(now, last_notification);
}

// Given a notifier not in downtime with notifications enabled
// When the notifier is OK
// And the filter on FLAPPINGSTART notification is OK to send notification
// And a FLAPPINGSTART notification is already sent
// And the flapping is stopped
// Then the FLAPPINGSTOP notification can be sent
// And the RECOVERY notification cannot be sent
TEST_F(FlappingNotification, StartStopFlappingWithoutRecovery) {
  time_t last_notification = _service->get_last_notification();
  _service->set_notifications_enabled(true);
  _service->set_current_state(0);
  _service->set_current_state_type(HARD_STATE);
  _service->set_flapping(true);
  _service->notify(notifier::FLAPPINGSTART, "admin", "Test start flapping");
  time_t current_notification = _service->get_last_notification();
  ASSERT_GT(current_notification, last_notification);
  _service->set_flapping(false);
  _service->set_current_state(0);
  time_t now = last_notification + 10;
  set_time(now);
  _service->notify(notifier::FLAPPINGSTOP, "admin", "Test stop flapping");
  last_notification = _service->get_last_notification();
  ASSERT_EQ(now, last_notification);
  ++now;
  set_time(now);
  _service->notify(notifier::RECOVERY, "admin", "Test recovery");
  last_notification = _service->get_last_notification();
  ASSERT_NE(now, last_notification);
}

// Given a notifier not in downtime with notifications enabled
// And the filter on FLAPPINGSTART notification is OK to send notification
// When set_service_flap is called
// Then a FLAPPINGSTART notification is sent and a comment is added.
TEST_F(FlappingNotification, StartStopFlappingAndComment) {
  time_t last_notification = _service->get_last_notification();
  _service->set_notifications_enabled(true);
  _service->set_current_state(0);
  _service->set_current_state_type(HARD_STATE);
  set_service_flap(_service, 10, 80, 10);
  ASSERT_TRUE(_service->get_flapping());
  time_t current_notification = _service->get_last_notification();
  ASSERT_GT(current_notification, last_notification);
  ASSERT_EQ(comment_list.size(), 1);
  time_t now = last_notification + 10;
  set_time(now);
  clear_service_flap(_service, 10, 80, 10);
  ASSERT_TRUE(comment_list.empty());
}
