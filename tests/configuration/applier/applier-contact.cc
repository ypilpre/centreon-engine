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

class ApplierContact : public ::testing::Test {
 public:
  void SetUp() {
//    set_time(20);
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

// Given contactgroup / contact appliers
// And a configuration contactgroup and a configuration contact
// Then the appliers add_object add the contact group and the contact.
TEST_F(ApplierContact, NewContactFromConfig) {
  configuration::applier::contact aply;
  configuration::applier::contactgroup aply_grp;
  configuration::contactgroup grp("test_group");
  configuration::contact ctct("test");
  ASSERT_TRUE(ctct.parse("contactgroups", "test_group"));
  ASSERT_TRUE(ctct.parse("host_notification_commands", "cmd1,cmd2"));
  aply_grp.add_object(grp);
  aply.add_object(ctct);
  aply.expand_objects(*config);
  contact_map const& sc(configuration::applier::state::instance().contacts());
  ASSERT_EQ(sc.size(), 1);
  ASSERT_EQ(sc.begin()->first, "test");
  ASSERT_EQ(sc.begin()->second->get_name(), "test");
  engine::contact* cc(sc.begin()->second.get());
  command_set::const_iterator
         it(cc->get_host_notification_commands().begin()),
         end(cc->get_host_notification_commands().end());
  ASSERT_EQ(it->first, "cmd1");
  ++it;
  ASSERT_EQ(it->first, "cmd2");
}

// Given a contact applier
// And a configuration contact
// When we modify the contact configuration with an unexisting contact
// configuration
// Then an exception is thrown.
TEST_F(ApplierContact, ModifyUnexistingContactConfigFromConfig) {
  configuration::applier::contact aply;
  configuration::contact ctct("test");
  ASSERT_TRUE(ctct.parse("contactgroups", "test_group"));
  ASSERT_TRUE(ctct.parse("host_notification_commands", "cmd1,cmd2"));
  ASSERT_THROW(aply.modify_object(ctct), std::exception);
}

// Given a contact applier
// And a configuration contact
// When we modify the contact configuration with an unexisting contact
// Then an exception is thrown.
TEST_F(ApplierContact, ModifyUnexistingContactFromConfig) {
  configuration::applier::contact aply;
  configuration::contact ctct("test");
  ASSERT_TRUE(ctct.parse("contactgroups", "test_group"));
  ASSERT_TRUE(ctct.parse("host_notification_commands", "cmd1,cmd2"));
  config->contacts().insert(ctct);
  ASSERT_THROW(aply.modify_object(ctct), std::exception);
}

// Given contactgroup / contact appliers
// And a configuration contactgroup and a configuration contact
// that are already in configuration
// When we change the contact configuration
// Then the appliers modify_object updates the contact.
TEST_F(ApplierContact, ModifyContactFromConfig) {
  configuration::applier::contact aply;
  configuration::applier::contactgroup aply_grp;
  configuration::contactgroup grp("test_group");
  configuration::contact ctct("test");
  ASSERT_TRUE(ctct.parse("contactgroups", "test_group"));
  ASSERT_TRUE(ctct.parse("host_notification_commands", "cmd1,cmd2"));
  aply_grp.add_object(grp);
  aply.add_object(ctct);
  aply.expand_objects(*config);
  ASSERT_TRUE(ctct.parse("alias", "test_alias"));
  ASSERT_TRUE(ctct.parse("host_notification_commands", "host_cmd1,host_cmd2"));
  ASSERT_TRUE(ctct.parse("service_notification_commands", "svc_cmd1,svc_cmd2"));
  ASSERT_TRUE(ctct.parse("_cust", "MyValue"));
  aply.modify_object(ctct);
  contact_map::const_iterator
    it(configuration::applier::state::instance().contacts_find("test"));
  ASSERT_TRUE(it->second->get_alias() == "test_alias");
}

// Given contactgroup / contact appliers
// And a configuration contactgroup and a configuration contact
// that are already in configuration
// When we change the contact configuration
// Then the appliers modify_object updates the contact.
TEST_F(ApplierContact, RemoveContactFromConfig) {
  configuration::applier::contact aply;
  configuration::applier::contactgroup aply_grp;
  configuration::contactgroup grp("test_group");
  configuration::contact ctct("test");
  ASSERT_TRUE(ctct.parse("contactgroups", "test_group"));
  ASSERT_TRUE(ctct.parse("host_notification_commands", "cmd1,cmd2"));
  ASSERT_TRUE(ctct.parse("service_notification_commands", "svc1,svc2"));
  ASSERT_TRUE(ctct.parse("_superVar", "superValue"));
  aply_grp.add_object(grp);
  aply.add_object(ctct);
  aply.expand_objects(*config);
  aply.remove_object(ctct);
  ASSERT_TRUE(configuration::applier::state::instance().contacts().empty());
}

// Given contactgroup / contact appliers
// And a configuration contactgroup and a configuration contact
// that are already in configuration
// When we resolve the contact configuration
// Then the contact contactgroups is cleared, nothing more if the
// contact check is OK. Here, since notification commands are empty,
// an exception is thrown.
TEST_F(ApplierContact, ResolveContactFromConfig) {
  configuration::applier::contact aply;
  configuration::applier::contactgroup aply_grp;
  configuration::contactgroup grp("test_group");
  configuration::contact ctct("test");
  ASSERT_TRUE(ctct.parse("contactgroups", "test_group"));
  ASSERT_TRUE(ctct.parse("host_notification_commands", "cmd1,cmd2"));
  aply_grp.add_object(grp);
  aply.add_object(ctct);
  aply.expand_objects(*config);
  ASSERT_THROW(aply.resolve_object(ctct), std::exception);
}
