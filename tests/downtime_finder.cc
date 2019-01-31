/*
** Copyright 2016,2018-2019 Centreon
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

#include <algorithm>
#include <cstring>
#include <gtest/gtest.h>
#include "com/centreon/engine/commands/command.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/downtime_finder.hh"
#include "com/centreon/engine/downtime_manager.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/service.hh"
#include "timeperiod/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

extern configuration::state* config;

class DowntimeFinderFindMatchingAllTest : public ::testing::Test {
 public:
  void SetUp() {
    if (config == NULL)
      config = new configuration::state;
    downtime_manager::load();
    configuration::applier::state::load();

    configuration::applier::host hst_aply;
    configuration::applier::service svc_aply;
    configuration::service csvc1;
    configuration::service csvc2;

    csvc1.parse("service_description", "test_service");
    csvc1.parse("host_name", "test_host");
    csvc2.parse("service_description", "other_service");
    csvc2.parse("host_name", "other_host");

    configuration::host hst1;
    hst1.parse("host_name", "test_host");
    hst_aply.add_object(hst1);
    configuration::host hst2;
    hst2.parse("host_name", "other_host");
    hst_aply.add_object(hst2);

    svc_aply.add_object(csvc1);
    svc_aply.add_object(csvc2);
    csvc1.parse("hosts", "test_host");
    csvc2.parse("hosts", "other_host");

    configuration::applier::command cmd_aply;
    configuration::command cmd("cmd");
    cmd.parse("command_line", "echo 1");
    csvc1.parse("check_command", "cmd");
    csvc2.parse("check_command", "cmd");
    cmd_aply.add_object(cmd);

    svc_aply.resolve_object(csvc1);
    svc_aply.resolve_object(csvc2);

    umap<std::pair<std::string, std::string>, shared_ptr<engine::service> >::iterator sit(
      configuration::applier::state::instance().services().begin());
    engine::service* service1(sit->second.get());
    ++sit;
    engine::service* service2(sit->second.get());
    if (service1->get_description() == "other_service") {
      engine::service* tmp_svc(service1);
      service1 = service2;
      service2 = tmp_svc;
    }

    // Here, service1 is test service and service2 is other service.
    umap<std::string, shared_ptr<engine::host> >::iterator hit(
      configuration::applier::state::instance().hosts().begin());
    engine::host* host1 = hit->second.get();
    if (host1->get_name() == "other_host") {
      ++hit;
      host1 = hit->second.get();
    }
    set_time(123456789);
    new_scheduled_downtime(
      service1,
      123456789,
      134567892,
      1,
      6,
      42,
      "test_author",
      "other_comment");
    new_scheduled_downtime(
      host1,
      134567892,
      234567891,
      1,
      0,
      84,
      "other_author",
      "test_comment");
    new_scheduled_downtime(
      service2,
      123456789,
      345678921,
      0,
      6,
      42,
      "",
      "test_comment");
    new_scheduled_downtime(
      service1,
      234567891,
      345678921,
      0,
      6,
      84,
      "test_author",
      "");
    _dtf = new downtime_finder();
  }

  void TearDown() {
    delete _dtf;
    configuration::applier::state::unload();
    downtime_manager::unload();
    delete config;
    config = NULL;
  }

  unsigned long new_scheduled_downtime(
                  notifications::notifiable* parent,
                  time_t start,
                  time_t end,
                  int fixed,
                  unsigned long triggered_by,
                  unsigned long duration,
                  std::string const& author,
                  std::string const& comment) {
    return (downtime_manager::instance().schedule(
              parent,
              0,
              author,
              comment,
              start,
              end,
              fixed,
              duration,
              triggered_by));
  }

 protected:
  downtime_finder* _dtf;
  downtime_finder::criteria_set criterias;
  downtime_finder::result_set result;
};

// Given a downtime_finder object with the test downtime list
// And no downtime are recorded with a host "anyhost"
// When find_matching_all() is called with criteria ("host", "anyhost")
// Then an empty result_set is returned
TEST_F(DowntimeFinderFindMatchingAllTest, WrongHostnameNotFound) {
  criterias.push_back(downtime_finder::criteria("host", "anyhost"));
  result = _dtf->find_matching_all(criterias);
  ASSERT_TRUE(result.empty());
}

// Given a downtime_finder object with the test downtime list
// When find_matching_all() is called with the criteria ("host", "")
// Then the result_set is empty since all hosts have a name
TEST_F(DowntimeFinderFindMatchingAllTest, EmptyHostnameNotFound) {
  criterias.push_back(downtime_finder::criteria("host", ""));
  result = _dtf->find_matching_all(criterias);
  ASSERT_TRUE(result.empty());
}

// Given a downtime finder object with the test downtime list
// When find_matching_all() is called with criteria ("service", "anyservice")
// Then an empty result_set is returned
TEST_F(DowntimeFinderFindMatchingAllTest, NullServiceNotFound) {
  criterias.push_back(downtime_finder::criteria("service", "anyservice"));
  result = _dtf->find_matching_all(criterias);
  ASSERT_TRUE(result.empty());
}

// Given a downtime finder object with the test downtime list
// When find_matching_all() is called with the criteria ("service", "")
// Then the result_set contains the downtime
TEST_F(DowntimeFinderFindMatchingAllTest, NullServiceFound) {
  criterias.push_back(downtime_finder::criteria("service", ""));
  result = _dtf->find_matching_all(criterias);
  ASSERT_FALSE(result.empty());
  ASSERT_EQ(result[0], 2ul);
}

// Given a downtime_finder object with the test downtime list
// And a downtime of the test list has a null author
// When find_matching_all() is called with the criteria ("author", "anyauthor")
// Then an empty result_set is returned
TEST_F(DowntimeFinderFindMatchingAllTest, EmptyAuthorNotFound) {
  criterias.push_back(downtime_finder::criteria("author", "anyauthor"));
  result = _dtf->find_matching_all(criterias);
  ASSERT_TRUE(result.empty());
}

// Given a downtime_finder object with the test downtime list
// And a downtime of the test list has a null author
// When find_matching_all() is called with the criteria ("author", "")
// Then the result_set contains the downtime
TEST_F(DowntimeFinderFindMatchingAllTest, EmptyAuthorFound) {
  criterias.push_back(downtime_finder::criteria("author", ""));
  result = _dtf->find_matching_all(criterias);
  ASSERT_TRUE(!result.empty());
  ASSERT_EQ(result[0], 3ul);
}

// Given a downtime_finder object with the test downtime list
// And a downtime of the test list has a null comment
// When find_matching_all() is called with the criteria ("comment", "anycomment")
// Then an empty result_set is returned
TEST_F(DowntimeFinderFindMatchingAllTest, EmptyCommentNotFound) {
  criterias.push_back(downtime_finder::criteria("comment", "anycomment"));
  result = _dtf->find_matching_all(criterias);
  ASSERT_TRUE(result.empty());
}

// Given a downtime_finder object with the test downtime list
// And a downtime of the test list has a null comment
// When find_matching_all() is called with the criteria ("comment", "")
// Then the result_set contains the downtime
TEST_F(DowntimeFinderFindMatchingAllTest, NullCommentFound) {
  criterias.push_back(downtime_finder::criteria("comment", ""));
  result = _dtf->find_matching_all(criterias);
  ASSERT_TRUE(!result.empty());
  ASSERT_EQ(result[0], 4ul);
}

// Given a downtime_finder object with the test downtime list
// When find_matching_all() is called with the criteria ("host", "test_host")
// Then all downtimes of host /test_host/ are returned
TEST_F(DowntimeFinderFindMatchingAllTest, MultipleHosts) {
  criterias.push_back(downtime_finder::criteria("host", "test_host"));
  result = _dtf->find_matching_all(criterias);
  std::sort(result.begin(), result.end());
  ASSERT_EQ(result.size(), 3ul);
  ASSERT_EQ(result[0], 1ul);
  ASSERT_EQ(result[1], 2ul);
  ASSERT_EQ(result[2], 4ul);
}

// Given a downtime_finder object with the test downtime list
// When find_matching_all() is called with the criteria ("service", "test_service")
// Then all downtimes of service /test_service/ are returned
TEST_F(DowntimeFinderFindMatchingAllTest, MultipleServices) {
  criterias.push_back(downtime_finder::criteria("service", "test_service"));
  result = _dtf->find_matching_all(criterias);
  std::sort(result.begin(), result.end());
  ASSERT_EQ(result.size(), 2ul);
  ASSERT_EQ(result[0], 1ul);
  ASSERT_EQ(result[1], 4ul);
}

// Given a downtime_finder object with the test downtime list
// When find_matching_all() is called with the criteria ("start", "123456789")
// Then all downtimes with 123456789 as start time are returned
TEST_F(DowntimeFinderFindMatchingAllTest, MultipleStart) {
  criterias.push_back(downtime_finder::criteria("start", "123456789"));
  result = _dtf->find_matching_all(criterias);
  std::sort(result.begin(), result.end());
  ASSERT_EQ(result.size(), 2ul);
  ASSERT_EQ(result[0], 1ul);
  ASSERT_EQ(result[1], 3ul);
}

// Given a downtime_finder object with the test downtime list
// When find_matching_all() is called with the criteria ("end", "134567892")
// Then all downtimes with 134567892 as end time are returned
TEST_F(DowntimeFinderFindMatchingAllTest, MultipleEnd) {
  criterias.push_back(downtime_finder::criteria("end", "345678921"));
  result = _dtf->find_matching_all(criterias);
  std::sort(result.begin(), result.end());
  ASSERT_EQ(result.size(), 2ul);
  ASSERT_EQ(result[0], 3ul);
  ASSERT_EQ(result[1], 4ul);
}

// Given a downtime_finder object with the test downtime list
// When find_matching_all() is called with the criteria ("fixed", "0")
// Then all downtimes that are not fixed are returned
TEST_F(DowntimeFinderFindMatchingAllTest, MultipleFixed) {
  criterias.push_back(downtime_finder::criteria("fixed", "0"));
  result = _dtf->find_matching_all(criterias);
  std::sort(result.begin(), result.end());
  ASSERT_EQ(result.size(), 2ul);
  ASSERT_EQ(result[0], 3ul);
  ASSERT_EQ(result[1], 4ul);
}

// Given a downtime_finder object with the test downtime list
// When find_matching_all() is called with the criteria ("triggered_by", "0")
// Then all downtimes that are not triggered by other downtimes are returned
TEST_F(DowntimeFinderFindMatchingAllTest, MultipleTriggeredBy) {
  criterias.push_back(downtime_finder::criteria("triggered_by", "0"));
  result = _dtf->find_matching_all(criterias);
  ASSERT_EQ(result.size(), 1ul);
  ASSERT_EQ(result[0], 2ul);
}

// Given a downtime_finder object with the test downtime list
// When find_matching_all() is called with the criteria ("duration", "42")
// Then all downtimes with a duration of 42 seconds are returned
TEST_F(DowntimeFinderFindMatchingAllTest, MultipleDuration) {
  criterias.push_back(downtime_finder::criteria("duration", "42"));
  result = _dtf->find_matching_all(criterias);
  std::sort(result.begin(), result.end());
  ASSERT_EQ(result.size(), 2ul);
  ASSERT_EQ(result[0], 1ul);
  ASSERT_EQ(result[1], 3ul);
}

// Given a downtime_finder object with the test downtime list
// When find_matching_all() is called with the criteria ("author", "test_author")
// Then all downtimes from author /test_author/ are returned
TEST_F(DowntimeFinderFindMatchingAllTest, MultipleAuthor) {
  criterias.push_back(downtime_finder::criteria("author", "test_author"));
  result = _dtf->find_matching_all(criterias);
  std::sort(result.begin(), result.end());
  ASSERT_EQ(result.size(), 2ul);
  ASSERT_EQ(result[0], 1ul);
  ASSERT_EQ(result[1], 4ul);
}

// Given a downtime_finder object with the test downtime list
// When find_matching_all() is called with the criteria ("comment", "test_comment")
// Then all downtimes with comment "test_comment" are returned
TEST_F(DowntimeFinderFindMatchingAllTest, MultipleComment) {
  criterias.push_back(downtime_finder::criteria("comment", "test_comment"));
  result = _dtf->find_matching_all(criterias);
  std::sort(result.begin(), result.end());
  ASSERT_EQ(result.size(), 2ul);
  ASSERT_EQ(result[0], 2ul);
  ASSERT_EQ(result[1], 3ul);
}

// Given a downtime_finder object with the test downtime list
// When findMatchinAll() is called with criterias ("author", "test_author"), ("duration", "42") and ("comment", "test_comment")
// Then all downtimes matching the criterias are returned
TEST_F(DowntimeFinderFindMatchingAllTest, MultipleCriterias) {
  criterias.push_back(downtime_finder::criteria("duration", "42"));
  criterias.push_back(downtime_finder::criteria("comment", "test_comment"));
  result = _dtf->find_matching_all(criterias);
  ASSERT_EQ(result.size(), 1ul);
  ASSERT_EQ(result[0], 3ul);
}
