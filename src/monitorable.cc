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

#include "com/centreon/engine/monitorable.hh"

using namespace com::centreon::engine;

/**
 *  Default constructor.
 */
monitorable::monitorable() {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
monitorable::monitorable(monitorable const& other)
  : notifications::notifier(other) {
  _internal_copy(other);
}

/**
 *  Destructor.
 */
monitorable::~monitorable() {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
monitorable& monitorable::operator=(monitorable const& other) {
  if (this != &other) {
    notifications::notifier::operator=(other);
    _internal_copy(other);
  }
  return (*this);
}

/**
 *  Get custom variables.
 *
 *  @return Set of this object's custom variables.
 */
customvar_set const& monitorable::get_customvars() const {
  return (_vars);
}

/**
 *  Set custom variable.
 *
 *  @param[in] var  New custom variable.
 */
void monitorable::set_customvar(customvar const& var) {
  _vars[var.get_name()] = var;
  return ;
}

/**
 *  Get host name.
 *
 *  @return This object's host name.
 */
std::string const& monitorable::get_host_name() const {
  return (_host_name);
}

/**
 *  Check if non-state retention is enabled for this object.
 *
 *  @return True if non-state retention is enabled.
 */
bool monitorable::get_retain_nonstate_info() const {
  return (_retain_nonstate_info);
}

/**
 *  Enable or disable non-state retention for this object.
 *
 *  @param[in] retain  True to retain non-state information.
 */
void monitorable::set_retain_nonstate_info(bool retain) {
  _retain_nonstate_info = retain;
  return ;
}

/**
 *  Check if state retention is enabled for this object.
 *
 *  @return True if state retention is enabled.
 */
bool monitorable::get_retain_state_info() const {
  return (_retain_state_info);
}

/**
 *  Enable or disable state retention for this object.
 *
 *  @param[in] retain  True to retain state information.
 */
void monitorable::set_retain_state_info(bool retain) {
  _retain_state_info = retain;
  return ;
}

/**
 *  Copy internal data members.
 *
 *  @param[in] other  Object to copy.
 */
void monitorable::_internal_copy(monitorable const& other) {
  _host_name = other._host_name;
  _retain_nonstate_info = other._retain_nonstate_info;
  _retain_state_info = other._retain_state_info;
  _vars = other._vars;
  return ;
}
