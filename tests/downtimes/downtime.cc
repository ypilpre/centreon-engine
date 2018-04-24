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
    // Initialization.
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();
    next_downtime_id = 0;

    // Create command.
    configuration::command cmd("cmd");
    cmd.parse("command_line", "echo 1");
    configuration::applier::command cmd_aply;
    cmd_aply.add_object(cmd);
    cmd_aply.expand_objects(*config);
    cmd_aply.resolve_object(cmd);

    // Create host.
    configuration::host hst;
    hst.parse("host_name", "test_host");
    hst.parse("check_command", "cmd");
    configuration::applier::host hst_aply;
    hst_aply.add_object(hst);
    hst_aply.expand_objects(*config);
    hst_aply.resolve_object(hst);

    // Create service.
    configuration::service svc;
    svc.parse("hosts", "test_host");
    svc.parse("service_description", "test description");
    svc.parse("check_command", "cmd");
    configuration::applier::service svc_aply;
    svc_aply.add_object(svc);
    svc_aply.expand_objects(*config);
    svc_aply.resolve_object(svc);
  }

  void TearDown() {
    configuration::applier::state::unload();
    /* downtimes have dependencies on config, so we remove them before the
       config to be cleaned up */
    scheduled_downtime_list.clear();
    delete config;
    config = NULL;
  }

  shared_ptr< ::host> get_test_host() {
    return (configuration::applier::state::instance().hosts_find(
              "test_host"));
  }

  shared_ptr< ::service> get_test_service() {
    return (configuration::applier::state::instance().services_find(
              std::make_pair("test_host", "test description")));
  }
};

// Given a service
// When a fixed downtime is scheduled on it,
// Then a downtime is created with id=1 and no comment.
// And host name and service description are available from the downtime.
// When it is unscheduled, a new comment is added
// And the downtime is removed.
TEST_F(Downtime, SimpleServiceFixedDowntime) {
  shared_ptr<engine::service> svc(get_test_service());
  set_time(20);
  ASSERT_TRUE(svc->schedule_downtime(
    downtime::SERVICE_DOWNTIME,
    time(NULL),
    "admin",
    "test downtime",
    40,
    60,
    true,
    0,
    20) == 0);
  std::auto_ptr<downtime> dt(scheduled_downtime_list.begin()->second);

  ASSERT_TRUE(comment_list.size() == 1);

  ASSERT_TRUE(comment_list.begin()->second->get_comment_data() == "This service has been scheduled for fixed downtime from 01-01-1970 01:00:40 to 01-01-1970 01:01:00. Notifications for the service will not be sent out during that time period.");
  // Only one downtime.
  ASSERT_EQ(dt->get_id(), 1);

  // And host name and service description are available
  engine::service* serv(static_cast<engine::service*>(dt->get_parent()));
  ASSERT_EQ(serv->get_host_name(), "test_host");
  ASSERT_EQ(serv->get_description(), "test description");

  // When it is time to start the downtime
  set_time(41);
  // And the handle method is called, the downtime is in effect
  dt->handle();
  ASSERT_TRUE(dt->get_in_effect());

  set_time(80);
  dt->handle();
  dt->unschedule();
  std::map<unsigned long, comment*>::const_iterator it(comment_list.begin());
  ASSERT_TRUE(comment_list.size() == 1);
  ASSERT_TRUE(it->second->get_comment_data() == "This service has been scheduled for fixed downtime from 01-01-1970 01:00:40 to 01-01-1970 01:01:00. Notifications for the service will not be sent out during that time period.");
  ++it;
  ASSERT_TRUE(it == comment_list.end());
  ASSERT_TRUE(scheduled_downtime_list.empty());
}

