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
#include <list>
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/macros/grab_host.hh"
#include "com/centreon/engine/objects/hostgroup.hh"
#include "com/centreon/engine/service.hh"
#include "../timeperiod/utils.hh"

class MacrosHost : public ::testing::Test {
 public:
  void SetUp() {
    _expected.clear();
    _free_macro = 0;
    _grabbed = NULL;
    memset(&_mac, 0, sizeof(_mac));
    grab_host_macros_r(&_mac, &_host);
    return ;
  }

  void TearDown() {
    if (_free_macro)
      delete[] _grabbed;
    return ;
  }

  void add_services_to_host() {
    // OK.
    for (int i(0); i < 4; ++i) {
      service s;
      s.set_current_state(0);
      _services.push_back(s);
      _host.add_service(&_services.back());
    }
    // WARNING.
    for (int i(0); i < 3; ++i) {
      service s;
      s.set_current_state(1);
      _services.push_back(s);
      _host.add_service(&_services.back());
    }
    // UNKNOWN.
    for (int i(0); i < 2; ++i) {
      service s;
      s.set_current_state(3);
      _services.push_back(s);
      _host.add_service(&_services.back());
    }
    // CRITICAL.
    for (int i(0); i < 1; ++i) {
      service s;
      s.set_current_state(2);
      _services.push_back(s);
      _host.add_service(&_services.back());
    }
    return ;
  }

  void grab_and_check(int id) {
    ASSERT_EQ(
      grab_standard_host_macro_r(
        &_mac,
        id,
        &_host,
        &_grabbed,
        &_free_macro),
      OK);
    ASSERT_TRUE(_grabbed);
    ASSERT_EQ(_grabbed, _expected);
    grab_host_macros_r(&_mac, &_host); // Cleanup.
    return ;
  }

 protected:
  std::string   _expected;
  int           _free_macro;
  char*         _grabbed;
  host          _host;
  nagios_macros _mac;
  std::list<service>
                _services;
};

// Given a host with its properties set
// And a nagios_macros object initialized with grab_host_macros_r()
// When grab_standard_host_macro() is called
// Then it provides the expected macro output

TEST_F(MacrosHost, GrabHostName) {
  _expected = "my host name";
  _host.set_name(_expected);
  grab_and_check(MACRO_HOSTNAME);
}

TEST_F(MacrosHost, GrabDisplayName) {
  _expected = "My Display Name !";
  _host.set_display_name(_expected);
  grab_and_check(MACRO_HOSTDISPLAYNAME);
}

TEST_F(MacrosHost, GrabAlias) {
  _expected = "This is an alias";
  _host.set_alias(_expected);
  grab_and_check(MACRO_HOSTALIAS);
}

TEST_F(MacrosHost, GrabAddress) {
  _expected = "host.address";
  _host.set_address(_expected);
  grab_and_check(MACRO_HOSTADDRESS);
}

TEST_F(MacrosHost, GrabState) {
  _expected = "DOWN";
  _host.set_current_state(1);
  grab_and_check(MACRO_HOSTSTATE);
}

TEST_F(MacrosHost, GrabStateId) {
  _expected = "1";
  _host.set_current_state(1);
  grab_and_check(MACRO_HOSTSTATEID);
}

TEST_F(MacrosHost, GrabLastState) {
  _expected = "UNREACHABLE";
  _host.set_last_state(2);
  grab_and_check(MACRO_LASTHOSTSTATE);
}

TEST_F(MacrosHost, GrabLastStateId) {
  _expected = "2";
  _host.set_last_state(2);
  grab_and_check(MACRO_LASTHOSTSTATEID);
}

TEST_F(MacrosHost, GrabCheckType) {
  _expected = "PASSIVE";
  _host.set_check_type(HOST_CHECK_PASSIVE);
  grab_and_check(MACRO_HOSTCHECKTYPE);
}

TEST_F(MacrosHost, GrabStateType) {
  _expected = "HARD";
  _host.set_current_state_type(HARD_STATE);
  grab_and_check(MACRO_HOSTSTATETYPE);
}

TEST_F(MacrosHost, GrabOutput) {
  _expected = "my output";
  _host.set_output(_expected);
  grab_and_check(MACRO_HOSTOUTPUT);
}

TEST_F(MacrosHost, GrabLongOutput) {
  _expected = "long output continued\non multiple lines";
  _host.set_long_output(_expected);
  grab_and_check(MACRO_LONGHOSTOUTPUT);
}

