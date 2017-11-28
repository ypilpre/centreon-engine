/*
** Copyright 2017 Merethis
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

#include "com/centreon/engine/not_found.hh"

using namespace com::centreon::engine;

/**
 *  Default constructor.
 */
not_found::not_found() {}

/**
 *  Debug constructor.
 *
 *  @param[in] file      File where exception was constructed.
 *  @param[in] function  Function where exception was constructed.
 *  @param[in] line      Line number where exception was constructed.
 */
not_found::not_found(char const* file, char const* function, int line)
  : error(file, function, line) {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
not_found::not_found(not_found const& other) : error(other) {}

/**
 *  Destructor.
 */
not_found::~not_found() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
not_found& not_found::operator=(not_found const& other) {
  error::operator=(other);
  return (*this);
}
