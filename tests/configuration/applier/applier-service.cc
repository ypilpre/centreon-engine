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
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ApplierService : public ::testing::Test {
 public:
  void SetUp() {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a contact
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

};

// Given service configuration with an host not defined
// Then the applier add_object throws an exception.
TEST_F(ApplierService, NewServiceWithHostNotDefinedFromConfig) {
  configuration::applier::service svc_aply;
  configuration::service svc;
  ASSERT_TRUE(svc.parse("hosts", "test_host"));
  ASSERT_TRUE(svc.parse("service_description", "test description"));
  svc_aply.add_object(svc);
  ASSERT_EQ(configuration::applier::state::instance().services().size(), 1);
}

// Given service configuration with a host defined
// Then the applier add_object creates the service
TEST_F(ApplierService, NewServiceFromConfig) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  hst_aply.add_object(hst);
  ASSERT_TRUE(svc.parse("host", "test_host"));
  ASSERT_TRUE(svc.parse("service_description", "test description"));
  svc_aply.add_object(svc);
  svc_aply.expand_objects(*config);
  service_map const& sm(configuration::applier::state::instance().services());
  ASSERT_EQ(sm.size(), 1);
  ASSERT_EQ(sm.begin()->first.first, "test_host");
  ASSERT_EQ(sm.begin()->first.second, "test description");
  ASSERT_EQ(sm.begin()->second->get_host_name(), "test_host");
  ASSERT_EQ(sm.begin()->second->get_description(), "test description");
}

// Given a service configuration,
// When we duplicate it, we get a configuration equal to the previous one.
// When two services are generated from the same configuration
// Then they are equal.
// When Modifying a configuration changes,
// Then the '<' effect on configurations.
TEST_F(ApplierService, ServicesEquality) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service csvc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  hst_aply.add_object(hst);
  ASSERT_TRUE(csvc.parse("hosts", "test_host"));
  ASSERT_TRUE(csvc.parse("service_description", "test description"));
  ASSERT_TRUE(csvc.parse("service_id", "12345"));
  ASSERT_TRUE(csvc.parse("acknowledgement_timeout", "21"));
  svc_aply.add_object(csvc);
  ASSERT_TRUE(csvc.parse("host_name", "test_host1"));
  svc_aply.add_object(csvc);
  service_map const& sm(configuration::applier::state::instance().services());
  ASSERT_EQ(sm.size(), 2);
  service_map::const_iterator it(sm.begin());
  shared_ptr<engine::service> svc1(it->second);
  ++it;
  shared_ptr<engine::service> svc2(it->second);
  configuration::service csvc1(csvc);
  ASSERT_EQ(csvc, csvc1);
  ASSERT_TRUE(csvc1.parse("recovery_notification_delay", "120"));
  ASSERT_TRUE(csvc < csvc1);
  ASSERT_TRUE(csvc.parse("retry_interval", "120"));
  ASSERT_TRUE(csvc1 < csvc);

  ASSERT_EQ(svc1, svc1);
  if (svc1->get_host_name() == "test_host1")
    ASSERT_TRUE(svc2.get() < svc1.get());
  else
    ASSERT_TRUE(svc1.get() < svc2.get());
}

// Given a service configuration applied to a service,
// When the check_validity() method is executed on the configuration,
// Then it throws an exception because:
//  1. it does not provide a service description
//  2. it is not attached to a host
//  3. the service does not contain any check command.
TEST_F(ApplierService, ServicesCheckValidity) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service csvc;

  // No service description
  ASSERT_THROW(csvc.check_validity(), engine::error);

  ASSERT_TRUE(csvc.parse("service_description", "test description"));
  // No host attached to
  ASSERT_THROW(csvc.check_validity(), engine::error);

  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  hst_aply.add_object(hst);
  ASSERT_TRUE(csvc.parse("hosts", "test_host"));

  // No check command
  ASSERT_THROW(csvc.check_validity(), engine::error);

  configuration::applier::command cmd_aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  csvc.parse("check_command", "cmd");

  // Check validity OK
  ASSERT_NO_THROW(csvc.check_validity());
}

// Given a service configuration,
// When the flap_detection_options is set to none,
// Then it is well recorded with only none.
TEST_F(ApplierService, ServicesFlapOptionsNone) {
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

  csvc.parse("flap_detection_options", "n");
  ASSERT_EQ(csvc.flap_detection_options(), configuration::service::none);
}

// Given a service configuration,
// When the flap_detection_options is set to all,
// Then it is well recorded with all.
TEST_F(ApplierService, ServicesFlapOptionsAll) {
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

  csvc.parse("flap_detection_options", "a");
  ASSERT_EQ(
    csvc.flap_detection_options(),
      configuration::service::ok
    | configuration::service::warning
    | configuration::service::critical
    | configuration::service::unknown);
}

// Given a service configuration,
// When the initial_state value is set to unknown,
// Then it is well recorded with unknown.
// When the initial_state value is set to whatever
// Then the parse method returns false.
TEST_F(ApplierService, ServicesInitialState) {
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

  ASSERT_TRUE(csvc.parse("initial_state", "u"));
  ASSERT_EQ(csvc.initial_state(), STATE_UNKNOWN);

  ASSERT_FALSE(csvc.parse("initial_state", "g"));
}

// Given a service configuration,
// When the stalking options are set to "c,w",
// Then they are well recorded with "critical | warning"
// When the initial_state value is set to "a"
// Then they are well recorded with "ok | warning | unknown | critical"
TEST_F(ApplierService, ServicesStalkingOptions) {
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

  ASSERT_TRUE(csvc.parse("stalking_options", "c,w"));
  ASSERT_EQ(
    csvc.stalking_options(),
    configuration::service::critical | configuration::service::warning);

  ASSERT_TRUE(csvc.parse("stalking_options", "a"));
  ASSERT_EQ(
    csvc.stalking_options(),
      configuration::service::ok
    | configuration::service::warning
    | configuration::service::unknown
    | configuration::service::critical);
}
