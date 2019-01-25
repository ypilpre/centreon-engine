/*
** Copyright 2018-2019 Centreon
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
#include "com/centreon/engine/macros/grab_service.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/engine/servicegroup.hh"
#include "../timeperiod/utils.hh"

class MacrosService : public ::testing::Test {
 public:
  void SetUp() {
    _expected.clear();
    _free_macro = 0;
    _grabbed = NULL;
    memset(&_mac, 0, sizeof(_mac));
    grab_service_macros_r(&_mac, &_service);
    return ;
  }

  void TearDown() {
    if (_free_macro)
      delete[] _grabbed;
    return ;
  }

  void grab_and_check(int id) {
    ASSERT_EQ(
      grab_standard_service_macro_r(
        &_mac,
        id,
        &_service,
        &_grabbed,
        &_free_macro),
      OK);
    ASSERT_TRUE(_grabbed);
    ASSERT_EQ(_grabbed, _expected);
    grab_service_macros_r(&_mac, &_service); // Cleanup.
    return ;
  }

 protected:
  std::string   _expected;
  int           _free_macro;
  char*         _grabbed;
  nagios_macros _mac;
  service       _service;
};

// Given a service with its properties set
// And a nagios_macros object initialized with grab_service_macros_r()
// When grab_standard_service_macro() is called
// Then it provides the expected macro output

TEST_F(MacrosService, GrabDescription) {
  _expected = "my service description";
  _service.set_description(_expected);
  grab_and_check(MACRO_SERVICEDESC);
}

TEST_F(MacrosService, GrabDisplayName) {
  _expected = "My Display Name !";
  _service.set_display_name(_expected);
  grab_and_check(MACRO_SERVICEDISPLAYNAME);
}

TEST_F(MacrosService, GrabOutput) {
  _expected = "my output";
  _service.set_output(_expected);
  grab_and_check(MACRO_SERVICEOUTPUT);
}

TEST_F(MacrosService, GrabLongOutput) {
  _expected = "long output continued\non multiple lines";
  _service.set_long_output(_expected);
  grab_and_check(MACRO_LONGSERVICEOUTPUT);
}

TEST_F(MacrosService, GrabPerfdata) {
  _expected = "metric=42u;;;";
  _service.set_perfdata(_expected);
  grab_and_check(MACRO_SERVICEPERFDATA);
}

TEST_F(MacrosService, GrabCheckCommand) {
  _expected = "/bin/echo command";
  _service.set_check_command_args(_expected);
  grab_and_check(MACRO_SERVICECHECKCOMMAND);
}

TEST_F(MacrosService, GrabCheckType) {
  _expected = "PASSIVE";
  _service.set_check_type(SERVICE_CHECK_PASSIVE);
  grab_and_check(MACRO_SERVICECHECKTYPE);
}

TEST_F(MacrosService, GrabStateType) {
  _expected = "HARD";
  _service.set_current_state_type(HARD_STATE);
  grab_and_check(MACRO_SERVICESTATETYPE);
}

TEST_F(MacrosService, GrabState) {
  _expected = "WARNING";
  _service.set_current_state(1);
  grab_and_check(MACRO_SERVICESTATE);
}

TEST_F(MacrosService, GrabStateId) {
  _expected = "1";
  _service.set_current_state(1);
  grab_and_check(MACRO_SERVICESTATEID);
}

TEST_F(MacrosService, GrabLastState) {
  _expected = "CRITICAL";
  _service.set_last_state(2);
  grab_and_check(MACRO_LASTSERVICESTATE);
}

TEST_F(MacrosService, GrabLastStateId) {
  _expected = "2";
  _service.set_last_state(2);
  grab_and_check(MACRO_LASTSERVICESTATEID);
}

TEST_F(MacrosService, GrabIsVolatile) {
  _expected = "1";
  _service.set_volatile(true);
  grab_and_check(MACRO_SERVICEISVOLATILE);
}

TEST_F(MacrosService, GrabAttempt) {
  _expected = "42";
  _service.set_current_attempt(42);
  grab_and_check(MACRO_SERVICEATTEMPT);
}

TEST_F(MacrosService, GrabMaxAttempts) {
  _expected = "100";
  _service.set_max_attempts(100);
  grab_and_check(MACRO_MAXSERVICEATTEMPTS);
}

TEST_F(MacrosService, GrabExecutionTime) {
  _expected = "0.3612";
  _service.set_execution_time(0.3612);
  grab_and_check(MACRO_SERVICEEXECUTIONTIME);
}

TEST_F(MacrosService, GrabLatency) {
  _expected = "3.14";
  _service.set_latency(3.14);
  grab_and_check(MACRO_SERVICELATENCY);
}

TEST_F(MacrosService, GrabLastCheck) {
  _expected = "1425879630";
  _service.set_last_check(strtol(_expected.c_str(), NULL, 10));
  grab_and_check(MACRO_LASTSERVICECHECK);
}

TEST_F(MacrosService, GrabLastStateChange) {
  _expected = "1587456321";
  _service.set_last_state_change(strtol(_expected.c_str(), NULL, 10));
  grab_and_check(MACRO_LASTSERVICESTATECHANGE);
}

TEST_F(MacrosService, GrabLastOk) {
  _expected = "1432145446";
  _service.set_last_time_ok(strtol(_expected.c_str(), NULL, 10));
  grab_and_check(MACRO_LASTSERVICEOK);
}

TEST_F(MacrosService, GrabLastWarning) {
  _expected = "1587430258";
  _service.set_last_time_warning(strtol(_expected.c_str(), NULL, 10));
  grab_and_check(MACRO_LASTSERVICEWARNING);
}

TEST_F(MacrosService, GrabLastUnknown) {
  _expected = "1589632156";
  _service.set_last_time_unknown(strtol(_expected.c_str(), NULL, 10));
  grab_and_check(MACRO_LASTSERVICEUNKNOWN);
}

TEST_F(MacrosService, GrabLastCritical) {
  _expected = "1598875621";
  _service.set_last_time_critical(strtol(_expected.c_str(), NULL, 10));
  grab_and_check(MACRO_LASTSERVICECRITICAL);
}

TEST_F(MacrosService, GrabDowntime) {
  _expected = "3";
  for (int i(0); i < 3; ++i)
    _service.inc_scheduled_downtime_depth();
  grab_and_check(MACRO_SERVICEDOWNTIME);
}

TEST_F(MacrosService, GrabPercentChange) {
  _expected = "42.84";
  _service.set_percent_state_change(42.84);
  grab_and_check(MACRO_SERVICEPERCENTCHANGE);
}

TEST_F(MacrosService, GrabDuration) {
  _expected = "1d 2h 3m 4s";
  _service.set_last_state_change(1500000000);
  set_time(1500093784);
  grab_and_check(MACRO_SERVICEDURATION);
}

TEST_F(MacrosService, GrabDurationSec) {
  _expected = "123456";
  _service.set_last_state_change(1500000000);
  set_time(1500123456);
  grab_and_check(MACRO_SERVICEDURATIONSEC);
}

TEST_F(MacrosService, GrabNotificationNumber) {
  _expected = "147852";
  _service.set_current_notification_number(147852);
  grab_and_check(MACRO_SERVICENOTIFICATIONNUMBER);
}

TEST_F(MacrosService, GrabNotificationId) {
  _expected = "456987";
  _service.set_current_notification_id(456987);
  grab_and_check(MACRO_SERVICENOTIFICATIONID);
}

TEST_F(MacrosService, GrabEventId) {
  _expected = "159753";
  _service.set_current_event_id(159753);
  grab_and_check(MACRO_SERVICEEVENTID);
}

TEST_F(MacrosService, GrabLastEventId) {
  _expected = "1478963";
  _service.set_last_event_id(1478963);
  grab_and_check(MACRO_LASTSERVICEEVENTID);
}

TEST_F(MacrosService, GrabProblemId) {
  _expected = "12369";
  _service.set_current_problem_id(12369);
  grab_and_check(MACRO_SERVICEPROBLEMID);
}

TEST_F(MacrosService, GrabLastProblemId) {
  _expected = "12368";
  _service.set_last_problem_id(12368);
  grab_and_check(MACRO_LASTSERVICEPROBLEMID);
}

TEST_F(MacrosService, GrabActionUrl) {
  _expected = "http://website.com/action";
  _service.set_action_url(_expected);
  grab_and_check(MACRO_SERVICEACTIONURL);
}

TEST_F(MacrosService, GrabNotesUrl) {
  _expected = "https://myurl.net/notes";
  _service.set_notes_url(_expected);
  grab_and_check(MACRO_SERVICENOTESURL);
}

TEST_F(MacrosService, GrabNotes) {
  _expected = "Noting something because it might be useful";
  _service.set_notes(_expected);
  grab_and_check(MACRO_SERVICENOTES);
}

TEST_F(MacrosService, GrabGroupNames) {
  _expected = "Group1,Group2,Group3";
  servicegroup sg[3];
  for (int i(0); i < sizeof(sg) / sizeof(*sg); ++i)
    memset(sg + i, 0, sizeof(*sg));
  sg[0].set_name("Group1");
  _service.add_group(sg + 0);
  sg[1].set_name("Group2");
  _service.add_group(sg + 1);
  sg[2].set_name("Group3");
  _service.add_group(sg + 2);
  grab_and_check(MACRO_SERVICEGROUPNAMES);
}

// XXX
// TEST_F(MacrosService, GrabAckAuthor) {
//   _expected = "XXX";
//   grab_and_check(MACRO_SERVICEACKAUTHOR);
// }

// TEST_F(MacrosService, GrabAckAuthorName) {
//   _expected = "XXX";
//   grab_and_check(MACRO_SERVICEACKAUTHORNAME);
// }

// TEST_F(MacrosService, GrabAckAuthorAlias) {
//   _expected = "XXX";
//   grab_and_check(MACRO_SERVICEACKAUTHORALIAS);
// }

// TEST_F(MacrosService, GrabAckComment) {
//   _expected = "XXX";
//   grab_and_check(MACRO_SERVICEACKCOMMENT);
// }

TEST_F(MacrosService, GrabTimezone) {
  _expected = ":Europe/Paris";
  _service.set_timezone(_expected);
  grab_and_check(MACRO_SERVICETIMEZONE);
}

TEST_F(MacrosService, GrabServiceId) {
  _expected = "42";
  _service.set_id(42);
  grab_and_check(MACRO_SERVICEID);
}
