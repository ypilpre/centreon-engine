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

#include <gtest/gtest.h>
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/retention/contact.hh"
#include "com/centreon/engine/retention/applier/contact.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

extern configuration::state* config;

class RetentionContact: public ::testing::Test {
 public:
  void SetUp() {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load(); // Needed to create a contact

    ctct = shared_ptr<retention::contact>(new retention::contact);
    ctct->set("contact_name", "contact_name");
    ctct->set("host_notification_period", "host_notification_period");
    ctct->set("host_notifications_enabled", "1");
    ctct->set("last_host_notification", "1300000");
    ctct->set("last_service_notification", "1400000");
    ctct->set("modified_attributes", "1");
    ctct->set("modified_host_attributes", "2");
    ctct->set("modified_service_attributes", "3");
    ctct->set("service_notification_period", "service_notification_period");
    ctct->set("service_notifications_enabled", "1");
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

 protected:
  shared_ptr<retention::contact> ctct;
};

// Given a retention contact
// When a copy of it is made
// Then they are equal.
// And each parameter of the copy can be fetched and is equal to the original
// one.
TEST_F(RetentionContact, CopyConstructor) {
  retention::contact copy(*ctct.get());
  ASSERT_TRUE(copy == *ctct.get());
  ASSERT_FALSE(copy != *ctct.get());
  ASSERT_EQ(copy.contact_name(), ctct->contact_name());
  ASSERT_EQ(copy.host_notification_period(), ctct->host_notification_period());
  ASSERT_EQ(copy.host_notifications_enabled(),
            ctct->host_notifications_enabled());
  ASSERT_EQ(copy.last_host_notification(), ctct->last_host_notification());
  ASSERT_EQ(copy.last_service_notification(), ctct->last_service_notification());
  ASSERT_EQ(copy.modified_attributes(), ctct->modified_attributes());
  ASSERT_EQ(copy.modified_host_attributes(), ctct->modified_host_attributes());
  ASSERT_EQ(copy.modified_service_attributes(),
            ctct->modified_service_attributes());
  ASSERT_EQ(copy.service_notification_period(),
            ctct->service_notification_period());
  ASSERT_EQ(copy.service_notifications_enabled(),
            ctct->service_notifications_enabled());
}

// Given a retention contact and a contact
// When the retention is applied
// Then the contact is filled with its properties.
TEST_F(RetentionContact, ApplyContact) {
  configuration::applier::contact aply;
  configuration::contact c;
  ASSERT_TRUE(c.parse("contact_name", "contact_name"));
  aply.add_object(c);
  ASSERT_EQ(configuration::applier::state::instance().contacts().size(), 1);
  std::list<shared_ptr<retention::contact> > lst;
  lst.push_back(ctct);

  retention::applier::contact aply_ctct;
  aply_ctct.apply(*config, lst);
  shared_ptr<engine::contact> ct(configuration::applier::state::instance().contacts().begin()->second);
  ASSERT_EQ(ct->get_name(), "contact_name");
  //ASSERT_TRUE(ct->get_host_notification_period() == NULL);
  ASSERT_TRUE(ct->get_host_notifications_enabled());
  ASSERT_EQ(ct->get_last_host_notification(), static_cast<time_t>(1300000));
  ASSERT_EQ(ct->get_last_service_notification(), static_cast<time_t>(1400000));
  ASSERT_EQ(ct->get_modified_attributes(), 1);
  ASSERT_EQ(ct->get_modified_host_attributes(), 2);
  ASSERT_EQ(ct->get_modified_service_attributes(), 3);
  //ASSERT_EQ(ct->get_service_notification_period()->name, "service_notification_period");
  ASSERT_TRUE(ct->get_service_notifications_enabled());
}
