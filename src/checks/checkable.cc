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

#include "com/centreon/engine/checks/checkable.hh"

using namespace com::centreon::engine::checks;

/**
 *  Default constructor.
 */
checkable::checkable()
  // XXX
{}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
checkable::checkable(checkable const& other) {
  _internal_copy(other);
}

/**
 *  Destructor.
 */
checkable::~checkable() {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
checkable& checkable::operator=(checkable const& other) {
  if (this != &other)
    _internal_copy(other);
  return (*this);
}

/**************************************
*                                     *
*         Check configuration         *
*                                     *
**************************************/

/**
 *  Check if active monitoring checks are enabled.
 *
 *  @return True if active checks are enabled.
 */
bool checkable::get_active_checks_enabled() const {
  return (_active_checks_enabled);
}

/**
 *  Enable or disable active monitoring checks.
 *
 *  @param[in] enable  True to enable active checks on object.
 */
void checkable::set_active_checks_enabled(bool enable) {
  _active_checks_enabled = enable;
  return ;
}

/**
 *  Get check command object.
 *
 *  @return Check command of this object. NULL for none.
 */
command* checkable::get_check_command() const {
  return (_check_command);
}

/**
 *  Set check command object.
 *
 *  @param[in] cmd  Check command object.
 */
void checkable::set_check_command(command* cmd) {
  _check_command = cmd;
  return ;
}

/**
 *  Get check command arguments (ARGx).
 *
 *  @return Unparsed check command arguments (command!ARG1!ARG2).
 */
std::string const& checkable::get_check_command_args() const {
  return (_check_command_args);
}

/**
 *  Set check command arguments.
 *
 *  @param[in] args  Check command arguments.
 */
void checkable::set_check_command_args(std::string const& args) {
  _check_command_args = args;
  return ;
}

/**
 *  Get check timeperiod.
 *
 *  @return Check timeperiod of this object. NULL for none.
 */
timeperiod* checkable::get_check_period() const {
  return (_check_period);
}

/**
 *  Set check timeperiod.
 *
 *  @param[in] tp  Check timeperiod.
 */
void checkable::set_check_period(timeperiod* tp) {
  _check_period = tp;
  return ;
}

/**
 *  Get the max number of attempts before considering this object HARD.
 *
 *  @return Max check attempts.
 */
int checkable::get_max_attempts() const {
  return (_max_attempts);
}

/**
 *  Set the maximum number of attempts.
 *
 *  @param[in] attempts  Maximum number of attempts.
 */
void checkable::set_max_attempts(int attempts) {
  _max_attempts = attempts;
  return ;
}

/**
 *  Get the normal check interval.
 *
 *  @return Normal check interval in seconds.
 */
int checkable::get_normal_check_interval() const {
  return (_normal_check_interval);
}

/**
 *  Set the normal check interval.
 *
 *  @param[in] interval  New normal check interval in seconds.
 */
void checkable::set_normal_check_interval(int interval) {
  _normal_check_interval = interval;
  return ;
}

/**
 *  Check if obsessive compulsive processing is enabled.
 *
 *  @return True if obsessive compulsive processing is enabled.
 */
bool checkable::get_ocp_enabled() const {
  return (_ocp_enabled);
}

/**
 *  Enable or disable obsessive compulsive processing.
 *
 *  @param[in] enable  True to enable.
 */
void checkable::set_ocp_enabled(bool enable) {
  _ocp_enabled = enable;
  return ;
}

/**
 *  Check if passive checks are accepted.
 *
 *  @return True if passive checks are accepted.
 */
bool checkable::get_passive_checks_enabled() const {
  return (_passive_checks_enabled);
}

/**
 *  Enable or disable passive checks on object.
 *
 *  @param[in] enable  True to enable passive checks on object.
 */
void checkable::set_passive_checks_enabled(bool enable) {
  _passive_checks_enabled = enable;
  return ;
}

/**
 *  Check if performance data should be externally processed.
 *
 *  @return True if performance data should be externally processed.
 */
bool checkable::get_process_perfdata() const {
  return (_process_perfdata);
}

/**
 *  Enable or disable performance data processing.
 *
 *  @param[in] process  True to enable external performance data
 *                      processing.
 */
void checkable::set_process_perfdata(bool process) {
  _process_perfdata = process;
  return ;
}

/**
 *  Get retry check interval.
 *
 *  @return Retry check interval in seconds.
 */
int checkable::get_retry_check_interval() const {
  return (_retry_check_interval);
}

/**
 *  Set retry check interval.
 *
 *  @param[in] interval  New retry check interval.
 */
void checkable::set_retry_check_interval(int interval) {
  _retry_check_interval = interval;
  return ;
}

/**
 *  Get timezone.
 *
 *  @return Timezone.
 */
std::string const& checkable::get_timezone() const {
  return (_timezone);
}

/**************************************
*                                     *
*            Check runtime            *
*                                     *
**************************************/

/**
 *  Get check options.
 *
 *  @return Check options.
 */
int checkable::get_check_options() const {
  return (_check_options);
}

/**
 *  Set check options.
 *
 *  @param[in] options  New check options.
 */
void checkable::set_check_options(int options) {
  _check_options = options;
  return ;
}

/**
 *  Get check type.
 *
 *  @return Check type.
 */
int checkable::get_check_type() const {
  return (_check_type);
}

/**
 *  Set check type.
 *
 *  @param[in] type  Check type.
 */
void checkable::set_check_type(int type) {
  _check_type = type;
  return ;
}

/**
 *  Get current check attempt.
 *
 *  @return Current check attempt number.
 */
int checkable::get_current_attempt() const {
  return (_current_attempt);
}

/**
 *  Set current check attempt.
 *
 *  @param[in] attempt  Current check attempt.
 */
void checkable::set_current_attempt(int attempt) {
  _current_attempt = attempt;
  return ;
}

/**
 *  Check if object is currently executing.
 *
 *  @return True if object is currently executing a check.
 */
bool checkable::get_executing() const {
  return (_executing);
}

/**
 *  Set whether a check is currently executing for this object.
 *
 *  @param[in] executing  True if a check is currently executing for
 *                        this object.
 */
void checkable::set_executing(bool executing) {
  _executing = executing;
  return ;
}

/**
 *  Get last plugin execution time.
 *
 *  @return Last plugin execution time in seconds.
 */
double checkable::get_execution_time() const {
  return (_execution_time);
}

/**
 *  Set last plugin execution time.
 *
 *  @param[in] execution_time  Last plugin execution time in seconds.
 */
void checkable::set_execution_time(double execution_time) {
  _execution_time = execution_time;
  return ;
}

/**
 *  Get last check.
 *
 *  @return Last time object was checked.
 */
time_t checkable::get_last_check() const {
  return (_last_check);
}

/**
 *  Set last time object was checked.
 *
 *  @param[in] last_check  Last time object was checked.
 */
void checkable::set_last_check(time_t last_check) {
  _last_check = last_check;
  return ;
}

/**
 *  Get latency.
 *
 *  @return Latency.
 */
double checkable::get_latency() const {
  return (_latency);
}

/**
 *  Set latency.
 *
 *  @param[in] latency  Last latency.
 */
void checkable::set_latency(double latency) {
  _latency = latency;
  return ;
}

/**
 *  Get long plugin output.
 *
 *  @return Long plugin output.
 */
std::string const& checkable::get_long_output() const {
  return (_long_output);
}

/**
 *  Set long plugin output.
 *
 *  @param[in] output  Long plugin output (everything after first line).
 */
void checkable::set_long_output(std::string const& output) {
  _long_output = output;
  return ;
}

/**
 *  Get the next time at which the object is supposed to be checked.
 *
 *  @return Next time at which object is supposed to be checked.
 */
time_t checkable::get_next_check() const {
  return (_next_check);
}

/**
 *  Set next time this object is supposed to be checked.
 *
 *  @param[in] next_check  Next expected check.
 */
void checkable::set_next_check(time_t next_check) {
  _next_check = next_check;
  return ;
}

/**
 *  Get first line of plugin output.
 *
 *  @return First line of plugin output.
 */
std::string const& checkable::get_output() const {
  return (_output);
}

/**
 *  Set first line of plugin output.
 *
 *  @param[in] output  First line of plugin output.
 */
void checkable::set_output(std::string const& output) {
  _output = output;
  return ;
}

/**
 *  Get performance data.
 *
 *  @return Unparsed performance data.
 */
std::string const& checkable::get_perfdata() const {
  return (_perfdata);
}

/**
 *  Set performance data.
 *
 *  @param[in] perfdata  Performance data.
 */
void checkable::set_perfdata(std::string const& perfdata) {
  _perfdata = perfdata;
  return ;
}

/**
 *  Check if object should be scheduled.
 *
 *  @return True if object should be scheduled.
 */
bool checkable::get_should_be_scheduled() const {
  return (_should_be_scheduled);
}

/**
 *  Set whether or not this object should be scheduled.
 *
 *  @param[in] schedule  True if this object should be scheduled.
 */
void checkable::set_should_be_scheduled(bool schedule) {
  _should_be_scheduled = schedule;
  return ;
}

/**************************************
*                                     *
*            State runtime            *
*                                     *
**************************************/

/**
 *  Get current event ID.
 *
 *  @return Current event ID.
 */
int checkable::get_current_event_id() const {
  return (_current_event_id);
}

/**
 *  Set current event ID.
 *
 *  @param[in] id  Current event ID.
 */
void checkable::set_current_event_id(int id) {
  _current_event_id = id;
  return ;
}

/**
 *  Get current problem ID.
 *
 *  @return Current problem ID.
 */
int checkable::get_current_problem_id() const {
  return (_current_problem_id);
}

/**
 *  Set current problem ID.
 *
 *  @param[in] id  Current problem ID.
 */
void checkable::set_current_problem_id(int id) {
  _current_problem_id = id;
  return ;
}

/**
 *  Get current state.
 *
 *  @return Current state.
 */
int checkable::get_current_state() const {
  return (_current_state);
}

/**
 *  Set current state.
 *
 *  @param[in] state  Current state.
 */
void checkable::set_current_state(int state) {
  _current_state = state;
  return ;
}

/**
 *  Get current state type.
 *
 *  @return Current state type.
 */
int checkable::get_current_state_type() const {
  return (_current_state_type);
}

/**
 *  Set current state type.
 *
 *  @param[in] type  State type.
 */
void checkable::set_current_state_type(int type) {
  _current_state_type = type;
  return ;
}

/**
 *  Check if object has already been checked.
 *
 *  @return True if object has already been checked.
 */
bool checkable::get_has_been_checked() const {
  return (_has_been_checked);
}

/**
 *  Set if this object has already been checked.
 *
 *  @param[in] checked  True if this object has already been checked.
 */
void checkable::set_has_been_checked(bool checked) {
  _has_been_checked = checked;
  return ;
}

/**
 *  Check if service's host had a problem at last check.
 *
 *  @return True if host had problem at last check.
 */
bool checkable::get_host_problem_at_last_check() const {
  return (_host_problem_at_last_check);
}

/**
 *  Set whether or not service's host had a problem at last check.
 *
 *  @param[in] problem  True if host had a problem.
 */
void checkable::set_host_problem_at_last_check(bool problem) {
  _host_problem_at_last_check = problem;
  return ;
}

/**
 *  Get last event ID.
 *
 *  @return Last event ID.
 */
int checkable::get_last_event_id() const {
  return (_last_event_id);
}

/**
 *  Set last event ID.
 *
 *  @param[in] id  Last event ID.
 */
void checkable::set_last_event_id(int id) {
  _last_event_id = id;
  return ;
}

/**
 *  Get last object hard state.
 *
 *  @return Last object hard state.
 */
int checkable::get_last_hard_state() const {
  return (_last_hard_state);
}

/**
 *  Set last object hard state.
 *
 *  @param[in] state  Last hard state.
 */
void checkable::set_last_hard_state(int state) {
  _last_hard_state = state;
  return ;
}

/**
 *  Get last time that this object's hard state changed.
 *
 *  @return Last hard state change.
 */
time_t checkable::get_last_hard_state_change() const {
  return (_last_hard_state_change);
}

/**
 *  Set last time that this object changed its hard state.
 *
 *  @param[in] last_change  Last hard state change.
 */
void checkable::set_last_hard_state_change(time_t last_change) {
  _last_hard_state_change = last_change;
  return ;
}

/**
 *  Get last problem ID.
 *
 *  @return Last problem ID.
 */
int checkable::get_last_problem_id() const {
  return (_last_problem_id);
}

/**
 *  Set last problem ID.
 *
 *  @param[in] id  Last problem ID.
 */
void checkable::set_last_problem_id(int id) {
  _last_problem_id = id;
  return ;
}

/**
 *  Get last object state.
 *
 *  @return Last object state.
 */
int checkable::get_last_state() const {
  return (_last_state);
}

/**
 *  Set last object state.
 *
 *  @param[in] state  Last state.
 */
void checkable::set_last_state(int state) {
  _last_state = state;
  return ;
}

/**
 *  Get last time state changed.
 *
 *  @return Last time state changed.
 */
time_t checkable::get_last_state_change() const {
  return (_last_state_change);
}

/**
 *  Set last time state changed.
 *
 *  @param[in] last_change  Last time state changed.
 */
void checkable::set_last_state_change(time_t last_change) {
  _last_state_change = last_change;
  return ;
}

/**
 *  Get modified attributes.
 *
 *  @return A bitfield where modified attributes bits are set to 1.
 */
int checkable::get_modified_attributes() const {
  return (_modified_attributes);
}

/**
 *  Set modified attributes.
 *
 *  @param[in] attributes  Bitfield of modified attributes.
 */
void checkable::set_modified_attributes(int attributes) {
  _modified_attributes = attributes;
  return ;
}

/**
 *  Get percent state change.
 *
 *  @return Percent stage change.
 */
double checkable::get_percent_state_change() const {
  return (_percent_state_change);
}

/**
 *  Set percent state change.
 *
 *  @param[in] change  Percent state change.
 */
void checkable::set_percent_state_change(double change) {
  _percent_state_change = change;
  return ;
}

/**************************************
*                                     *
*            Event handler            *
*                                     *
**************************************/

/**
 *  Get event handler.
 *
 *  @return Event handler object. NULL if none.
 */
command* checkable::get_event_handler() const {
  return (_event_handler);
}

/**
 *  Set event handler.
 *
 *  @param[in] cmd  Event handler object.
 */
void checkable::set_event_handler(command* cmd) {
  _event_handler = cmd;
  return ;
}

/**
 *  Get event handler arguments.
 *
 *  @return Unparsed command arguments (command!ARG1!ARG2...).
 */
std::string const& checkable::get_event_handler_args() const {
  return (_event_handler_args);
}

/**
 *  Set event handler arguments.
 *
 *  @param[in] args  Event handler arguments.
 */
void checkable::set_event_handler_args(std::string const& args) {
  _event_handler_args = args;
  return ;
}

/**
 *  Check if event handler is enabled.
 *
 *  @return True if event handler is enabled.
 */
bool checkable::get_event_handler_enabled() const {
  return (_event_handler_enabled);
}

/**
 *  Enable or disable event handler.
 *
 *  @param[in] enable  True to enable event handler for this object.
 */
void checkable::set_event_handler_enabled(bool enable) {
  _event_handler_enabled = enable;
  return ;
}

/**************************************
*                                     *
*            Flap detection           *
*                                     *
**************************************/

/**
 *  Check if flap detection is enabled.
 *
 *  @return True if flap detection is enabled.
 */
bool checkable::get_flap_detection_enabled() const {
  return (_flap_detection_enabled);
}

/**
 *  Enable or disable flap detection.
 *
 *  @param[in] enable  True to enable flap detection.
 */
void checkable::set_flap_detection_enabled(bool enable) {
  _flap_detection_enabled = enable;
  return ;
}

/**
 *  Check if object is currently flapping.
 *
 *  @return True if object is currently flapping.
 */
bool checkable::get_flapping() const {
  return (_flapping);
}

/**
 *  Set whether object is flapping or not.
 *
 *  @param[in] flapping  True if object is flapping.
 */
void checkable::set_flapping(bool flapping) {
  _flapping = flapping;
  return ;
}

/**
 *  Get low flap threshold.
 *
 *  @return Low flap threshold.
 */
double checkable::get_low_flap_threshold() const {
  return (_low_flap_threshold);
}

/**
 *  Get high flap threshold.
 *
 *  @return High flap threshold.
 */
double checkable::get_high_flap_threshold() const {
  return (_high_flap_threshold);
}

/**
 *  Add a state to the history.
 *
 *  @param[in] state  New state to add to history.
 */
void checkable::add_historical_state(int state) {
  // XXX
}

/**
 *  Get an historical state.
 *
 *  @param[in] index  State index from 0 (oldest) to the max number of
 *                    historical states - 1 (newest).
 */
int checkable::get_historical_state(int index) {
  // XXX
}

/**************************************
*                                     *
*          Freshness checks           *
*                                     *
**************************************/

/**
 *  Check if object is being freshened.
 *
 *  @return True if object is being freshened.
 */
bool checkable::get_being_freshened() const {
  return (_being_freshened);
}

/**
 *  Set whether or not this object is being freshened.
 *
 *  @param[in] freshened  True if object is being freshened.
 */
void checkable::set_being_freshened(bool freshened) {
  _being_freshened = freshened;
  return ;
}

/**
 *  Check if freshness checks are enabled.
 *
 *  @return True if freshness checks are enabled.
 */
bool checkable::get_freshness_checks_enabled() const {
  return (_freshness_checks_enabled);
}

/**
 *  Enable or disable freshness checks.
 *
 *  @param[in] enable  True to enable freshness checks.
 */
void checkable::set_freshness_checks_enabled(bool enable) {
  _freshness_checks_enabled = enable;
  return ;
}

/**
 *  Get freshness threshold.
 *
 *  @return Freshness threshold.
 */
int checkable::get_freshness_threshold() const {
  return (_freshness_threshold);
}

/**************************************
*                                     *
*          Private methods            *
*                                     *
**************************************/

/**
 *  Copy internal data members.
 *
 *  @param[in] other  Object to copy.
 */
void checkable::_internal_copy(checkable const& other) {
  _active_checks_enabled = other._active_checks_enabled;
  _being_freshened = other._being_freshened;
  _check_command = other._check_command;
  _check_command_args = other._check_command_args;
  _check_options = other._check_options;
  _check_period = other._check_period;
  _check_type = other._check_type;
  _current_attempt = other._current_attempt;
  _current_event_id = other._current_event_id;
  _current_problem_id = other._current_problem_id;
  _current_state = other._current_state;
  _current_state_type = other._current_state_type;
  _event_handler = other._event_handler;
  _event_handler_args = other._event_handler_args;
  _event_handler_enabled = other._event_handler_enabled;
  _executing = other._executing;
  _execution_time = other._execution_time;
  _flap_detection_enabled = other._flap_detection_enabled;
  _flapping = other._flapping;
  _freshness_checks_enabled = other._freshness_checks_enabled;
  _freshness_threshold = other._freshness_threshold;
  _has_been_checked = other._has_been_checked;
  _host_problem_at_last_check = other._host_problem_at_last_check;
  _last_check = other._last_check;
  _last_event_id = other._last_event_id;
  _last_hard_state = other._last_hard_state;
  _last_hard_state_change = other._last_hard_state_change;
  _last_problem_id = other._last_problem_id;
  _last_state = other._last_state;
  _last_state_change = other._last_state_change;
  _latency = other._latency;
  _long_output = other._long_output;
  _max_attempts = other._max_attempts;
  _modified_attributes = other._modified_attributes;
  _next_check = other._next_check;
  _normal_check_interval = other._normal_check_interval;
  _ocp_enabled = other._ocp_enabled;
  _output = other._output;
  _passive_checks_enabled = other._passive_checks_enabled;
  _percent_state_change = other._percent_state_change;
  _perfdata = other._perfdata;
  _process_perfdata = other._process_perfdata;
  _retry_check_interval = other._retry_check_interval;
  _should_be_scheduled = other._should_be_scheduled;
  _timezone = other._timezone;
  return ;
}

bool checkable::is_in_downtime() const {
  return _in_downtime;
}
