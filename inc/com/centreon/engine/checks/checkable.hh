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

#ifndef CCE_CHECKS_CHECKABLE_HH
#  define CCE_CHECKS_CHECKABLE_HH

#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/objects/command.hh"
#  include "com/centreon/engine/objects/timeperiod.hh"

CCE_BEGIN()

namespace              checks {

  /**
   *  @class checkable checkable.hh "com/centreon/engine/checks/checkable.hh"
   *  @brief Object executing checks.
   *
   */
  class                checkable {
   public:
                       checkable();
                       checkable(checkable const& other);
    virtual            ~checkable();
    checkable&         operator=(checkable const& other);
    bool               are_checks_enabled() const;
    bool               get_accept_passive_service_checks() const;
    command*           get_check_command() const;
    std::string        get_check_command_args() const;
    bool               get_check_freshness() const;
    int                get_check_options() const;
    timeperiod*        get_check_period() const;
    int                get_check_type() const;
    int                get_current_attempt() const;
    int                get_current_event_id() const;
    int                get_current_problem_id() const;
    int                get_current_state() const;
    command*           get_event_handler() const;
    double             get_execution_time() const;
    int                get_freshness_threshold() const;
    int                get_initial_state() const;
    time_t             get_last_check() const;
    int                get_last_event_id() const;
    int                get_last_state() const;
    int                get_last_hard_state() const;
    time_t             get_last_hard_state_change() const;
    int                get_last_problem_id() const;
    time_t             get_last_state_change() const;
    double             get_latency() const;
    std::string const& get_long_output() const;
    int                get_max_attempts() const;
    time_t             get_next_check() const;
    int                get_normal_check_interval() const;
    std::string const& get_output() const;
    double             get_percent_state_change() const;
    std::string const& get_perfdata() const;
    bool               get_process_perfdata() const;
    int                get_retry_check_interval() const;
    bool               get_should_be_scheduled() const;
    int                get_state_type() const;
    std::string const& get_timezone() const;
    bool               has_been_checked() const;
    bool               host_problem_at_last_check() const;
    bool               is_being_freshened() const;
    bool               is_event_handler_enabled() const;
    bool               is_executing() const;
    bool               is_flap_detection_enabled() const;
    bool               is_flapping() const;
    bool               is_obsessed_over() const;
    void               set_being_freshened(bool freshened);
    void               set_check_options(int options);
    void               set_check_type(int type);
    void               set_checked(bool checked);
    void               set_current_attempt(int attempt);
    void               set_current_event_id(int id);
    void               set_current_problem_id(int id);
    void               set_current_state(int state);
    void               set_executing(bool executing);
    void               set_execution_time(double execution_time);
    void               set_host_problem_at_last_check(bool problem);
    void               set_last_check(time_t last_check);
    void               set_last_event_id(int id);
    void               set_last_hard_state(int state);
    void               set_last_hard_state_change(time_t last_change);
    void               set_last_problem_id(int id);
    void               set_last_state(int state);
    void               set_last_state_change(time_t last_change);
    void               set_latency(double latency);
    void               set_long_output(std::string const& output);
    void               set_next_check(time_t next_check);
    void               set_output(std::string const& output);
    void               set_perfdata(std::string const& perfdata);
    void               set_state_type(int state);
    void               set_should_be_scheduled(bool schedule);
  };

}

CCE_END()

#endif // !CCE_CHECKS_CHECKABLE_HH
