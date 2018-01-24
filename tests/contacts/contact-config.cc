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
#include "com/centreon/engine/commands/raw.hh"
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

class ContactConfig : public ::testing::Test {
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

// Given an empty name
// When a new is created with it as argument,
// Then it returns a NULL pointer.
TEST_F(ContactConfig, NewContactWithNoName) {
  configuration::contact ctct("");
  ASSERT_THROW(new engine::contact(ctct), std::exception);
}

// Given a non empty name
// When a new contact is created with it, but no alias is given
// Then the contact is well created and its alias equals its name.
TEST_F(ContactConfig, ContactWithoutAlias) {
  configuration::contact ctct("test");
  std::auto_ptr<engine::contact> c(new engine::contact(ctct));
  ASSERT_EQ(c->get_name(), "test");
  ASSERT_EQ(c->get_alias(), "test");
}

// Given a configuration contact
// When a contact is created from it
// Then it inherits its properties.
TEST_F(ContactConfig, ContactFromConfig) {
  configuration::contact ctct("test");
  ctct.parse("contact_name", "test");
  ctct.parse("alias", "test_alias");
  ctct.parse("email", "test_email");
  ctct.parse("host_notification_commands", "command1,command2");
  ctct.parse("service_notification_commands", "svc_cmd1,svc_cmd2,svc_cmd3");
  ctct.parse("pager", "test pager");
  std::auto_ptr<engine::contact> c(new engine::contact(ctct));
  ASSERT_EQ(c->get_name(), "test");
  ASSERT_EQ(c->get_alias(), "test_alias");
  ASSERT_EQ(c->get_email(), "test_email");
  ASSERT_EQ(c->get_pager(), "test pager");
}
