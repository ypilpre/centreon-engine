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

#include "com/centreon/engine/host.hh"

using namespace com::centreon::engine;

/**
 *  Default constructor.
 */
host::host()
// XXX
{}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
host::host(host const& other) : monitorable(other) {
  _internal_copy(other);
}

/**
 *  Destructor.
 */
host::~host() {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
host& host::operator=(host const& other) {
  if (this != &other)
    _internal_copy(other);
  return (*this);
}

/**************************************
*                                     *
*           Configuration             *
*                                     *
**************************************/

/**
 *  Get children.
 *
 *  @return List of children.
 */
std::list<host*> const& host::get_children() const {
  return (_children);
}

/**
 *  Get initial state.
 *
 *  @return Initial state.
 */
int host::get_initial_state() const {
  return (_initial_state);
}

/**
 *  Get parents.
 *
 *  @return List of parents.
 */
std::list<host*> const& host::get_parents() const {
  return (_parents);
}

/**
 *  Check if host should be stalked on down states.
 *
 *  @return True if host should be stalked.
 */
bool host::get_stalk_on_down() const {
  return (_stalk_on_down);
}

/**
 *  Check if host should be stalked on unreachable states.
 *
 *  @return True if host should be stalked.
 */
bool host::get_stalk_on_unreachable() const {
  return (_stalk_on_unreachable);
}

/**
 *  Check if host should be stalked on up states.
 *
 *  @return True if host should be stalked.
 */
bool host::get_stalk_on_up() const {
  return (_stalk_on_up);
}

/**************************************
*                                     *
*           State runtime             *
*                                     *
**************************************/

/**
 *  Get last time host was down.
 *
 *  @return Last time host was down.
 */
time_t host::get_last_time_down() const {
  return (_last_time_down);
}

/**
 *  Set last time host was down.
 *
 *  @param[in] last_time  Last time host was down.
 */
void host::set_last_time_down(time_t last_time) {
  _last_time_down = last_time;
  return ;
}

/**
 *  Get last time host was unreachable.
 *
 *  @return Last time host was unreachable.
 */
time_t host::get_last_time_unreachable() const {
  return (_last_time_unreachable);
}

/**
 *  Set last time host was unreachable.
 *
 *  @param[in] last_time  Last time host was unreachable.
 */
void host::set_last_time_unreachable(time_t last_time) {
  _last_time_unreachable = last_time;
  return ;
}

/**
 *  Get last time host was up.
 *
 *  @return Last time host was up.
 */
time_t host::get_last_time_up() const {
  return (_last_time_up);
}

/**
 *  Set last time host was up.
 *
 *  @param[in] last_time  Last time host was up.
 */
void host::set_last_time_up(time_t last_time) {
  _last_time_up = last_time;
  return ;
}

/**
 *  Check if host's current check should be rescheduled.
 *
 *  @return True if check should be rescheduled.
 */
bool host::get_should_reschedule_current_check() const {
  return (_should_reschedule_current_check);
}

/**
 *  Set if host's current check should be rescheduled.
 *
 *  @param[in] reschedule  True if check should be rescheduled.
 */
void host::set_should_reschedule_current_check(bool reschedule) {
  _should_reschedule_current_check = reschedule;
  return ;
}

/**************************************
*                                     *
*            Notification             *
*                                     *
**************************************/

/**
 *  Check if host should notify on down states.
 *
 *  @return True if host should notify.
 */
bool host::get_notify_on_down() const {
  return (_notify_on_down);
}

/**
 *  Set whether or not host should notify on down states.
 *
 *  @param[in] notify  True to notify.
 */
void host::set_notify_on_down(bool notify) {
  _notify_on_down = notify;
  return ;
}

/**
 *  Check if host should notify on unreachable states.
 *
 *  @return True if host should notify.
 */
bool host::get_notify_on_unreachable() const {
  return (_notify_on_unreachable);
}

/**
 *  Set whether or not host should notify on unreachable states.
 *
 *  @param[in] notify  True to notify.
 */
void host::set_notify_on_unreachable(bool notify) {
  _notify_on_unreachable = notify;
  return ;
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
void host::_internal_copy(host const& other) {
  _children = other._children;
  _initial_state = other._initial_state;
  _last_time_down = other._last_time_down;
  _last_time_unreachable = other._last_time_unreachable;
  _last_time_up = other._last_time_up;
  _notify_on_down = other._notify_on_down;
  _notify_on_unreachable = other._notify_on_unreachable;
  _parents = other._parents;
  _should_reschedule_current_check = other._should_reschedule_current_check;
  _stalk_on_down = other._stalk_on_down;
  _stalk_on_unreachable = other._stalk_on_unreachable;
  _stalk_on_up = other._stalk_on_up;
  return ;
}
