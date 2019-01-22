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

class AcknowledgementNotification : public ::testing::Test {
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
    contact_map& cm = configuration::applier::state::instance().contacts();
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
  _service->set_current_state(0);
  _service->set_last_state_change(time(NULL));
  _service->set_notifications_enabled(true);
  time_t last_notification = _service->get_last_notification();
  _service->set_notify_on(notifier::ON_RECOVERY, true);
  _service->set_notify_on(notifier::ON_WARNING, true);
  _service->set_notify_on(notifier::ON_CRITICAL, true);
  // When the service is set in hard WARNING
  time_t now = time(NULL) + 20;
  set_time(now);
  _service->set_current_state(1);
  _service->set_current_state_type(HARD_STATE);
  _service->notify(notifier::PROBLEM, "admin", "Test comment");
  // Then a notification is sent
  ASSERT_GE(_service->get_last_notification(), now);

  // When the service is acknowledged (normally)
  now += 20;
  set_time(now);
  _service->set_acknowledged(notifier::ACKNOWLEDGEMENT_NORMAL);
  _service->set_last_acknowledgement(now);
  // Then no more notification is sent while the state is WARNING
  _service->notify(notifier::ACKNOWLEDGEMENT, "admin", "Test comment");
  ASSERT_GE(_service->get_last_notification(), now);
  now += 20;
  set_time(now);
  _service->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_FALSE(_service->get_last_notification() >= now);

  // When the service is set to hard CRITICAL
  now += _service->get_notification_interval() + 1;
  set_time(now);
  _service->set_current_state(1);
  _service->set_last_state(1);
  _service->set_last_hard_state(1);
  _service->set_last_state_change(now);
  _service->set_last_hard_state_change(now);
  _service->set_last_check(now);
  _service->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_LT(_service->get_last_notification(), now);

  // When the service is set to hard CRITICAL
  now += 20;
  set_time(now);
  _service->set_current_state(2);
  _service->set_last_hard_state(2);
  _service->set_last_state_change(now);
  _service->set_last_hard_state_change(now);
  _service->set_last_check(now);
  _service->update_acknowledgement_on_state_change();
  // Then the acknowledgement is removed and a notification is sent.
  _service->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_FALSE(_service->is_acknowledged());
  ASSERT_GE(_service->get_last_notification(), now);

  // When the service is set to OK,
  now += 20;
  set_time(now);
  _service->set_current_state(0);
  _service->set_last_state_change(now);
  _service->set_last_check(now);
  // Then a recovery notification is sent.
  _service->notify(notifier::RECOVERY, "admin", "Test comment");
  ASSERT_GE(_service->get_last_notification(), now);
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
  _service->set_current_state(0);
  _service->set_last_state_change(time(NULL));
  _service->set_notifications_enabled(true);
  time_t last_notification = _service->get_last_notification();
  _service->set_notify_on(notifier::ON_RECOVERY, true);
  _service->set_notify_on(notifier::ON_WARNING, true);
  _service->set_notify_on(notifier::ON_CRITICAL, true);
  // When the service is set in hard WARNING
  time_t now = time(NULL) + 20;
  set_time(now);
  _service->set_current_state(1);
  _service->set_current_state_type(HARD_STATE);
  _service->notify(notifier::PROBLEM, "admin", "Test comment");
  // Then a notification is sent
  ASSERT_GE(_service->get_last_notification(), now);

  // When the service is acknowledged (normally)
  now += 20;
  set_time(now);
  _service->set_acknowledged(notifier::ACKNOWLEDGEMENT_STICKY);
  _service->set_last_acknowledgement(now);
  // Then no more notification is sent while the state is WARNING
  _service->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_FALSE(_service->get_last_notification() >= now);

  now += _service->get_notification_interval() + 1;
  set_time(now);
  _service->set_current_state(1);
  _service->set_last_state(1);
  _service->set_last_hard_state(1);
  _service->set_last_state_change(now);
  _service->set_last_hard_state_change(now);
  _service->set_last_check(now);
  _service->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_FALSE(_service->get_last_notification() >= now);

  // When the service is set to hard CRITICAL
  now += 20;
  set_time(now);
  _service->set_current_state(2);
  _service->set_last_hard_state(2);
  _service->set_last_state_change(now);
  _service->set_last_hard_state_change(now);
  _service->set_last_check(now);
  // Then no more notification is sent while the state is WARNING
  _service->notify(notifier::PROBLEM, "admin", "Test comment");
  ASSERT_FALSE(_service->get_last_notification() >= now);

  // When the service is set to OK,
  now += 20;
  set_time(now);
  _service->set_current_state(0);
  _service->set_last_state_change(now);
  _service->set_last_check(now);
  _service->update_acknowledgement_on_state_change();
  // Then the acknowledgement is removed and a recovery notification is sent.
  _service->notify(notifier::RECOVERY, "admin", "Test comment");
  ASSERT_FALSE(_service->is_acknowledged());
  ASSERT_TRUE(_service->get_last_notification() >= now);
}
