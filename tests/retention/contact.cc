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
#include "com/centreon/engine/retention/contact.hh"

using namespace com::centreon::engine;

class RetentionContact: public ::testing::Test {
 public:
  void SetUp() {
    ctct.set("contact_name", "contact_name");
    ctct.set("host_notification_period", "host_notification_period");
    ctct.set("host_notifications_enabled", "1");
    ctct.set("last_host_notification", "1300000");
    ctct.set("last_service_notification", "1300000");
    ctct.set("modified_attributes", "1");
    ctct.set("modified_host_attributes", "2");
    ctct.set("modified_service_attributes", "3");
    ctct.set("service_notification_period", "service_notification_period");
    ctct.set("service_notifications_enabled", "1");
  }

 protected:
  retention::contact ctct;
};

// Given a retention contact
// When a copy of it is made
// Then they are equal.
// And each parameter of the copy can be fetched and is equal to the original
// one.
TEST_F(RetentionContact, CopyConstructor) {
  retention::contact copy(ctct);
  ASSERT_TRUE(copy == ctct);
  ASSERT_FALSE(copy != ctct);
  ASSERT_EQ(copy.contact_name(), ctct.contact_name());
  ASSERT_EQ(copy.host_notification_period(), ctct.host_notification_period());
  ASSERT_EQ(copy.host_notifications_enabled(), ctct.host_notifications_enabled());
  ASSERT_EQ(copy.last_host_notification(), ctct.last_host_notification());
  ASSERT_EQ(copy.last_service_notification(), ctct.last_service_notification());
  ASSERT_EQ(copy.modified_attributes(), ctct.modified_attributes());
  ASSERT_EQ(copy.modified_host_attributes(), ctct.modified_host_attributes());
  ASSERT_EQ(copy.modified_service_attributes(), ctct.modified_service_attributes());
  ASSERT_EQ(copy.service_notification_period(), ctct.service_notification_period());
  ASSERT_EQ(copy.service_notifications_enabled(), ctct.service_notifications_enabled());
}
