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
#include "com/centreon/engine/commands/set.hh"
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

class ApplierContactgroup : public ::testing::Test {
 public:
  void SetUp() {
//    set_time(20);
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a contact
    commands::set::load();
  }

  void TearDown() {
    commands::set::unload();
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

};

// Given a contactgroup applier and a configuration contactgroup
// Then the add_object() of the applier creates the contactgroup.
TEST_F(ApplierContactgroup, NewContactgroupFromConfig) {
  configuration::applier::contactgroup aply_grp;
  configuration::contactgroup grp("test_group");
  aply_grp.add_object(grp);
  contactgroup_map const& cgs(configuration::applier::state::instance().contactgroups());
  ASSERT_EQ(cgs.size(), 1);
}

// Given a contactgroup applier and a configuration contactgroup
// Then the add_object() of the applier creates the contactgroup.
TEST_F(ApplierContactgroup, ContactgroupAddContact) {
//  configuration::applier::contactgroup aply_grp;
//  configuration::applier::command aply_cmd;
//  configuration::applier::contact aply;
//
//  configuration::contactgroup grp("test_group");
//  configuration::contact ct("test_contact");
//  configuration::command cmd("test_cmd");
//  ct.parse("host_notification_command", "test_cmd");
//  ct.parse("service_notification_command", "test_cmd");
//  cmd.parse("command_name", "test_cmd");
//  cmd.parse("command_line", "echo 'hello'");
//  aply_grp.add_object(grp);
//  aply_cmd.add_object(cmd);
//  aply.add_object(ct);
//  ASSERT_THROW(aply.resolve_object(ct), std::exception);
}
