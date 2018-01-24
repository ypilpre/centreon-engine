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
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/contactgroup.hh"
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;
extern int config_errors;
extern int config_warnings;

class ApplierContactgroup : public ::testing::Test {
 public:
  void SetUp() {
    config_errors = 0;
    config_warnings = 0;
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

// Given a contactgroup applier and a configuration contactgroup
// Then the add_object() of the applier creates the contactgroup.
TEST_F(ApplierContactgroup, NewContactgroupFromConfig) {
  configuration::applier::contactgroup aplyr;
  configuration::contactgroup grp("test_group");
  aplyr.add_object(grp);
  contactgroup_map const& cgs(configuration::applier::state::instance().contactgroups());
  ASSERT_EQ(cgs.size(), 1);
}

// Given an empty contactgroup
// When the resolve_object() method is called
// Then no warning, nor error are given
TEST_F(ApplierContactgroup, ResolveEmptyContactgroup) {
  configuration::applier::contactgroup aplyr;
  configuration::contactgroup grp("test");
  aplyr.add_object(grp);
  aplyr.expand_objects(*config);
  aplyr.resolve_object(grp);
  ASSERT_EQ(config_warnings, 0);
  ASSERT_EQ(config_errors, 0);
}

// Given a contactgroup with a non-existing contact
// When the resolve_object() method is called
// Then an exception is thrown
// And the method returns 1 error
TEST_F(ApplierContactgroup, ResolveInexistentContact) {
  configuration::applier::contactgroup aplyr;
  configuration::contactgroup grp("test");
  grp.parse("members", "non_existing_contact");
  aplyr.add_object(grp);
  aplyr.expand_objects(*config);
  ASSERT_THROW(aplyr.resolve_object(grp), std::exception);
  ASSERT_EQ(config_warnings, 0);
  ASSERT_EQ(config_errors, 1);
}
