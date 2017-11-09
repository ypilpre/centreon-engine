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

#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/contacts/contact_group.hh"

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
contact_group::contact_group() {}

/**
 * Copy constructor.
 *
 * @param[in] other Object to copy.
 */
contact_group::contact_group(contact_group const& other) {}

/**
 * Assignment operator.
 *
 * @param[in] other Object to copy.
 *
 * @return This object
 */
contact_group& contact_group::operator=(contact_group const& other) {
  return (*this);
}

/**
 * Destructor.
 */
contact_group::~contact_group() {
}

void contact_group::fill_contact_users(
                      std::list<shared_ptr<contact_user> >& lst) {

  for (
    std::list<shared_ptr<contact_generic> >::iterator it(_contacts.begin()),
                                                      end(_contacts.end());
    it != end;
    ++it) {
    (*it)->fill_contact_users(lst);
  }
}