// Given a service
// When a downtime is scheduled on it,
// Then a downtime is created with id=1 and no comment.
// And host name and service description are available from the downtime.
// When it is unscheduled, a new comment is added
// And the downtime is removed.
TEST_F(Downtime, SimpleServiceFlexibleDowntime) {
  shared_ptr<engine::service> svc(get_test_service());

  // No comment for now.
  ASSERT_TRUE(comment_list.empty());
  set_time(20);
  ASSERT_TRUE(svc->schedule_downtime(
    downtime::SERVICE_DOWNTIME,
    time(NULL),
    "admin",
    "test downtime",
    40, 60,
    false,
    0, 120) == 0);
  std::auto_ptr<downtime> dt(scheduled_downtime_list.begin()->second);

  // Only one downtime.
  ASSERT_EQ(dt->get_id(), 1);

  // And host name and service description are available
  engine::service* s(static_cast<engine::service*>(dt->get_parent()));
  ASSERT_EQ(s->get_host_name(), "test_host");
  ASSERT_EQ(s->get_description(), "test description");

  std::map<unsigned long, comment*>::const_iterator it(comment_list.begin());
  ASSERT_TRUE(it != comment_list.end());
  ASSERT_TRUE(it->second->get_comment_data() == "This service has been scheduled for flexible downtime starting between 01-01-1970 01:00:40 and 01-01-1970 01:01:00 and lasting for a period of 0 hours and 2 minutes. Notifications for the service will not be sent out during that time period.");
  ++it;
  ASSERT_TRUE(it == comment_list.end());

  // When it is time to start the downtime
  set_time(41);
  // And the handle method is called, the downtime is in effect
  svc->set_current_state(2);
  dt->handle();
  ASSERT_TRUE(dt->get_in_effect());

  set_time(80);
  dt->handle();
  dt->unschedule();
  ASSERT_TRUE(scheduled_downtime_list.empty());
}

// Given a host
// When a fixed downtime is scheduled on it,
// Then a downtime is created with id=1.
// And host name is available from the downtime.
// When it is unscheduled, a new comment is added
// And the downtime is removed.
TEST_F(Downtime, SimpleHostFixedDowntime) {
  shared_ptr<engine::host> hst(get_test_host());
  set_time(20);
  ASSERT_TRUE(hst->schedule_downtime(
    downtime::HOST_DOWNTIME,
    time(NULL),
    "admin",
    "test downtime",
    40, 60,
    true,
    0, 20) == 0);
  std::auto_ptr<downtime> dt(scheduled_downtime_list.begin()->second);

  // Only one downtime.
  ASSERT_EQ(dt->get_id(), 1);
  // One comment for now.
  std::map<unsigned long, comment*>::const_iterator it(comment_list.begin());
  ASSERT_TRUE(it != comment_list.end());
  ASSERT_TRUE(it->second->get_comment_data() == "This host has been scheduled for fixed downtime from 01-01-1970 01:00:40 to 01-01-1970 01:01:00. Notifications for the host will not be sent out during that time period.");
  ++it;
  ASSERT_TRUE(it == comment_list.end());

  // And host name and service description are available
  engine::host* h(static_cast<engine::host*>(dt->get_parent()));
  ASSERT_EQ(h->get_name(), "test_host");

  // When it is time to start the downtime
  set_time(41);
  // And the handle method is called, the downtime is in effect
  dt->handle();
  ASSERT_TRUE(dt->get_in_effect());

  set_time(80);
  dt->handle();
  dt->unschedule();
  ASSERT_TRUE(scheduled_downtime_list.empty());
}

// Given a host
// When a flexible downtime is scheduled on it,
// And the host is non-ok
// Then a downtime is created with id=1.
// And host name is available from the downtime.
// When it is unscheduled, a new comment is added
// And the downtime is removed.
TEST_F(Downtime, SimpleHostFlexibleDowntime) {
  shared_ptr<engine::host> hst(get_test_host());

  // No comment for now.
  ASSERT_TRUE(comment_list.empty());
  set_time(20);
  ASSERT_TRUE(hst->schedule_downtime(
    downtime::HOST_DOWNTIME,
    time(NULL),
    "admin",
    "test downtime",
    40, 60,
    false,
    0, 120) == 0);
  std::auto_ptr<downtime> dt(scheduled_downtime_list.begin()->second);

  // Only one downtime.
  ASSERT_EQ(dt->get_id(), 1);

  // And host name and service description are available
  engine::host* h(static_cast<engine::host*>(dt->get_parent()));
  ASSERT_EQ(h->get_name(), "test_host");

  std::map<unsigned long, comment*>::const_iterator it(comment_list.begin());
  ASSERT_TRUE(it != comment_list.end());
  ASSERT_TRUE(it->second->get_comment_data() == "This host has been scheduled for flexible downtime starting between 01-01-1970 01:00:40 and 01-01-1970 01:01:00 and lasting for a period of 0 hours and 2 minutes. Notifications for the host will not be sent out during that time period.");
  ++it;
  ASSERT_TRUE(it == comment_list.end());

  // When it is time to start the downtime
  set_time(41);
  // And the handle method is called, the downtime is in effect
  hst->set_current_state(2);
  dt->handle();
  ASSERT_TRUE(dt->get_in_effect());

  set_time(80);
  dt->handle();
  dt->unschedule();
  ASSERT_TRUE(scheduled_downtime_list.empty());
}

