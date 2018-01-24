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
#include "../../timeperiod/utils.hh"
#include "../test_notifier.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::notifications;

extern configuration::state* config;

class FlappingNotification : public ::testing::Test {
 public:
  void SetUp() {
    set_time(20);
    _notifier.reset(new test_notifier());
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();
    contact_map& cm = configuration::applier::state::instance().contacts();
    configuration::contact ctct("test");
    cm["test"] = shared_ptr<engine::contact>(new engine::contact(ctct));
    _notifier->add_contact(configuration::applier::state::instance().contacts_find("test").get());
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

 protected:
  std::auto_ptr<test_notifier>       _notifier;
};

// Given a notifier not in downtime with notifications enabled
// Then The filter on STARTFLAPPING notification is OK to send notification
// When a STARTFLAPPING notification is already sent
// Then the FLAPPINGSTOP notification can be sent when the flapping is finished.
TEST_F(FlappingNotification, StartStopFlapping) {
  time_t last_notification = _notifier->get_last_notification();
  _notifier->set_notifications_enabled(true);
  _notifier->set_flapping(true);
  _notifier->notify(notifier::FLAPPINGSTART, "admin", "Test start flapping");
  time_t current_notification = _notifier->get_last_notification();
  ASSERT_GT(current_notification, last_notification);
  _notifier->set_flapping(false);
  time_t now = last_notification + 10;
  set_time(now);
  _notifier->notify(notifier::FLAPPINGSTOP, "admin", "Test stop flapping");
  last_notification = _notifier->get_last_notification();
  ASSERT_EQ(now, last_notification);
}

// Given a notifier not in downtime with notifications enabled
// Then The filter on STARTFLAPPING notification is OK to send notification
// When a STARTFLAPPING notification is already sent
// Then the FLAPPINGDISABLED notification can be sent when the flapping is finished.
TEST_F(FlappingNotification, StartDisableFlapping) {
  time_t last_notification = _notifier->get_last_notification();
  _notifier->set_notifications_enabled(true);
  _notifier->set_flapping(true);
  _notifier->notify(notifier::FLAPPINGSTART, "admin", "Test start flapping");
  time_t current_notification = _notifier->get_last_notification();
  ASSERT_GT(current_notification, last_notification);
  _notifier->set_flapping(false);
  time_t now = last_notification + 10;
  set_time(now);
  _notifier->notify(notifier::FLAPPINGDISABLED, "admin", "Test disable flapping");
  last_notification = _notifier->get_last_notification();
  ASSERT_EQ(now, last_notification);
}

// Given a notifier not in downtime with notifications enabled
// Then The filter on STARTFLAPPING notification is OK to send notification
// When a STARTFLAPPING notification is already sent
// Then the STARTFLAPPING notification can not be resent.
TEST_F(FlappingNotification, StartStartFlapping) {
  time_t last_notification = _notifier->get_last_notification();
  _notifier->set_notifications_enabled(true);
  _notifier->set_flapping(true);
  _notifier->notify(notifier::FLAPPINGSTART, "admin", "Test start flapping");
  time_t current_notification = _notifier->get_last_notification();
  ASSERT_GT(current_notification, last_notification);
  _notifier->set_flapping(false);
  time_t now = last_notification + 10;
  set_time(now);
  _notifier->notify(notifier::FLAPPINGSTOP, "admin", "Test stop flapping");
  last_notification = _notifier->get_last_notification();
  ASSERT_EQ(now, last_notification);
}
