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
// When a contact has this configuration applied to itself
// Then it inherits its properties.
TEST_F(ContactConfig, ContactUpdatedFromConfig) {
  configuration::contact ctct("test");
  std::auto_ptr<engine::contact> c(new engine::contact(ctct));
  ctct.parse("contact_name", "test");
  ctct.parse("alias", "test_alias");
  ctct.parse("email", "test_email");
  ctct.parse("host_notification_commands", "command1,command2");
  ctct.parse("service_notification_commands", "svc_cmd1,svc_cmd2,svc_cmd3");
  ctct.parse("pager", "test pager");
  c->update_config(ctct);
  ASSERT_EQ(c->get_name(), "test");
  ASSERT_EQ(c->get_alias(), "test_alias");
  ASSERT_EQ(c->get_email(), "test_email");
  ASSERT_EQ(c->get_pager(), "test pager");
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

// Given a contact
// When the check method is executed
// Then 2 warnings and 2 errors are returned:
//  * error 1 => no service notification command
//  * error 2 => no host notification command
//  * warning 1 => no service notification period
//  * warning 2 => no host notification period
// And the method returns false (errors number != 0)
TEST_F(ContactConfig, ContactCheck) {
  configuration::contact ctct("test");
  std::auto_ptr<engine::contact> c(new engine::contact(ctct));
  int w = 0, e = 0;
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 2);
  ASSERT_EQ(e, 2);
}

// Given a contact
// When adding a service notification period by its name without a corresponding
// notification period definition
// Then the check method returns false with 1 warning and 3 errors.
TEST_F(ContactConfig, AddServiceNotificationTimeperiod) {
  configuration::contact ctct("test");
  ASSERT_TRUE(ctct.parse("service_notification_period", "notif_period"));
  std::auto_ptr<engine::contact> c(new engine::contact(ctct));
  int w = 0, e = 0;
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 1);
  ASSERT_EQ(e, 3);
}

// Given a contact
// When adding a host notification period by its name without a corresponding
// notification period definition
// Then the check method returns false with 1 warning and 3 errors.
TEST_F(ContactConfig, AddHostNotificationTimeperiod) {
  configuration::contact ctct("test");
  ASSERT_TRUE(ctct.parse("host_notification_period", "notif_period"));
  std::auto_ptr<engine::contact> c(new engine::contact(ctct));
  int w = 0, e = 0;
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 1);
  ASSERT_EQ(e, 3);
}

// Given a contact
// When adding a host notification command with a null/empty name
// Then an exception is thrown.
TEST_F(ContactConfig, AddHostNotificationCommand) {
  configuration::contact ctct("test");
  std::auto_ptr<engine::contact> c(new engine::contact(ctct));
  ASSERT_THROW(c->add_host_notification_command(NULL), std::exception);
  ASSERT_THROW(c->add_host_notification_command(""), std::exception);
}

// Given a contact
// When adding a service notification command with an null/empty name
// Then an exception is thrown.
TEST_F(ContactConfig, AddServiceNotificationCommand) {
  configuration::contact ctct("test");
  std::auto_ptr<engine::contact> c(new engine::contact(ctct));
  ASSERT_THROW(c->add_service_notification_command(NULL), std::exception);
  ASSERT_THROW(c->add_service_notification_command(""), std::exception);
}

// Given a contact
// When we add a service command to him,
// Then the check method returns false and always set the error value to 2.
TEST_F(ContactConfig, ContactWithNonExistingServiceCommand) {
  configuration::contact ctct("test");
  std::auto_ptr<engine::contact> c(new engine::contact(ctct));
  c->add_service_notification_command("service_command");
  int w = 0, e = 0;
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 2);
  ASSERT_EQ(e, 2);
}

// Given a contact
// When we add a service command to him,
// Then the check method returns false and always set the error value to 2.
TEST_F(ContactConfig, ContactWithServiceCommand) {
  configuration::contact ctct("test");
  std::auto_ptr<engine::contact> c(new engine::contact(ctct));
  engine::commands::command::add_command(new engine::commands::raw(
                                      "service_command",
                                      "echo 1",
                                      NULL));
  c->add_service_notification_command("service_command");
  int w = 0, e = 0;
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 2);
  ASSERT_EQ(e, 1);
}

// Given a contact
// When we add a host command to him,
// Then the check method returns false and always set the error value to 2.
TEST_F(ContactConfig, ContactWithHostCommand) {
  configuration::contact ctct("test");
  std::auto_ptr<engine::contact> c(new engine::contact(ctct));
  engine::commands::command::add_command(new engine::commands::raw(
                                      "host_command",
                                      "echo 2",
                                      NULL));
  c->add_host_notification_command("host_command");
  int w = 0, e = 0;
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 2);
  ASSERT_EQ(e, 1);
}

// Given a contact
// When he is notified on a host recovery
// Then he should be notified on host down or on host unreachable.
TEST_F(ContactConfig, ContactWithRecoveryHostNotification) {
  configuration::contact ctct("test");
  ctct.parse("service_notification_options", "0");
  ctct.parse("host_notification_options", "1");
  ctct.parse("host_notifications_enabled", "1");
  ctct.parse("service_notifications_enabled", "1");
  std::auto_ptr<engine::contact> c(new engine::contact(ctct));
//        "test",
//        "",
//        "",
//        "",
//        std::vector<std::string>(),
//        "",
//        "",
//        0, 0, 0, 0, 0, 0, // services notifications
//        1, 0, 0, 0, 0,    // hosts notifications
//        1, 1, 0, 0, 0, ""));
  int w(0), e(0);
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 2);
  ASSERT_EQ(e, 2);
}

// Given a contact
// When he is notified on a service recovery
// Then he should be notified on service warning or critical.
TEST_F(ContactConfig, ContactWithRecoveryServiceNotification) {
  configuration::contact ctct("test");
  ctct.parse("service_notification_options", "1");
  ctct.parse("host_notification_options", "0");
  ctct.parse("host_notifications_enabled", "1");
  ctct.parse("service_notifications_enabled", "1");
  std::auto_ptr<engine::contact> c(new engine::contact(ctct));
//  std::auto_ptr<engine::contact> c(new engine::contact(
//        "test",
//        "",
//        "",
//        "",
//        std::vector<std::string>(),
//        "",
//        "",
//        1, 0, 0, 0, 0, 0, // services notifications
//        0, 0, 0, 0, 0,    // hosts notifications
//        1, 1, 0, 0, 0, ""));
  int w(0), e(0);
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 2);
  ASSERT_EQ(e, 2);
}
