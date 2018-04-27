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
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/downtime_manager.hh"
#include "../timeperiod/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

class DowntimeManager : public ::testing::Test {
 public:
  void SetUp() {
    // Initialization.
    if (config == NULL)
      config = new configuration::state;
    comment_list.clear();
    downtime_manager::load();
    configuration::applier::state::load();

    // Create command.
    configuration::command cmd("cmd");
    cmd.parse("command_line", "echo 1");
    configuration::applier::command cmd_aply;
    cmd_aply.add_object(cmd);
    cmd_aply.expand_objects(*config);
    cmd_aply.resolve_object(cmd);

    // Create host.
    configuration::host hst;
    hst.parse("host_name", "test_host");
    hst.parse("check_command", "cmd");
    configuration::applier::host hst_aply;
    hst_aply.add_object(hst);
    hst_aply.expand_objects(*config);
    hst_aply.resolve_object(hst);

    // Create service.
    configuration::service svc;
    svc.parse("hosts", "test_host");
    svc.parse("service_description", "test description");
    svc.parse("check_command", "cmd");
    configuration::applier::service svc_aply;
    svc_aply.add_object(svc);
    svc_aply.expand_objects(*config);
    svc_aply.resolve_object(svc);

    // Set base downtime parameters.
    time_t now(time(NULL));
    _entry_time = now + 20;
    _author = "Admin (author)";
    _comment = "Test downtime comment";
    _start_time = now + 40;
    _end_time = now + 60;
    _fixed = true;
    _duration = 20;
    _triggered_by = 0;
    _propagate = downtime_manager::DOWNTIME_PROPAGATE_NONE;
    _id = 0;
  }

  void TearDown() {
    configuration::applier::state::unload();
    downtime_manager::unload();
    comment_list.clear();
  }

  shared_ptr< ::host> get_test_host() {
    return (configuration::applier::state::instance().hosts_find(
              "test_host"));
  }

  shared_ptr< ::service> get_test_service() {
    return (configuration::applier::state::instance().services_find(
              std::make_pair("test_host", "test description")));
  }

  unsigned long schedule_test_downtime(notifications::notifier* target) {
    return (downtime_manager::instance().schedule(
              target,
              _entry_time,
              _author,
              _comment,
              _start_time,
              _end_time,
              _fixed,
              _duration,
              _triggered_by,
              _propagate,
              _id));
  }

 protected:
  time_t                        _entry_time;
  std::string                   _author;
  std::string                   _comment;
  time_t                        _start_time;
  time_t                        _end_time;
  bool                          _fixed;
  unsigned long                 _duration;
  unsigned long                 _triggered_by;
  downtime_manager::propagation _propagate;
  unsigned long                 _id;

};

// Given a downtime_manager
// And a target
// When schedule() is called on the target with a downtime already past
// Then schedule() returns 0
// And no downtime is scheduled
TEST_F(DowntimeManager, SchedulePastDowntime) {
  // Given
  shared_ptr<service> target(get_test_service());

  // When
  _start_time = 1234567890;
  _end_time = 1324567890;
  set_time(1423567890);
  ASSERT_EQ(schedule_test_downtime(target.get()), 0ul);

  // Then
  ASSERT_TRUE(downtime_manager::instance().get_downtimes().empty());
}

// Given a downtime_manager
// And a target
// When schedule() is called on the target with the end time less than the start time
// Then schedule() returns 0
// And no downtime is scheduled
TEST_F(DowntimeManager, ScheduleInconsistentDowntime) {
  // Given
  shared_ptr<service> target(get_test_service());

  // When
  {
    time_t tmp(_start_time);
    _start_time = _end_time;
    _end_time = tmp;
  }
  ASSERT_EQ(schedule_test_downtime(target.get()), 0ul);

  // Then
  ASSERT_TRUE(downtime_manager::instance().get_downtimes().empty());
}

