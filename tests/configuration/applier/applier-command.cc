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
#include "../../timeperiod/utils.hh"
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/connector.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/command.hh"
#include "com/centreon/engine/configuration/connector.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ApplierCommand : public ::testing::Test {
 public:
  void SetUp() {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a contact
    commands::set::load();                  // Needed to create commands
  }

  void TearDown() {
    commands::set::unload();
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

};

// Given a command applier
// And a configuration command just with a name
// Then the applier add_object addsthe command in the configuration set
// but not in the commands map (the command is unusable).
TEST_F(ApplierCommand, UnusableCommandFromConfig) {
  configuration::applier::command aply;
  configuration::command cmd("cmd");
  aply.add_object(cmd);
  set_command s(config->commands());
  ASSERT_EQ(s.size(), 1);
  command_map const& cm(configuration::applier::state::instance().commands());
  ASSERT_EQ(cm.size(), 0);
}

// Given a command applier
// And a configuration command with a name and a command line
// Then the applier add_object adds the command into the configuration set
// and the commands map (accessible from commands::set::instance()).
TEST_F(ApplierCommand, NewCommandFromConfig) {
  configuration::applier::command aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  aply.add_object(cmd);
  set_command s(config->commands());
  ASSERT_EQ(s.size(), 1);
  shared_ptr<commands::command> cc(commands::set::instance().get_command("cmd"));
  ASSERT_EQ(cc->get_name(), "cmd");
  ASSERT_EQ(cc->get_command_line(), "echo 1");
}

// Given a command applier
// And a configuration command with a name, a command line and a connector
// Then the applier add_object adds the command into the configuration set
// but not in the commands map (the connector is not defined).
TEST_F(ApplierCommand, NewCommandWithEmptyConnectorFromConfig) {
  configuration::applier::command aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  cmd.parse("connector", "perl");
  ASSERT_THROW(aply.add_object(cmd), std::exception);
  set_command s(config->commands());
  ASSERT_EQ(s.size(), 1);
  ASSERT_THROW(shared_ptr<commands::command> cc(
                 commands::set::instance().get_command("cmd")),
               std::exception);
}

// Given a command applier
// And a configuration command with a name, a command line and a connector
// And the connector is well defined.
// Then the applier add_object adds the command into the configuration set
// but not in the commands map (the connector is not defined).
TEST_F(ApplierCommand, NewCommandWithConnectorFromConfig) {
  configuration::applier::command aply;
  configuration::applier::connector cnn_aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  cmd.parse("connector", "perl");
  configuration::connector cnn("perl");

  cnn_aply.add_object(cnn);
  aply.add_object(cmd);

  set_command s(config->commands());
  ASSERT_EQ(s.size(), 1);
  shared_ptr<commands::command> cc(commands::set::instance().get_command("cmd"));
  ASSERT_EQ(cc->get_name(), "cmd");
  ASSERT_EQ(cc->get_command_line(), "echo 1");
}
