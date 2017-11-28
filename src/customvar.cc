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

#include "com/centreon/engine/customvar.hh"

using namespace com::centreon::engine;

/**
 *  Default constructor.
 */
customvar::customvar() : _modified(false) {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
customvar::customvar(customvar const& other) {
  _modified = other._modified;
  _name = other._name;
  _value = other._value;
}

/**
 *  Destructor.
 */
customvar::~customvar() {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
customvar& customvar::operator=(customvar const& other) {
  if (this != &other) {
    _modified = other._modified;
    _name = other._name;
    _value = other._value;
  }
  return (*this);
}

/**
 *  Get variable modification flag.
 *
 *  @return True if variable was modified.
 */
bool customvar::get_modified() const {
  return (_modified);
}

/**
 *  Get variable name.
 *
 *  @return Variable name.
 */
std::string const& customvar::get_name() const {
  return (_name);
}

/**
 *  Set variable name.
 *
 *  @param[in] name  Variable name.
 */
void customvar::set_name(std::string const& name) {
  _name = name;
  return ;
}

/**
 *  Get variable value.
 *
 *  @return Variable value.
 */
std::string const& customvar::get_value() const {
  return (_value);
}

/**
 *  Set variable modification flag.
 *
 *  @param[in] modified  True if variable was modified.
 */
void customvar::set_modified(bool modified) {
  _modified = modified;
  return ;
}

/**
 *  Set variable value.
 *
 *  @param[in] value  New variable value.
 */
void customvar::set_value(std::string const& value) {
  _value = value;
  return ;
}
