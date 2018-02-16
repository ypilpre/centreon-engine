/*
** Copyright 2016 Centreon
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

#include <cstring>
#include <gtest/gtest.h>
#include "com/centreon/engine/commands/command.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/downtime_finder.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/service.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

extern configuration::state* config;

class DowntimeFinderFindMatchingAllTest : public ::testing::Test {
 public:
  void SetUp() {
    downtime* dtp;

    if (config == NULL)
      config = new configuration::state;
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

    configuration::applier::command cmd_aply;
    configuration::command cmd("cmd");
    cmd.parse("command_line", "echo 1");
    csvc1.parse("check_command", "cmd");
    csvc2.parse("check_command", "cmd");
    cmd_aply.add_object(cmd);

    svc_aply.resolve_object(csvc1);

    umap<std::pair<std::string, std::string>, shared_ptr<engine::service> >::iterator sit(
      configuration::applier::state::instance().services().begin());
    engine::service* service1(sit->second.get());
    ++sit;
    engine::service* service2(sit->second.get());

    umap<std::string, shared_ptr<engine::host> >::iterator hit(
      configuration::applier::state::instance().hosts().begin());
    engine::host* host1 = hit->second.get();
    ++hit;
    engine::host* host2 = hit->second.get();

    std::cout << "step 1" << std::endl;
    dtp = new_scheduled_downtime(
             1,
             service1,
             123456789,
             134567892,
             1,
             6,
             42,
             "test_author",
             "other_comment");
    scheduled_downtime_list[dtp->get_id()] = dtp;
    std::cout << "step 2" << std::endl;
    dtp = new_scheduled_downtime(
             2,
             host1,
             234567891,
             134567892,
             1,
             0,
             84,
             "other_author",
             "test_comment");
    scheduled_downtime_list[dtp->get_id()] = dtp;
    std::cout << "step 3" << std::endl;
    dtp = new_scheduled_downtime(
             3,
             service2,
             123456789,
             345678921,
             0,
             6,
             42,
             "",
             "test_comment");
    scheduled_downtime_list[dtp->get_id()] = dtp;
    std::cout << "step 4" << std::endl;
    dtp = new_scheduled_downtime(
             4,
             service1,
             234567891,
             345678921,
             0,
             6,
             84,
             "test_author",
             "");
    scheduled_downtime_list[dtp->get_id()] = dtp;
//    dtp = new_scheduled_downtime(
//             5,
//             "other_host",
//             "test_service",
//             123456789,
//             134567892,
//             1,
//             0,
//             42,
//             "test_author",
//             "test_comment");
//    scheduled_downtime_list[dtp->get_id()] = dtp;
    _dtf = new downtime_finder();
    std::cout << "step 5" << std::endl;
  }

  void TearDown() {
    delete _dtf;
//    for (std::map<unsigned long, downtime* >::iterator
//           it(scheduled_downtime_list.begin()),
//           end(scheduled_downtime_list.end());
//         it != end;
//         ++it) {
//      delete it->second;
//    }
    scheduled_downtime_list.clear();
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

  downtime* new_scheduled_downtime(
                        unsigned long downtime_id,
                        monitorable* parent,
                        time_t start,
                        time_t end,
                        int fixed,
                        unsigned long triggered_by,
                        unsigned long duration,
                        std::string const& author,
                        std::string const& comment) {
    return new downtime(
                 downtime::ANY_DOWNTIME,
                 parent,
                 0,
                 author,
                 comment,
                 start,
                 end,
                 fixed,
                 triggered_by,
                 duration);
  }

 protected:
  downtime_finder* _dtf;
  downtime* dtl;
  downtime_finder::criteria_set criterias;
  downtime_finder::result_set result;
  downtime_finder::result_set expected;
};

// Given a downtime_finder object with the test downtime list
// And a downtime of the test list has a null host_name
// When find_matching_all() is called with criteria ("host", "anyhost")
// Then an empty result_set is returned
TEST_F(DowntimeFinderFindMatchingAllTest, NullHostNotFound) {
    std::cout << "test 1" << std::endl;
  criterias.push_back(downtime_finder::criteria("host", "anyhost"));
    std::cout << "test 2" << std::endl;
  result = _dtf->find_matching_all(criterias);
    std::cout << "test 3" << std::endl;
  ASSERT_TRUE(result.empty());
    std::cout << "test 4" << std::endl;
}

// Given a downtime_finder object with the test downtime list
// And a downtime of the test list has a null host_name
// When find_matching_all() is called with the criteria ("host", "")
// Then the result_set contains the downtime
TEST_F(DowntimeFinderFindMatchingAllTest, NullHostFound) {
    std::cout << "test 5" << std::endl;
  criterias.push_back(downtime_finder::criteria("host", ""));
    std::cout << "test 6" << std::endl;
//  result = _dtf->find_matching_all(criterias);
//  expected.push_back(1);
//  ASSERT_EQ(result, expected);
}

// Given a downtime finder object with the test downtime list
// And a downtime of the test list has a null service_description
// When find_matching_all() is called with criteria ("service", "anyservice")
// Then an empty result_set is returned
//FIXME DBR
//TEST_F(DowntimeFinderFindMatchingAllTest, NullServiceNotFound) {
//  criterias.push_back(downtime_finder::criteria("service", "anyservice"));
//  result = _dtf->find_matching_all(criterias);
//  ASSERT_TRUE(result.empty());
//}

// Given a downtime finder object with the test downtime list
// And a downtime the test list has a null service_description
// When find_matching_all() is called with the criteria ("service", "")
// Then the result_set contains the downtime
// FIXME DBR
//TEST_F(DowntimeFinderFindMatchingAllTest, NullServiceFound) {
//  criterias.push_back(downtime_finder::criteria("service", ""));
//  result = _dtf->find_matching_all(criterias);
//  expected.push_back(2);
//  ASSERT_EQ(result, expected);
//}

// Given a downtime_finder object with the test downtime list
// And a downtime of the test list has a null author
// When find_matching_all() is called with the criteria ("author", "anyauthor")
// Then an empty result_set is returned
//TEST_F(DowntimeFinderFindMatchingAllTest, EmptyAuthorNotFound) {
//  criterias.push_back(downtime_finder::criteria("author", "anyauthor"));
//  result = _dtf->find_matching_all(criterias);
//  ASSERT_TRUE(result.empty());
//}

// Given a downtime_finder object with the test downtime list
// And a downtime of the test list has a null author
// When find_matching_all() is called with the criteria ("author", "")
// Then the result_set contains the downtime
// FIXME DBR
//TEST_F(DowntimeFinderFindMatchingAllTest, NullAuthorFound) {
//  criterias.push_back(downtime_finder::criteria("author", ""));
//  result = _dtf->find_matching_all(criterias);
//  expected.push_back(3);
//  ASSERT_EQ(result, expected);
//}

// Given a downtime_finder object with the test downtime list
// And a downtime of the test list has a null comment
// When find_matching_all() is called with the criteria ("comment", "anycomment")
// Then an empty result_set is returned
//TEST_F(DowntimeFinderFindMatchingAllTest, NullCommentNotFound) {
//  criterias.push_back(downtime_finder::criteria("comment", "anycomment"));
//  result = __dtf->find_matching_all(criterias);
//  ASSERT_TRUE(result.empty());
//}

// Given a downtime_finder object with the test downtime list
// And a downtime of the test list has a null comment
// When find_matching_all() is called with the criteria ("comment", "")
// Then the result_set contains the downtime
//FIXME DBR
//TEST_F(DowntimeFinderFindMatchingAllTest, NullCommentFound) {
//  criterias.push_back(downtime_finder::criteria("comment", ""));
//  result = _dtf->find_matching_all(criterias);
//  expected.push_back(4);
//  ASSERT_EQ(result, expected);
//}

// Given a downtime_finder object with the test downtime list
// When find_matching_all() is called with the criteria ("host", "test_host")
// Then all downtimes of host /test_host/ are returned
// FIXME DBR
//TEST_F(DowntimeFinderFindMatchingAllTest, MultipleHosts) {
//  criterias.push_back(downtime_finder::criteria("host", "test_host"));
//  result = __dtf->find_matching_all(criterias);
//  expected.push_back(2);
//  expected.push_back(4);
//  ASSERT_EQ(result, expected);
//}

// Given a downtime_finder object with the test downtime list
// When find_matching_all() is called with the criteria ("service", "test_service")
// Then all downtimes of service /test_service/ are returned
// FIXME DBR
//TEST_F(DowntimeFinderFindMatchingAllTest, MultipleServices) {
//  criterias.push_back(downtime_finder::criteria("service", "test_service"));
//  result = _dtf->find_matching_all(criterias);
//  expected.push_back(1);
//  expected.push_back(4);
//  expected.push_back(5);
//  ASSERT_EQ(result, expected);
//}

// Given a downtime_finder object with the test downtime list
// When find_matching_all() is called with the criteria ("start", "123456789")
// Then all downtimes with 123456789 as start time are returned
// FIXME DBR
//TEST_F(DowntimeFinderFindMatchingAllTest, MultipleStart) {
//  criterias.push_back(downtime_finder::criteria("start", "123456789"));
//  result = _dtf->find_matching_all(criterias);
//  expected.push_back(1);
//  expected.push_back(3);
//  expected.push_back(5);
//  ASSERT_EQ(result, expected);
//}

// Given a downtime_finder object with the test downtime list
// When find_matching_all() is called with the criteria ("end", "134567892")
// Then all downtimes with 134567892 as end time are returned
//FIXME DBR
//TEST_F(DowntimeFinderFindMatchingAllTest, MultipleEnd) {
//  criterias.push_back(downtime_finder::criteria("end", "134567892"));
//  result = _dtf->find_matching_all(criterias);
//  expected.push_back(1);
//  expected.push_back(2);
//  expected.push_back(5);
//  ASSERT_EQ(result, expected);
//}

// Given a downtime_finder object with the test downtime list
// When find_matching_all() is called with the criteria ("fixed", "0")
// Then all downtimes that are not fixed are returned
// FIXME DBR
//TEST_F(DowntimeFinderFindMatchingAllTest, MultipleFixed) {
//  criterias.push_back(downtime_finder::criteria("fixed", "0"));
//  result = _dtf->find_matching_all(criterias);
//  expected.push_back(3);
//  expected.push_back(4);
//  ASSERT_EQ(result, expected);
//}

// FIXME DBR
//// Given a downtime_finder object with the test downtime list
//// When find_matching_all() is called with the criteria ("triggered_by", "0")
//// Then all downtimes that are not triggered by other downtimes are returned
//TEST_F(DowntimeFinderFindMatchingAllTest, MultipleTriggeredBy) {
//  criterias.push_back(downtime_finder::criteria("triggered_by", "0"));
//  result = _dtf->find_matching_all(criterias);
//  expected.push_back(2);
//  expected.push_back(5);
//  ASSERT_EQ(result, expected);
//}
//
//// Given a downtime_finder object with the test downtime list
//// When find_matching_all() is called with the criteria ("duration", "42")
//// Then all downtimes with a duration of 42 seconds are returned
//TEST_F(DowntimeFinderFindMatchingAllTest, MultipleDuration) {
//  criterias.push_back(downtime_finder::criteria("duration", "42"));
//  result = _dtf->find_matching_all(criterias);
//  expected.push_back(1);
//  expected.push_back(3);
//  expected.push_back(5);
//  ASSERT_EQ(result, expected);
//}
//
//// Given a downtime_finder object with the test downtime list
//// When find_matching_all() is called with the criteria ("author", "test_author")
//// Then all downtimes from author /test_author/ are returned
//TEST_F(DowntimeFinderFindMatchingAllTest, MultipleAuthor) {
//  criterias.push_back(downtime_finder::criteria("author", "test_author"));
//  result = _dtf->find_matching_all(criterias);
//  expected.push_back(1);
//  expected.push_back(4);
//  expected.push_back(5);
//  ASSERT_EQ(result, expected);
//}
//
//// Given a downtime_finder object with the test downtime list
//// When find_matching_all() is called with the criteria ("comment", "test_comment")
//// Then all downtimes with comment "test_comment" are returned
//TEST_F(DowntimeFinderFindMatchingAllTest, MultipleComment) {
//  criterias.push_back(downtime_finder::criteria("comment", "test_comment"));
//  result = _dtf->find_matching_all(criterias);
//  expected.push_back(2);
//  expected.push_back(3);
//  expected.push_back(5);
//  ASSERT_EQ(result, expected);
//}
//
//// Given a downtime_finder object with the test downtime list
//// When findMatchinAll() is called with criterias ("author", "test_author"), ("duration", "42") and ("comment", "test_comment")
//// Then all downtimes matching the criterias are returned
//TEST_F(DowntimeFinderFindMatchingAllTest, MultipleCriterias) {
//  criterias.push_back(downtime_finder::criteria("author", "test_author"));
//  criterias.push_back(downtime_finder::criteria("duration", "42"));
//  criterias.push_back(downtime_finder::criteria("comment", "test_comment"));
//  result = _dtf->find_matching_all(criterias);
//  expected.push_back(5);
//  ASSERT_EQ(result, expected);
//}
