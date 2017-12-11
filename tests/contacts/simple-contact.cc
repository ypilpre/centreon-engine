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

class SimpleContact : public ::testing::Test {
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
TEST_F(SimpleContact, NewContactWithNoName) {
  ASSERT_TRUE(engine::contact::add_contact("") == NULL);
}

// Given a non empty name
// When the add_contact function is called with it as argument,
// Then a contact is created and the method returns a pointer to it
// And since alias is empty, it is replaced by the name.
TEST_F(SimpleContact, NewContactWithName) {
  engine::contact* c(engine::contact::add_contact("test"));
  ASSERT_EQ(c->get_name(), "test");
  ASSERT_EQ(c->get_alias(), "test");
}

// Given a non empty name
// When the add_contact function is called with it as argument two times,
// Then the contact is well created but just one time.
TEST_F(SimpleContact, NewContactRepeatedTwoTimes) {
  engine::contact* c(engine::contact::add_contact("test"));
  ASSERT_EQ(c->get_name(), "test");
  ASSERT_TRUE(engine::contact::add_contact("test") == NULL);
}

// Given a non empty name and an empty alias
// When the add_contact function is called with them as argument,
// Then the contact is created with an alias replaced by the name.
TEST_F(SimpleContact, NewContactWithNonEmptyAlias) {
  engine::contact* c(engine::contact::add_contact(
                       "test",
                       "alias_test",
                       "email@mail.com",
                       "pager"));
  ASSERT_EQ(c->get_alias(), "alias_test");
  ASSERT_EQ(c->get_email(), "email@mail.com");
  ASSERT_EQ(c->get_pager(), "pager");
}

// Given a contact
// When the check method is executed
// Then 2 warnings and 2 errors are returned:
//  * error 1 => no service notification command
//  * error 2 => no host notification command
//  * warning 1 => no service notification period
//  * warning 2 => no host notification period
// And the method returns false (errors number != 0)
TEST_F(SimpleContact, TestContactCheck) {
  engine::contact* c(engine::contact::add_contact("test"));
  int w = 0, e = 0;
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 2);
  ASSERT_EQ(e, 2);
}

// Given a contact
// When adding a service notification period by its name without a corresponding
// notification period definition
// Then the check method returns false with 1 warning and 3 errors.
TEST_F(SimpleContact, AddServiceNotificationTimeperiod) {
  engine::contact* c(engine::contact::add_contact("test"));
  c->set_service_notification_period_name("notif_period");
  int w = 0, e = 0;
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 1);
  ASSERT_EQ(e, 3);
}

// Given a contact
// When adding a host notification period by its name without a corresponding
// notification period definition
// Then the check method returns false with 1 warning and 3 errors.
TEST_F(SimpleContact, AddHostNotificationTimeperiod) {
  engine::contact* c(engine::contact::add_contact("test"));
  c->set_host_notification_period_name("notif_period");
  int w = 0, e = 0;
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 1);
  ASSERT_EQ(e, 3);
}

// Given a contact
// When adding a host notification command with an null/empty name
// Then an exception is thrown.
TEST_F(SimpleContact, AddHostNotificationCommand) {
  engine::contact* c(engine::contact::add_contact("test"));
  ASSERT_THROW(c->add_host_notification_command(NULL), std::exception);
  ASSERT_THROW(c->add_host_notification_command(""), std::exception);
}

// Given a contact
// When adding a service notification command with an null/empty name
// Then an exception is thrown.
TEST_F(SimpleContact, AddServiceNotificationCommand) {
  engine::contact* c(engine::contact::add_contact("test"));
  ASSERT_THROW(c->add_service_notification_command(NULL), std::exception);
  ASSERT_THROW(c->add_service_notification_command(""), std::exception);
}

// Given a contact
// When we add a service command to him,
// Then the check method returns false and always set the error value to 2.
TEST_F(SimpleContact, ContactWithNonExistingServiceCommand) {
  engine::contact* c(engine::contact::add_contact("test"));
  c->add_service_notification_command("service_command");
  int w = 0, e = 0;
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 2);
  ASSERT_EQ(e, 2);
}

// Given a contact
// When we add a service command to him,
// Then the check method returns false and always set the error value to 2.
TEST_F(SimpleContact, ContactWithServiceCommand) {
  engine::contact* c(engine::contact::add_contact("test"));
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
TEST_F(SimpleContact, ContactWithHostCommand) {
  engine::contact* c(engine::contact::add_contact("test"));
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
TEST_F(SimpleContact, ContactWithRecoveryHostNotification) {
  engine::contact* c(engine::contact::add_contact(
        "test",
        "",
        "",
        "",
        std::vector<std::string>(),
        "",
        "",
        0, 0, 0, 0, 0, 0, // services notifications
        1, 0, 0, 0, 0,    // hosts notifications
        1, 1, 0, 0, 0, ""));
  int w(0), e(0);
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 3);
  ASSERT_EQ(e, 2);
}

// Given a contact
// When he is notified on a service recovery
// Then he should be notified on service warning or critical.
TEST_F(SimpleContact, ContactWithRecoveryServiceNotification) {
  engine::contact* c(engine::contact::add_contact(
        "test",
        "",
        "",
        "",
        std::vector<std::string>(),
        "",
        "",
        1, 0, 0, 0, 0, 0, // services notifications
        0, 0, 0, 0, 0,    // hosts notifications
        1, 1, 0, 0, 0, ""));
  int w(0), e(0);
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 3);
  ASSERT_EQ(e, 2);
}
