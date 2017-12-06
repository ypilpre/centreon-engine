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
TEST_F(ContactConfig, ContactUpdatedFromConfig) {
  engine::contact* c(engine::contact::add_contact("test"));
  configuration::contact ctct("test");
  ctct.parse("contact_name", "test");
  ctct.parse("alias", "test_alias");
  ctct.parse("email", "test_email");
  ctct.parse("host_notification_commands", "command1,command2");
  ctct.parse("service_notification_commands", "svc_cmd1,svc_cmd2,svc_cmd3");
  c->update_config(ctct);
  ASSERT_EQ(c->get_name(), "test");
  ASSERT_EQ(c->get_alias(), "test_alias");
  ASSERT_EQ(c->get_email(), "test_email");
}
