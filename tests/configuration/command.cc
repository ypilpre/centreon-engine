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
#include "com/centreon/engine/configuration/command.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/notifications/notifiable.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ConfigCommand : public ::testing::Test {
 public:
  void SetUp() {
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

// When I create a configuration::command with an empty name
// Then an exception is thrown.
TEST_F(ConfigCommand, NewCommandWithNoName) {
  configuration::command cmd("");
  ASSERT_THROW(cmd.check_validity(), std::exception);
}

// When I create a configuration::command with an empty command_line
// Then an exception is thrown.
TEST_F(ConfigCommand, NewCommandWithNameButNoCmdLine) {
  configuration::command cmd("cmd");
  ASSERT_THROW(cmd.check_validity(), std::exception);
}

// When I create a configuration::command with a non empty command_line
// Then I get it.
TEST_F(ConfigCommand, NewCommandWithName) {
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  cmd.check_validity();
  ASSERT_TRUE(cmd.key() == cmd.command_name());
}

// When I create a configuration::command with a non empty command_line
// And I copy it
// Then I get two equal commands.
// When I change the second command_line
// Then I get two distinct commands.
// Then the order between the two commands is as expected.
TEST_F(ConfigCommand, NewCommandFromAnother) {
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  configuration::command cmd1(cmd);
  ASSERT_EQ(cmd, cmd1);
  cmd1.parse("command_line", "echo 2");
  ASSERT_TRUE(cmd != cmd1);
  ASSERT_TRUE(cmd < cmd1);
  cmd1.parse("command_name", "cmd1");
  ASSERT_TRUE(cmd < cmd1);
}

// When I create a configuration::command with a non empty command_line
// And I create a second one with just a name.
// And I merge the first one into the second.
// Then I get a second command with the same command line.
TEST_F(ConfigCommand, CommandsMerge) {
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  cmd.parse("connector", "perl");
  configuration::command cmd1("cmd1");
  cmd1.merge(cmd);
  ASSERT_TRUE(cmd.command_line() == cmd1.command_line());
  ASSERT_TRUE(cmd.connector() == cmd1.connector());
}