TEST_F(MacrosHost, GrabPerfdata) {
  _expected = "metric=42u;;;";
  _host.set_perfdata(_expected);
  grab_and_check(MACRO_HOSTPERFDATA);
}

TEST_F(MacrosHost, GrabCheckCommand) {
  _expected = "/bin/echo command";
  _host.set_check_command_args(_expected);
  grab_and_check(MACRO_HOSTCHECKCOMMAND);
}

TEST_F(MacrosHost, GrabCurrentAttempt) {
  _expected = "42";
  _host.set_current_attempt(42);
  grab_and_check(MACRO_HOSTATTEMPT);
}

TEST_F(MacrosHost, GrabMaxAttempts) {
  _expected = "100";
  _host.set_max_attempts(100);
  grab_and_check(MACRO_MAXHOSTATTEMPTS);
}

TEST_F(MacrosHost, GrabPercentChange) {
  _expected = "42.84";
  _host.set_percent_state_change(42.84);
  grab_and_check(MACRO_HOSTPERCENTCHANGE);
}

TEST_F(MacrosHost, GrabExecutionTime) {
  _expected = "0.3612";
  _host.set_execution_time(0.3612);
  grab_and_check(MACRO_HOSTEXECUTIONTIME);
}

TEST_F(MacrosHost, GrabLatency) {
  _expected = "3.14";
  _host.set_latency(3.14);
  grab_and_check(MACRO_HOSTLATENCY);
}

TEST_F(MacrosHost, GrabDowntime) {
  _expected = "3";
  for (int i(0); i < 3; ++i)
    _host.inc_scheduled_downtime_depth();
  grab_and_check(MACRO_HOSTDOWNTIME);
}

TEST_F(MacrosHost, GrabDuration) {
  _expected = "1d 2h 3m 4s";
  _host.set_last_state_change(1500000000);
  set_time(1500093784);
  grab_and_check(MACRO_HOSTDURATION);
}

TEST_F(MacrosHost, GrabDurationSec) {
  _expected = "123456";
  _host.set_last_state_change(1500000000);
  set_time(1500123456);
  grab_and_check(MACRO_HOSTDURATIONSEC);
}

TEST_F(MacrosHost, GrabLastHostCheck) {
  _expected = "1425879630";
  _host.set_last_check(strtol(_expected.c_str(), NULL, 10));
  grab_and_check(MACRO_LASTHOSTCHECK);
}

TEST_F(MacrosHost, GrabLastHostStateChange) {
  _expected = "1587456321";
  _host.set_last_state_change(strtol(_expected.c_str(), NULL, 10));
  grab_and_check(MACRO_LASTHOSTSTATECHANGE);
}

TEST_F(MacrosHost, GrabLastHostUp) {
  _expected = "1432145446";
  _host.set_last_time_up(strtol(_expected.c_str(), NULL, 10));
  grab_and_check(MACRO_LASTHOSTUP);
}

TEST_F(MacrosHost, GrabLastHostDown) {
  _expected = "1587430258";
  _host.set_last_time_down(strtol(_expected.c_str(), NULL, 10));
  grab_and_check(MACRO_LASTHOSTDOWN);
}

TEST_F(MacrosHost, GrabLastHostUnreachable) {
  _expected = "1589632156";
  _host.set_last_time_unreachable(strtol(_expected.c_str(), NULL, 10));
  grab_and_check(MACRO_LASTHOSTUNREACHABLE);
}

TEST_F(MacrosHost, GrabNotificationNumber) {
  _expected = "147852";
  _host.set_current_notification_number(147852);
  grab_and_check(MACRO_HOSTNOTIFICATIONNUMBER);
}

TEST_F(MacrosHost, GrabNotificationId) {
  _expected = "456987";
  _host.set_current_notification_id(456987);
  grab_and_check(MACRO_HOSTNOTIFICATIONID);
}

TEST_F(MacrosHost, GrabEventId) {
  _expected = "159753";
  _host.set_current_event_id(159753);
  grab_and_check(MACRO_HOSTEVENTID);
}

TEST_F(MacrosHost, GrabLastEventId) {
  _expected = "1478963";
  _host.set_last_event_id(1478963);
  grab_and_check(MACRO_LASTHOSTEVENTID);
}

TEST_F(MacrosHost, GrabProblemId) {
  _expected = "12369";
  _host.set_current_problem_id(12369);
  grab_and_check(MACRO_HOSTPROBLEMID);
}

