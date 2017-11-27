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
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class SimpleContactgroup : public ::testing::Test {
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
// When the add_contactgroup function is called with it as argument,
// Then it returns a NULL pointer.
TEST_F(SimpleContactgroup, NewContactgroupWithNoName) {
  ASSERT_TRUE(engine::contactgroup::add_contactgroup("") == NULL);
}

// Given a non empty name
// When the add_contactgroup function is called with it as argument,
// Then a contactgroup is created and the method returns a pointer to it
// And since alias is empty, it is replaced by the name.
TEST_F(SimpleContactgroup, NewContactgroupWithName) {
  engine::contactgroup* c(engine::contactgroup::add_contactgroup("test"));
  ASSERT_EQ(c->get_name(), "test");
  ASSERT_EQ(c->get_alias(), "test");
}

// Given a non empty name
// When the add_contactgroup function is called with it as argument two times,
// Then the contactgroup is well created but just one time.
TEST_F(SimpleContactgroup, NewContactgroupRepeatedTwoTimes) {
  engine::contactgroup* c(engine::contactgroup::add_contactgroup("test"));
  ASSERT_EQ(c->get_name(), "test");
  ASSERT_TRUE(engine::contactgroup::add_contactgroup("test") == NULL);
}

// Given a non empty name and a non empty alias
// When the add_contactgroup function is called with them as argument,
// Then the contactgroup is created with the given name and alias.
TEST_F(SimpleContactgroup, NewContactgroupWithNonEmptyAlias) {
  engine::contactgroup* c(engine::contactgroup::add_contactgroup(
                       "test",
                       "alias_test"));
  ASSERT_EQ(c->get_alias(), "alias_test");
}

// Given an empty contactgroup
// When the check method is executed
// Then no warning, nor error are given.
// And the method returns true (errors count == 0)
TEST_F(SimpleContactgroup, TestContactgroupCheck) {
  engine::contactgroup* c(engine::contactgroup::add_contactgroup("test"));
  int w = 0, e = 0;
  ASSERT_TRUE(c->check(&w, &e));
}

// Given a contactgroup
// When the add_contact method is called with an empty name
// Then an exception is thrown.
TEST_F(SimpleContactgroup, TestContactgroupCheckWithBadContact) {
  engine::contactgroup* c(engine::contactgroup::add_contactgroup("test"));
  ASSERT_THROW(c->add_contact(""), std::exception);
}

// Given a contactgroup
// When the add_contact method is called with a non empty name
// corresponding to a non existing contact
// Then an error is given
// And the method returns false.
TEST_F(SimpleContactgroup, TestContactgroupCheckWithOneInexistentContact) {
  engine::contactgroup* c(engine::contactgroup::add_contactgroup("test"));
  c->add_contact("centreon");
  int w(0), e(0);
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 0);
  ASSERT_EQ(e, 1);
}

// Given a contactgroup
// When the add_contact method is called with a non empty name
// corresponding to an existing contact
// Then an error is given
// And the method returns false.
// Then the contact group contains only the add user.
// When the clear_members() is executed, the contact group contains no more
// member.
TEST_F(SimpleContactgroup, TestContactgroupCheckWithOneContact) {
  engine::contactgroup* c(engine::contactgroup::add_contactgroup("test"));
  c->add_contact("centreon");
  engine::contact* user(engine::contact::add_contact("centreon"));
  int w(0), e(0);
  ASSERT_TRUE(c->check(&w, &e));
  ASSERT_EQ(w, 0);
  ASSERT_EQ(e, 0);

  ASSERT_FALSE(c->contains_member("nagios"));
  ASSERT_TRUE(c->contains_member("centreon"));

  c->clear_members();
  ASSERT_TRUE(c->get_members().empty());
}

#if 0
// Given a contact
// When adding a service notification period by its name without a corresponding
// notification period definition
// Then the check method returns false with 1 warning and 3 errors.
TEST_F(SimpleContactgroup, AddServiceNotificationTimeperiod) {
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
TEST_F(SimpleContactgroup, AddHostNotificationTimeperiod) {
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
TEST_F(SimpleContactgroup, AddHostNotificationCommand) {
  engine::contact* c(engine::contact::add_contact("test"));
  ASSERT_THROW(c->add_host_notification_command(NULL), std::exception);
  ASSERT_THROW(c->add_host_notification_command(""), std::exception);
}

// Given a contact
// When adding a service notification command with an null/empty name
// Then an exception is thrown.
TEST_F(SimpleContactgroup, AddServiceNotificationCommand) {
  engine::contact* c(engine::contact::add_contact("test"));
  ASSERT_THROW(c->add_service_notification_command(NULL), std::exception);
  ASSERT_THROW(c->add_service_notification_command(""), std::exception);
}

// Given a contact
// When we add a service command to him, but the service command contains
// no command.
// Then the check method returns false and always set the error value to 2.
TEST_F(SimpleContactgroup, ContactWithServiceCommand) {
  engine::contact* c(engine::contact::add_contact("test"));
  c->add_service_notification_command("service_command");
  int w = 0, e = 0;
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 2);
  ASSERT_EQ(e, 2);
}

// Given a contact
// When we add a host command to him, but the host command contains
// no command.
// Then the check method returns false and always set the error value to 2.
TEST_F(SimpleContactgroup, ContactWithHostCommand) {
  engine::contact* c(engine::contact::add_contact("test"));
  c->add_host_notification_command("host_command");
  int w = 0, e = 0;
  ASSERT_FALSE(c->check(&w, &e));
  ASSERT_EQ(w, 2);
  ASSERT_EQ(e, 2);
}

// Given a contact
// When he is notified on a host recovery
// Then he should be notified on host down or on host unreachable.
TEST_F(SimpleContactgroup, ContactWithRecoveryHostNotification) {
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
TEST_F(SimpleContactgroup, ContactWithRecoveryServiceNotification) {
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
#endif
