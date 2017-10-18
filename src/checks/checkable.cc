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

using namespace com::centreon::engine;
using namespace com::centreon::engine::checks;

/**************************************                                         
*                                     *                                         
*           Public Methods            *                                         
*                                     *                                         
**************************************/                                         

/**
 * Constructor.
 */
checkable::checkable()
  : _in_downtime(false), _is_flapping(false) {
}

/**
 * Copy constructor.
 *
 * @param[in] other Object to copy.
 */
checkable::checkable(checkable const& other) {
}

/**
 * Destructor.
 */
checkable::~checkable() {
}

/**
 * Assignment operator.
 *
 * @param[in] other Object to copy.
 *
 * @return This object
 */
checkable& checkable::operator=(checkable const& other) {

  return (*this);
}

/**
 * This method tells if this checkable is in downtime.
 *
 * @return a boolean
 */
bool checkable::in_downtime() const {
  // FIXME: must be implemented
  return _in_downtime;
}

/**
 * This method tells if this checkable is flapping.
 *
 * @return a boolean
 */
bool checkable::is_flapping() const {
  // FIXME: must be implemented
  return _is_flapping;
}

int checkable::get_state() const {
  // FIXME: must be implemented
  return _state;
}

int checkable::get_last_state() const {
  // FIXME: must be implemented
  return 0;
}

int checkable::get_last_hard_state() const {
  // FIXME: must be implemented
  return 0;
}