// Given a downtime_manager
// And a host
// When schedule() is called on the host
// Then a new downtime is created in the downtime list
// And it has its properties properly filled
TEST_F(DowntimeManager, ScheduleHostDowntime) {
  // Given
  shared_ptr<host> target(get_test_host());

  // When
  ASSERT_EQ(schedule_test_downtime(target.get()), 1ul);

  // Then
  ASSERT_EQ(downtime_manager::instance().get_downtimes().size(), 1u);
  downtime const& dt(downtime_manager::instance().get_downtimes().begin()->second);
  ASSERT_EQ(dt.get_id(), 1ul);
  ASSERT_EQ(dt.get_parent(), target.get());
  ASSERT_EQ(dt.get_entry_time(), _entry_time);
  ASSERT_EQ(dt.get_author(), _author);
  ASSERT_EQ(dt.get_comment(), _comment);
  ASSERT_EQ(dt.get_start_time(), _start_time);
  ASSERT_EQ(dt.get_end_time(), _end_time);
  ASSERT_EQ(dt.get_fixed(), _fixed);
  ASSERT_EQ(dt.get_duration(), _duration);
  ASSERT_EQ(dt.get_triggered_by(), _triggered_by);
  ASSERT_FALSE(dt.get_in_effect());
}

// Given a downtime_manager
// And a service
// When schedule() is called on the service
// Then a new downtime is created in the downtime list
// And it has its properties properly filled
TEST_F(DowntimeManager, ScheduleServiceDowntime) {
  // Given
  shared_ptr<service> target(get_test_service());

  // When
  ASSERT_EQ(schedule_test_downtime(target.get()), 1ul);

  // Then
  ASSERT_EQ(downtime_manager::instance().get_downtimes().size(), 1u);
  downtime const& dt(downtime_manager::instance().get_downtimes().begin()->second);
  ASSERT_EQ(dt.get_id(), 1ul);
  ASSERT_EQ(dt.get_parent(), target.get());
  ASSERT_EQ(dt.get_entry_time(), _entry_time);
  ASSERT_EQ(dt.get_author(), _author);
  ASSERT_EQ(dt.get_comment(), _comment);
  ASSERT_EQ(dt.get_start_time(), _start_time);
  ASSERT_EQ(dt.get_end_time(), _end_time);
  ASSERT_EQ(dt.get_fixed(), _fixed);
  ASSERT_EQ(dt.get_duration(), _duration);
  ASSERT_EQ(dt.get_triggered_by(), _triggered_by);
  ASSERT_FALSE(dt.get_in_effect());
}

// Given a downtime_manager
// And a host
// And a downtime created on the host
// When unschedule() is called on the downtime
// Then it is removed from the downtime list
TEST_F(DowntimeManager, UnscheduleNotStartedHostDowntime) {
  // Given
  shared_ptr<host> target(get_test_host());
  schedule_test_downtime(target.get());

  // When
  downtime_manager::instance().unschedule(1);

  // Then
  ASSERT_TRUE(downtime_manager::instance().get_downtimes().empty());
}

// Given a downtime_manager
// And a service
// And a downtime created on the service
// When unschedule() is called on the downtime
// Then it is removed from the downtime list
TEST_F(DowntimeManager, UnscheduleNotStartedServiceDowntime) {
  // Given
  shared_ptr<service> target(get_test_service());
  schedule_test_downtime(target.get());

  // When
  downtime_manager::instance().unschedule(1);

  // Then
  ASSERT_TRUE(downtime_manager::instance().get_downtimes().empty());
}

// Given a downtime_manager
// And a host
// And a downtime created on the host
// When start() is called on the downtime
// Then the downtime is marked as started
// And the host is marked in downtime
// And a comment is created
TEST_F(DowntimeManager, StartHostDowntime) {
  // Given
  shared_ptr<host> target(get_test_host());
  _start_time = time(NULL);
  _end_time = _start_time + 1000;
  _duration = _end_time - _start_time;
  schedule_test_downtime(target.get());

  // When
  downtime_manager::instance().start(1);

  // Then
  ASSERT_EQ(downtime_manager::instance().get_downtimes().size(), 1u);
  downtime const& dt(downtime_manager::instance().get_downtimes().begin()->second);
  ASSERT_TRUE(dt.get_in_effect());
  ASSERT_EQ(target->get_scheduled_downtime_depth(), 1);
  ASSERT_EQ(comment_list.size(), 1u);
  std::string beginning("This host has been scheduled for fixed downtime from ");
  ASSERT_EQ(
    comment_list.begin()->second->get_comment_data().substr(0, beginning.size()),
    beginning);
}