TEST_F(MacrosHost, GrabLastProblemId) {
  _expected = "12368";
  _host.set_last_problem_id(12368);
  grab_and_check(MACRO_LASTHOSTPROBLEMID);
}

TEST_F(MacrosHost, GrabActionUrl) {
  _expected = "http://website.com/action";
  _host.set_action_url(_expected);
  grab_and_check(MACRO_HOSTACTIONURL);
}

TEST_F(MacrosHost, GrabNotesUrl) {
  _expected = "https://myurl.net/notes";
  _host.set_notes_url(_expected);
  grab_and_check(MACRO_HOSTNOTESURL);
}

TEST_F(MacrosHost, GrabNotes) {
  _expected = "Noting something because it might be useful";
  _host.set_notes(_expected);
  grab_and_check(MACRO_HOSTNOTES);
}

TEST_F(MacrosHost, GrabGroupNames) {
  _expected = "Group1,Group2,Group3";
  hostgroup hg[3];
  for (int i(0); i < sizeof(hg) / sizeof(*hg); ++i)
    memset(hg + i, 0, sizeof(*hg));
  hg[0].group_name = new char[7];
  strcpy(hg[0].group_name, "Group1");
  _host.add_group(hg + 0);
  hg[1].group_name = new char[7];
  strcpy(hg[1].group_name, "Group2");
  _host.add_group(hg + 1);
  hg[2].group_name = new char[7];
  strcpy(hg[2].group_name, "Group3");
  _host.add_group(hg + 2);
  grab_and_check(MACRO_HOSTGROUPNAMES);
  for (int i(0); i < sizeof(hg) / sizeof(*hg); ++i)
    delete[] hg[i].group_name;
}

TEST_F(MacrosHost, GrabTotalServices) {
  _expected = "10";
  add_services_to_host();
  grab_and_check(MACRO_TOTALHOSTSERVICES);
}

TEST_F(MacrosHost, GrabTotalServicesOk) {
  _expected = "4";
  add_services_to_host();
  grab_and_check(MACRO_TOTALHOSTSERVICESOK);
}

TEST_F(MacrosHost, GrabTotalServicesWarning) {
  _expected = "3";
  add_services_to_host();
  grab_and_check(MACRO_TOTALHOSTSERVICESWARNING);
}

TEST_F(MacrosHost, GrabTotalServicesUnknown) {
  _expected = "2";
  add_services_to_host();
  grab_and_check(MACRO_TOTALHOSTSERVICESUNKNOWN);
}

TEST_F(MacrosHost, GrabTotalServicesCritical) {
  _expected = "1";
  add_services_to_host();
  grab_and_check(MACRO_TOTALHOSTSERVICESCRITICAL);
}

TEST_F(MacrosHost, GrabAckAuthor) {
  _expected = "XXX";
  grab_and_check(MACRO_HOSTACKAUTHOR);
}

TEST_F(MacrosHost, GrabAckAuthorName) {
  _expected = "XXX";
  grab_and_check(MACRO_HOSTACKAUTHORNAME);
}

TEST_F(MacrosHost, GrabAckAuthorAlias) {
  _expected = "XXX";
  grab_and_check(MACRO_HOSTACKAUTHORALIAS);
}

TEST_F(MacrosHost, GrabAckComment) {
  _expected = "XXX";
  grab_and_check(MACRO_HOSTACKCOMMENT);
}

TEST_F(MacrosHost, GrabParents) {
  _expected = "Parent1,Parent2,Parent3";
  host h[3];
  h[0].set_name("Parent1");
  h[1].set_name("Parent2");
  h[2].set_name("Parent3");
  for (int i(0); i < sizeof(h) / sizeof(*h); ++i)
    _host.add_parent(h + i);
  grab_and_check(MACRO_HOSTPARENTS);
}

TEST_F(MacrosHost, GrabChildren) {
  _expected = "Child1,Child2,Child3";
  host h[3];
  h[0].set_name("Child1");
  h[1].set_name("Child2");
  h[2].set_name("Child3");
  for (int i(0); i < sizeof(h) / sizeof(*h); ++i)
    _host.add_child(h + i);
  grab_and_check(MACRO_HOSTCHILDREN);
}

TEST_F(MacrosHost, GrabHostId) {
  _expected = "42";
  _host.set_id(42);
  grab_and_check(MACRO_HOSTID);
}

TEST_F(MacrosHost, GrabTimezone) {
  _expected = ":Europe/Paris";
  _host.set_timezone(_expected);
  grab_and_check(MACRO_HOSTTIMEZONE);
}
