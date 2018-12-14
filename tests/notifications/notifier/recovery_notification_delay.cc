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
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/process_manager.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::notifications;

extern configuration::state* config;

class RecoveryNotificationDelay : public ::testing::Test {
 public:
  void SetUp() {
    process_manager::load();
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();
    config->enable_notifications(true);

    {
      char tmp[] = "/tmp/engine_recovery_notif_delayXXXXXX";
      _file = tmpnam(tmp);
    }

    configuration::command cmd("test_cmd");
    {
      std::string cmdline;
      cmdline = "touch ";
      cmdline.append(_file);
      cmd.parse("command_line", cmdline.c_str());
    }
    configuration::applier::command cmd_aply;
    cmd_aply.add_object(cmd);
    cmd_aply.resolve_object(cmd);

    configuration::contact cntct("test_contact");
    cntct.parse("host_notifications_enabled", "1");
    cntct.parse("host_notification_options", "-1");
    cntct.parse("host_notification_commands", "test_cmd");
    cntct.parse("service_notifications_enabled", "1");
    cntct.parse("service_notification_options", "-1");
    cntct.parse("service_notification_commands", "test_cmd");
    configuration::applier::contact cntct_aply;
    cntct_aply.add_object(cntct);
    cntct_aply.resolve_object(cntct);

    configuration::host hst;
    hst.parse("host_name", "test_host");
    hst.parse("check_command", "test_cmd");
    hst.parse("contacts", "test_contact");
    configuration::applier::host hst_aply;
    hst_aply.add_object(hst);
    hst_aply.resolve_object(hst);

    configuration::service csvc;
    csvc.parse("service_description", "test_service");
    csvc.parse("host_name", "test_host");
    csvc.parse("check_command", "test_cmd");
    csvc.parse("recovery_notification_delay", "10");
    csvc.parse("contacts", "test_contact");
    configuration::applier::service svc_aply;
    svc_aply.add_object(csvc);
    svc_aply.resolve_object(csvc);

    _service = configuration::applier::state::instance()
                 .services().begin()->second.get();
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
    process_manager::unload();
    ::remove(_file.c_str());
  }

 protected:
  std::string       _file;
  engine::service*  _service;
};

// Given a service in CRITICAL state
// And the recovery notification delay is not elapsed
// When service is back to OK state
// Then no notification is sent
TEST_F(RecoveryNotificationDelay, NotElapsed) {
  // Given
  _service->set_current_state_type(HARD_STATE);
  _service->set_current_state(2);
  _service->add_notification_flag(notifier::PROBLEM);
  _service->set_current_notification_number(1);
  _service->set_first_notification(15);
  _service->set_last_notification(15);
  set_time(20);

  // When
  _service->set_current_state(0);
  _service->notify(notifier::RECOVERY, "admin", "Test comment");

  // Then
  std::ifstream ifs(_file.c_str());
  ASSERT_FALSE(ifs.good());
}

// Given a service in CRITICAL state
// And the recovery notification is elapsed
// When service is back to OK state
// Then a notification is sent
TEST_F(RecoveryNotificationDelay, Elapsed) {
  // Given
  _service->set_current_state_type(HARD_STATE);
  _service->set_current_state(2);
  _service->add_notification_flag(notifier::PROBLEM);
  _service->set_current_notification_number(1);
  _service->set_first_notification(10);
  _service->set_last_notification(15);
  set_time(20);

  // When
  _service->set_current_state(0);
  _service->notify(notifier::RECOVERY, "admin", "Test comment");

  // Then
  std::ifstream ifs(_file.c_str());
  ASSERT_TRUE(ifs.good());
}
