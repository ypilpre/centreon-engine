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
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contactgroup.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/notifications/notifiable.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ConfigContactgroup : public ::testing::Test {
 public:
  void SetUp() {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a contactgroup
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

};

// When I create a configuration::contactgroup with an empty name
// Then an exception is thrown.
TEST_F(ConfigContactgroup, NewContactgroupWithNoName) {
  configuration::contactgroup cg("");
  ASSERT_THROW(cg.check_validity(), std::exception);
}

// Given a configuration::contactgroup
// When I set one of its attribute
// Then I can retrieve it.
// When I copy the contactgroup
// Then they are equal.
// When I change some of the parameters of one or the other
// Then comparaison between contactgroups works as expected.
// When I merge a second contactgroup into a first contactgroup,
// Then the final contactgroup is filled as expected.
TEST_F(ConfigContactgroup, NewContactFromContact) {
  configuration::contactgroup cg("test");
  ASSERT_TRUE(cg.parse("contactgroup_name", "test_contactgroup_name"));
  ASSERT_EQ(cg.contactgroup_name(), "test_contactgroup_name");
  ASSERT_EQ(cg.contactgroup_name(), cg.key());
  ASSERT_TRUE(cg.parse("alias", "test_alias"));
  ASSERT_EQ(cg.alias(), "test_alias");
  ASSERT_TRUE(cg.parse("contactgroup_members", "contact1,contact2"));
  {
    std::set<std::string> myset;
    myset.insert("contact1");
    myset.insert("contact2");
    ASSERT_EQ(cg.contactgroup_members(), myset);
  }
  ASSERT_TRUE(cg.parse("members", "ct1,ct2"));
  {
    std::set<std::string> myset;
    myset.insert("ct1");
    myset.insert("ct2");
    ASSERT_EQ(cg.members(), myset);
  }

  configuration::contactgroup cg_copy(cg);

  ASSERT_EQ(cg, cg_copy);
  ASSERT_FALSE(cg != cg_copy);

  cg_copy.parse("members", "ct1,yy2,zz3");
  ASSERT_TRUE(cg < cg_copy);

  cg.parse("contactgroup_members", "contact1,contact2,contact3");
  ASSERT_TRUE(cg_copy < cg);

  cg.parse("contactgroup_name", "test_name1");
  ASSERT_TRUE(cg_copy < cg);

  set_string s;
  s.insert("ct1");
  s.insert("ct2");
  s.insert("zz3");
  cg.merge(cg_copy);
  for (set_string::const_iterator
         cit(cg.members().begin()),
         cend(cg.members().end()),
         sit(s.begin()),
         send(s.end());
       cit != cend && sit != send;
       ++cit, ++sit) {
    ASSERT_TRUE(*cit == *sit);
  }
}
