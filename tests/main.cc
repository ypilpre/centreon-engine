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

#include <gtest/gtest.h>
#include "com/centreon/clib.hh"
#include "com/centreon/engine/timezone_manager.hh"
#include "com/centreon/logging/engine.hh"
#include "com/centreon/logging/file.hh"

class  CentreonEngineEnvironment : public testing::Environment {
public:
  void SetUp() {
    setenv("TZ", ":Europe/Paris", 1);
    com::centreon::clib::load(com::centreon::clib::with_logging_engine);
    // Enable all Centreon Engine logs.
    /*
    com::centreon::logging::backend*
      backend(new com::centreon::logging::file(stdout));
    com::centreon::logging::engine::instance().add(
      backend,
      -1,
      7);
    */
    com::centreon::engine::timezone_manager::load();
    return ;
  }

  void TearDown() {
    com::centreon::engine::timezone_manager::unload();
    com::centreon::clib::unload();
    return ;
  }
};

/**
 *  Tester entry point.
 *
 *  @param[in] argc  Argument count.
 *  @param[in] argv  Argument values.
 *
 *  @return 0 on success, any other value on failure.
 */
int main(int argc, char* argv[]) {
  // GTest initialization.
  testing::InitGoogleTest(&argc, argv);

  // Set specific environment.
  testing::AddGlobalTestEnvironment(new CentreonEngineEnvironment());

  // Run all tests.
  return (RUN_ALL_TESTS());
}
