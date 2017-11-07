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
 *  Get host name.
 *
 *  @return This object's host name.
 */
std::string const& monitorable::get_host_name() const {
  return (_host_name);
}

/**
 *  Copy internal data members.
 *
 *  @param[in] other  Object to copy.
 */
void monitorable::_internal_copy(monitorable const& other) {
  _host_name = other._host_name;
  return ;
}
