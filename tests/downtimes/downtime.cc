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
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class Downtime : public ::testing::Test {
 public:
  void SetUp() {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();
    set_time(20);
    configuration::service svc_cfg;
    svc_cfg.parse("host", "test-host");
    svc_cfg.parse("service_description", "test-svc-description");
    configuration::applier::service aply;
    aply.add_object(svc_cfg);
    _my_svc = find_service("test-host", "test-svc-description");
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

  shared_ptr<engine::service> _my_svc;
};

// Given a notifier in downtime
// When the notify method is called with PROBLEM type
// Then the filter method returns false and no notification is sent.
//TEST_F(Downtime, SimpleServiceDowntime) {
//  ASSERT_TRUE(_my_svc->schedule_downtime(
//    downtime::SERVICE_DOWNTIME,
//    time(NULL),
//    "admin",
//    "test downtime",
//    40, 60,
//    false,
//    0, 20) == 0);
//}
