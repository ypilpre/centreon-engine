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
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/notifications/notifiable.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::notifications;

extern configuration::state* config;

class CommentTest : public ::testing::Test {
 public:
  void SetUp() {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();

    configuration::applier::host hst_aply;
    configuration::applier::service svc_aply;
    configuration::service csvc;

    csvc.parse("service_description", "check description");
    csvc.parse("host_name", "test_host");

    configuration::host hst;
    hst.parse("host_name", "test_host");
    hst_aply.add_object(hst);
    svc_aply.add_object(csvc);
    csvc.parse("hosts", "test_host");

    configuration::applier::command cmd_aply;
    configuration::command cmd("cmd");
    cmd.parse("command_line", "echo 1");
    csvc.parse("check_command", "cmd");
    cmd_aply.add_object(cmd);

    svc_aply.resolve_object(csvc);

    _service = configuration::applier::state::instance()
                 .services().begin()->second.get();

    set_time(20);
    contact_map& cm(configuration::applier::state::instance().contacts());
    cm["test"] = shared_ptr<engine::contact>(new engine::contact());
    cm["test"]->set_name("test");
    _service->add_contact(
      configuration::applier::state::instance().contacts_find("test").get());
    _service->set_notify_on(notifiable::ON_RECOVERY, true);
    _service->set_notify_on(notifiable::ON_FLAPPING, true);
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

 protected:
  engine::service*  _service;
};

// Given a service
// When a comment is added to the service
// Then its id is fixed to 1.
// When a second comment is added to the service
// Then its id is fixed to 2.
// When comments are removed
// Then the global map containing comments is newly empty.
TEST_F(CommentTest, AddRemoveComment) {
  comment* cmt(comment::add_new_comment(
    comment::HOST_COMMENT,
    comment::FLAPPING_COMMENT,
    _service,
    time(NULL),
    "test author",
    "test comment",
    0,
    comment::COMMENTSOURCE_INTERNAL,
    false,
    static_cast<time_t>(0)));
  ASSERT_EQ(cmt->get_id(), 1);
  comment* cmt1(comment::add_new_comment(
    comment::HOST_COMMENT,
    comment::FLAPPING_COMMENT,
    _service,
    time(NULL),
    "test author1",
    "test comment1",
    0,
    comment::COMMENTSOURCE_INTERNAL,
    false,
    static_cast<time_t>(0)));
  ASSERT_EQ(cmt->get_id(), 1);
  ASSERT_EQ(cmt1->get_id(), 2);
  comment::delete_comment(cmt->get_id());
  comment::delete_comment(cmt1->get_id());
  ASSERT_TRUE(comment_list.empty());
}
