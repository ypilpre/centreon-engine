/*
** Copyright 2018 Centreon
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
#include "com/centreon/engine/configuration/applier/hostgroup.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/hostgroup.hh"
#include "com/centreon/engine/notifications/notifiable.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;
extern int config_errors;
extern int config_warnings;

class ApplierHostgroup : public ::testing::Test {
 public:
  void SetUp() {
    config_errors = 0;
    config_warnings = 0;
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a host
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }
};

// Given a hostgroup applier
// And a configuration hostgroup
// When we modify the hostgroup configuration with a non existing
// hostgroup configuration
// Then an exception is thrown.
TEST_F(ApplierHostgroup, ModifyUnexistingHostgroupConfigFromConfig) {
  configuration::applier::hostgroup aply;
  configuration::hostgroup hg("test");
  ASSERT_TRUE(hg.parse("members", "host1"));
  ASSERT_THROW(aply.modify_object(hg), std::exception);
}

// Given a hostgroup applier
// And a configuration hostgroup
// When we modify the hostgroup configuration with a non existing
// hostgroup
// Then an exception is thrown.
TEST_F(ApplierHostgroup, ModifyUnexistingHostgroupFromConfig) {
  configuration::applier::hostgroup aply;
  configuration::hostgroup hg("test");
  ASSERT_TRUE(hg.parse("members", "host1"));
  config->hostgroups().insert(hg);
  ASSERT_THROW(aply.modify_object(hg), std::exception);
}

// Given a hostgroup applier
// And a configuration hostgroup in configuration
// When we modify the hostgroup configuration
// Then the applier modify_object updates the hostgroup.
TEST_F(ApplierHostgroup, ModifyHostgroupFromConfig) {
  configuration::applier::hostgroup aply;
  configuration::hostgroup hg("test");
  ASSERT_TRUE(hg.parse("members", "host1"));
  aply.add_object(hg);
  hostgroup_map::const_iterator
    it(configuration::applier::state::instance().hostgroups().find("test"));
  ASSERT_TRUE(it->second->get_alias() == "test");

  ASSERT_TRUE(hg.parse("alias", "test_renamed"));
  aply.modify_object(hg);
  it = configuration::applier::state::instance().hostgroups().find("test");
  ASSERT_TRUE(it->second->get_alias() == "test_renamed");
}

// Given an empty hostgroup
// When the resolve_object() method is called
// Then no warning, nor error are given
TEST_F(ApplierHostgroup, ResolveEmptyhostgroup) {
  configuration::applier::hostgroup aplyr;
  configuration::hostgroup grp("test");
  aplyr.add_object(grp);
  aplyr.expand_objects(*config);
  aplyr.resolve_object(grp);
  ASSERT_EQ(config_warnings, 0);
  ASSERT_EQ(config_errors, 0);
}

// Given a hostgroup with a non-existing host
// When the resolve_object() method is called
// Then an exception is thrown
// And the method returns 1 error
TEST_F(ApplierHostgroup, ResolveInexistentHost) {
  configuration::applier::hostgroup aplyr;
  configuration::hostgroup grp("test");
  grp.parse("members", "non_existing_host");
  aplyr.add_object(grp);
  aplyr.expand_objects(*config);
  ASSERT_THROW(aplyr.resolve_object(grp), std::exception);
  ASSERT_EQ(config_warnings, 0);
  ASSERT_EQ(config_errors, 1);
}

// Given a hostgroup with a host
// When the resolve_object() method is called
// Then the host is really added to the host group.
TEST_F(ApplierHostgroup, ResolveHostgroup) {
  configuration::applier::host aply;
  configuration::applier::hostgroup aply_grp;
  configuration::hostgroup grp("test_group");
  configuration::host hst("test");
  aply.add_object(hst);
  ASSERT_TRUE(hst.parse("hostgroups", "test_group"));
  grp.parse("members", "test");
  aply_grp.add_object(grp);
  aply_grp.expand_objects(*config);
  ASSERT_NO_THROW(aply_grp.resolve_object(grp));
}

// Given a hostgroup with a host already configured
// And a second hostgroup configuration
// When we set the first one as hostgroup member to the second
// Then the parse method returns true and set the first one host
// to the second one.
TEST_F(ApplierHostgroup, SetHostgroupMembers) {
  configuration::applier::host aply;
  configuration::applier::hostgroup aply_grp;
  configuration::hostgroup grp("test_group");
  configuration::host hst("test");
  aply.add_object(hst);
  grp.parse("members", "test");
  aply_grp.add_object(grp);
  aply_grp.expand_objects(*config);
  aply_grp.resolve_object(grp);
  ASSERT_TRUE(grp.members().size() == 1);

  configuration::hostgroup grp1("big_group");
  ASSERT_TRUE(grp1.parse("hostgroup_members", "test_group"));
  aply_grp.add_object(grp1);
  aply_grp.expand_objects(*config);

  // grp1 must be reload because the expand_objects reload them totally.
  ASSERT_TRUE(config->hostgroups_find("big_group")->members().size() == 1);
}

// Given a hostgroup applier
// And a configuration hostgroup in configuration
// When we remove the configuration
// Then it is really removed
TEST_F(ApplierHostgroup, RemoveHostgroupFromConfig) {
  configuration::applier::host aply;
  configuration::applier::hostgroup aply_grp;
  configuration::hostgroup hg("test_group");
  configuration::host hst("test");
  aply.add_object(hst);
  hg.parse("members", "test");
  aply_grp.add_object(hg);
  aply_grp.expand_objects(*config);
  aply_grp.resolve_object(hg);

  /* The second resolve_object clears members before re-creating them.
   * broker events are then sent. */
  aply_grp.resolve_object(hg);
  ASSERT_TRUE(hg.members().size() == 1);

  aply_grp.remove_object(hg);
  ASSERT_TRUE(
    configuration::applier::state::instance().hostgroups().empty());
}