// Given a downtime_manager
// And a service
// And a downtime created on the service
// When start() is called on the downtime
// Then the downtime is marked as started
// And the service is marked in downtime
// And a comment is created
TEST_F(DowntimeManager, StartServiceDowntime) {
  // Given
  shared_ptr<service> target(get_test_service());
  _start_time = time(NULL);
  _end_time = _start_time + 1000;
  _duration = _end_time - _start_time;
  schedule_test_downtime(target.get());

  // When
  downtime_manager::instance().start(1);

  // Then
  ASSERT_EQ(downtime_manager::instance().get_downtimes().size(), 1u);
  downtime const& dt(downtime_manager::instance().get_downtimes().begin()->second);
  ASSERT_TRUE(dt.get_in_effect());
  ASSERT_EQ(target->get_scheduled_downtime_depth(), 1);
  ASSERT_EQ(comment_list.size(), 1u);
  std::string beginning("This service has been scheduled for fixed downtime from ");
  ASSERT_EQ(
    comment_list.begin()->second->get_comment_data().substr(0, beginning.size()),
    beginning);
}

// Given a downtime_manager
// And a downtime started on a host
// When stop() is called on the downtime
// Then the downtime is deleted
// And the host is not in downtime anymore
TEST_F(DowntimeManager, StopLastHostDowntime) {
  // Given
  shared_ptr<host> target(get_test_host());
  _start_time = time(NULL);
  _end_time = _start_time + 1000;
  _duration = _end_time - _start_time;
  schedule_test_downtime(target.get());
  downtime_manager::instance().start(1);

  // When
  downtime_manager::instance().stop(1);

  // Then
  ASSERT_TRUE(downtime_manager::instance().get_downtimes().empty());
  ASSERT_EQ(target->get_scheduled_downtime_depth(), 0);
}

// Given a downtime_manager
// And a downtime started on a service
// When stop() is called on the downtime
// Then the downtime is deleted
// And the service is not in downtime anymore
TEST_F(DowntimeManager, StopLastServiceDowntime) {
  // Given
  shared_ptr<service> target(get_test_service());
  _start_time = time(NULL);
  _end_time = _start_time + 1000;
  _duration = _end_time - _start_time;
  schedule_test_downtime(target.get());
  downtime_manager::instance().start(1);

  // When
  downtime_manager::instance().stop(1);

  // Then
  ASSERT_TRUE(downtime_manager::instance().get_downtimes().empty());
  ASSERT_EQ(target->get_scheduled_downtime_depth(), 0);
}

// Given a downtime_manager
// And multiple downtimes started on a host
// When stop() is called on the downtime
// Then the downtime is deleted
// And the host is still in downtime
TEST_F(DowntimeManager, StopOneOfMultipleHostDowntime) {
  // Given
  shared_ptr<host> target(get_test_host());
  _start_time = time(NULL);
  _end_time = _start_time + 1000;
  _duration = _end_time - _start_time;
  schedule_test_downtime(target.get());
  schedule_test_downtime(target.get());
  downtime_manager::instance().start(1);
  downtime_manager::instance().start(2);

  // When
  downtime_manager::instance().stop(2);

  // Then
  ASSERT_EQ(downtime_manager::instance().get_downtimes().size(), 1u);
  ASSERT_EQ(target->get_scheduled_downtime_depth(), 1);
}

// Given a downtime_manager
// And multiple downtimes started on a service
// When stop() is called on the downtime
// Then the downtime is deleted
// And the service is still in downtime
TEST_F(DowntimeManager, StopOneOfMultipleServiceDowntime) {
  // Given
  shared_ptr<service> target(get_test_service());
  _start_time = time(NULL);
  _end_time = _start_time + 1000;
  _duration = _end_time - _start_time;
  schedule_test_downtime(target.get());
  schedule_test_downtime(target.get());
  downtime_manager::instance().start(1);
  downtime_manager::instance().start(2);

  // When
  downtime_manager::instance().stop(2);

  // Then
  ASSERT_EQ(downtime_manager::instance().get_downtimes().size(), 1u);
  ASSERT_EQ(target->get_scheduled_downtime_depth(), 1);
}
