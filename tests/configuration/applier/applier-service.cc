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
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/engine/notifications/notifier.hh"
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

// Given contactgroup / contact appliers
// And a configuration contactgroup and a configuration contact
// Then the appliers add_object add the contact group and the contact.
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