// Given a host
// When a flexible downtime is scheduled on it,
// Then no downtime is created while the host is in OK state
// When the host state changes between start and end time
// Then a downtime is created with id=1.
// And host name is available from the downtime.
// When it is unscheduled, a new comment is added
// And the downtime is removed.
TEST_F(Downtime, SimpleHostFlexibleDowntime2) {
  shared_ptr<engine::host> hst(get_test_host());

  // No comment for now.
  ASSERT_TRUE(comment_list.empty());
  hst->set_current_state(0);
  set_time(20);
  ASSERT_TRUE(hst->schedule_downtime(
    downtime::HOST_DOWNTIME,
    time(NULL),
    "admin",
    "test downtime",
    40, 60,
    false,
    0, 120) == 0);
  std::auto_ptr<downtime> dt(scheduled_downtime_list.begin()->second);

  // Only one downtime.
  ASSERT_EQ(dt->get_id(), 1);

  // And host name and service description are available
  engine::host* h(static_cast<engine::host*>(dt->get_parent()));
  ASSERT_EQ(h->get_name(), "test_host");

  std::map<unsigned long, comment*>::const_iterator it(comment_list.begin());
  ASSERT_TRUE(it != comment_list.end());
  ASSERT_TRUE(it->second->get_comment_data() == "This host has been scheduled for flexible downtime starting between 01-01-1970 01:00:40 and 01-01-1970 01:01:00 and lasting for a period of 0 hours and 2 minutes. Notifications for the host will not be sent out during that time period.");
  ++it;
  ASSERT_TRUE(it == comment_list.end());

  // When it is time to start the downtime
  set_time(41);
  dt->handle();
  ASSERT_FALSE(dt->get_in_effect());

  set_time(60);
  hst->set_current_state(2);
  dt->handle();
  ASSERT_TRUE(dt->get_in_effect());

  dt->unschedule();

  it = comment_list.begin();
  ASSERT_TRUE(it != comment_list.end());
  ASSERT_TRUE(it->second->get_comment_data() == "This host has been scheduled for flexible downtime starting between 01-01-1970 01:00:40 and 01-01-1970 01:01:00 and lasting for a period of 0 hours and 2 minutes. Notifications for the host will not be sent out during that time period.");
  ++it;
  ASSERT_TRUE(it == comment_list.end());

  ASSERT_TRUE(scheduled_downtime_list.empty());
}

// Given a service
// When a downtime is scheduled on it,
// Then a downtime is created with id=1 and no comment.
// And host name and service description are available from the downtime.
// When it is unscheduled, a new comment is added
// And the downtime is removed.
TEST_F(Downtime, TriggeredDowntime) {
  shared_ptr<engine::service> svc(get_test_service());

  // No comment for now.
  ASSERT_TRUE(comment_list.empty());
  set_time(20);
  ASSERT_TRUE(svc->schedule_downtime(
    downtime::SERVICE_DOWNTIME,
    time(NULL),
    "admin",
    "test downtime",
    40, 60,
    false,
    0, 120) == 0);

  // Here we define a downtime triggered by the previous one.
  ASSERT_TRUE(svc->schedule_downtime(
    downtime::SERVICE_DOWNTIME,
    time(NULL),
    "admin",
    "test downtime",
    60, 70,
    false,
    1, 120) == 0);

  std::map<unsigned long, downtime*>::iterator it(
    scheduled_downtime_list.begin());
  std::auto_ptr<downtime> dt1(it->second);
  ++it;
  std::auto_ptr<downtime> dt2(it->second);

  // When it is time to start the downtime
  set_time(41);
  // And the handle method is called, the downtime is in effect
  svc->set_current_state(2);
  dt1->handle();
  dt2->handle();
  ASSERT_TRUE(dt1->get_in_effect());
  ASSERT_TRUE(dt2->get_in_effect());

  set_time(80);
  //dt1->handle();
  dt1->unschedule();
  ASSERT_TRUE(scheduled_downtime_list.empty());
}
