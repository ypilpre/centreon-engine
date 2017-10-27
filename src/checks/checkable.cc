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
  : _in_downtime(false), _is_flapping(false) {}

/**
 * Copy constructor.
 *
 * @param[in] other Object to copy.
 */
checkable::checkable(checkable const& other)
  : _in_downtime(other._in_downtime),
    _is_flapping(other._is_flapping),
    _current_state(other._current_state),
    _last_state(other._last_state),
    _last_hard_state(other._last_hard_state) {}

/**
 * Assignment operator.
 *
 * @param[in] other Object to copy.
 *
 * @return This object
 */
checkable& checkable::operator=(checkable const& other) {

  _in_downtime = other._in_downtime;
  _is_flapping = other._is_flapping;
  _current_state = other._current_state;
  _last_state = other._last_state;
  _last_hard_state = other._last_hard_state;
  return (*this);
}

/**
 * Destructor.
 */
checkable::~checkable() {
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

int checkable::get_current_state() const {
  // FIXME: must be implemented
  return _current_state;
}

int checkable::get_last_state() const {
  // FIXME: must be implemented
  return _last_state;
}

int checkable::get_last_hard_state() const {
  // FIXME: must be implemented
  return _last_hard_state;
}

void checkable::set_current_state(int state) {
  _last_state = _current_state;
  _current_state = state;
}

void checkable::set_last_state(int state) {
  _last_state = state;
}

void checkable::set_last_hard_state(int state) {
  _last_hard_state = state;
}

