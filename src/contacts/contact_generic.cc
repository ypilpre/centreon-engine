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

#include <list>
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/contacts/contact_generic.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::contacts;

/**************************************                                         
*                                     *                                         
*           Public Methods            *                                         
*                                     *                                         
**************************************/                                         

/**
 * Constructor.
 */
contact_generic::contact_generic() {}

/**
 * Copy constructor.
 *
 * @param[in] other Object to copy.
 */
contact_generic::contact_generic(contact_generic const& other) {}

/**
 * Assignment operator.
 *
 * @param[in] other Object to copy.
 *
 * @return This object
 */
contact_generic& contact_generic::operator=(contact_generic const& other) {
  return (*this);
}

/**
 * Destructor.
 */
contact_generic::~contact_generic() {
}
