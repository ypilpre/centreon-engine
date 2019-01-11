/*
** Copyright 2011-2013,2015-2016 Centreon
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

#include "com/centreon/engine/objects/customvariable.hh"

using namespace com::centreon::engine;

customvariable::customvariable(std::string const& key, std::string const& value)
 : _key(key),
   _value(value),
   _is_sent(false) {}

customvariable::customvariable(customvariable const& other)
 : _key(other._key),
   _value(other._value),
   _is_sent(other._is_sent) {}

customvariable& customvariable::operator=(customvariable const& other) {
  if (this != &other) {
    _key = other._key;
    _value = other._value;
    _is_sent = other._is_sent;
  }
  return *this;
}

customvariable::~customvariable() {}

void customvariable::set_sent(bool sent) {
  _is_sent = sent;
}

bool customvariable::is_sent() const {
  return _is_sent;
}

std::string const& customvariable::get_value() const {
  return _value;
}

void customvariable::set_value(std::string const& value) {
  _value = value;
}

bool customvariable::operator==(customvariable const& other) const {
  return _key == other._key && _value == other._value && _is_sent == other._is_sent;
}

bool customvariable::operator<(customvariable const& other) const {
  return _key < other._key || _value < other._value || _is_sent < other._is_sent;
}

bool customvariable::operator!=(customvariable const& other) {
  return _key != other._key || _value != other._value || _is_sent != other._is_sent;
}
