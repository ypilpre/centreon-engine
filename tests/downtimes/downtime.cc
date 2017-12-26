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
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class Downtime : public ::testing::Test {
 public:
  void SetUp() {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();
    configuration::applier::host hst_aply;
    configuration::applier::service svc_aply;
    configuration::service csvc;
    ASSERT_TRUE(csvc.parse("service_description", "test description"));
    configuration::host hst;
    ASSERT_TRUE(hst.parse("host_name", "test_host"));
    hst_aply.add_object(hst);
    ASSERT_TRUE(csvc.parse("hosts", "test_host"));
    configuration::applier::command cmd_aply;
    configuration::command cmd("cmd");
    cmd.parse("command_line", "echo 1");
    csvc.parse("check_command", "cmd");
    svc_aply.add_object(csvc);
  }

  void TearDown() {
    configuration::applier::state::unload();
    /* downtimes have dependencies on config, so we remove them before the
       config to be cleaned up */
    scheduled_downtime_list.clear();
    delete config;
    config = NULL;
  }
};

// Given a service in downtime
// When a downtime is scheduled,
// Then a downtime is created with id=1 and no comment.
// And host name and service description are available from the downtime.
TEST_F(Downtime, SimpleServiceDowntime) {
  shared_ptr<engine::service> svc(find_service(
                                    "test_host",
                                    "test description"));
  set_time(20);
  ASSERT_TRUE(svc->schedule_downtime(
    downtime::SERVICE_DOWNTIME,
    time(NULL),
    "admin",
    "test downtime",
    40, 60,
    true,
    0, 20) == 0);
  std::auto_ptr<downtime> dt(scheduled_downtime_list.begin()->second);

  // Only one downtime.
  ASSERT_EQ(dt->get_id(), 1);
  // No comment for now.
  ASSERT_EQ(dt->get_comment_id(), 0);

  // And host name and service description are available
  ASSERT_EQ(dt->get_host_name(), "test_host");
  ASSERT_EQ(dt->get_service_description(), "test description");

  // When it is time to start the downtime
  set_time(41);
  // And the handle method is called, the downtime is in effect
  dt->handle();
  ASSERT_EQ(dt->get_in_effect(), true);

  set_time(80);
  dt->handle();
  dt->unschedule();
  comment* cm = comment_list;
  ASSERT_TRUE(strcmp(cm->comment_data, "This service has been scheduled for fixed downtime from 01-01-1970 01:00:40 to 01-01-1970 01:01:00. Notifications for the service will not be sent out during that time period.") == 0);
  cm = cm->next;
  ASSERT_TRUE(cm == NULL);
  ASSERT_TRUE(scheduled_downtime_list.empty());
}
