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
#include "com/centreon/engine/configuration/connector.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ConfigConnector : public ::testing::Test {
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

// When I create a configuration::connector with an empty name
// Then an exception is thrown.
TEST_F(ConfigConnector, NewConnectorWithNoName) {
  configuration::connector cnn("");
  ASSERT_THROW(cnn.check_validity(), std::exception);
}

// When I create a configuration::connector with an empty connector_line
// Then an exception is thrown.
TEST_F(ConfigConnector, NewConnectorWithNameButNoCmdLine) {
  configuration::connector cnn("cnn");
  ASSERT_THROW(cnn.check_validity(), std::exception);
}

// When I create a configuration::connector with a non empty connector_line
// Then I get it.
TEST_F(ConfigConnector, NewConnectorWithName) {
  configuration::connector cnn("cnn");
  cnn.parse("connector_line", "echo 1");
  cnn.check_validity();
  ASSERT_TRUE(cnn.key() == cnn.connector_name());
}

// When I create a configuration::connector with a non empty connector_line
// And I copy it
// Then I get two equal connectors.
// When I change the second connector_line
// Then I get two distinct connectors.
// Then the order between the two connectors is as expected.
TEST_F(ConfigConnector, NewConnectorFromAnother) {
  configuration::connector cnn("cnn");
  cnn.parse("connector_line", "echo 1");
  configuration::connector cnn1(cnn);
  ASSERT_EQ(cnn, cnn1);
  cnn1.parse("connector_line", "echo 2");
  ASSERT_TRUE(cnn != cnn1);
  ASSERT_TRUE(cnn < cnn1);
  cnn1.parse("connector_name", "cnn1");
  ASSERT_TRUE(cnn < cnn1);
}

//// When I create a configuration::command with a non empty command_line
//// And I create a second one with just a name.
//// And I merge the first one into the second.
//// Then I get a second command with the same command line.
//TEST_F(ConfigConnector, ConnectorMerge) {
//  configuration::command cmd("cmd");
//  cmd.parse("command_line", "echo 1");
//  cmd.parse("connector", "perl");
//  configuration::command cmd1("cmd1");
//  cmd1.merge(cmd);
//  ASSERT_TRUE(cmd.command_line() == cmd1.command_line());
//  ASSERT_TRUE(cmd.connector() == cmd1.connector());
//}
