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
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/configuration/state.hh"
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

// Given an empty name
// When the add_contact function is called with it as argument,
// Then it returns a NULL pointer.
TEST_F(ApplierContact, NewContactFromConfig) {
  configuration::applier::contact aply;
  configuration::contact ctct("test");
  ASSERT_TRUE(ctct.parse("host_notification_commands", "cmd1,cmd2"));
  aply.add_object(ctct);
  contact_set const& sc(configuration::applier::state::instance().contacts());
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
