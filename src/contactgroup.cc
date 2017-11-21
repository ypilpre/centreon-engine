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

#include <memory>
#include <sstream>
#include "com/centreon/engine/contactgroup.hh"
#include "com/centreon/engine/configuration/contactgroup.hh"

using namespace com::centreon;
using namespace com::centreon::engine;


/**************************************                                         
*                                     *                                         
*           Public Methods            *                                         
*                                     *                                         
**************************************/                                         

/**
 * Constructor.
 */
contactgroup::contactgroup() {}

/**
 * Copy constructor.
 *
 * @param[in] other Object to copy.
 */
contactgroup::contactgroup(contactgroup const& other) {}

/**
 * Assignment operator.
 *
 * @param[in] other Object to copy.
 *
 * @return This object
 */
contactgroup& contactgroup::operator=(contactgroup const& other) {
  return (*this);
}

/**
 * Destructor.
 */
contactgroup::~contactgroup() {
}

bool contactgroup::check(int* w, int* e) {
  (void)w;
  int errors(0);

  // Check all the group members.
  for (umap<std::string, shared_ptr<contact> >::const_iterator
         it(cg->get_members().begin()),
         end(cg->get_members().end());
         it != end;
         ++it) {
    contact* temp_contact(find_contact(it->first));
    if (!temp_contact) {
      logger(log_verification_error, basic)
        << "Error: Contact '" << temp_contactsmember->contact_name
        << "' specified in contact group '" << cg->get_name()
        << "' is not defined anywhere!";
      errors++;
    }

    // Save the contact pointer for later.
    it->second = temp_contact;
  }
//  for (contactsmember* temp_contactsmember(cg->members);
//       temp_contactsmember;
//       temp_contactsmember = temp_contactsmember->next) {
//    contact* temp_contact(
//               find_contact(temp_contactsmember->contact_name));
//    if (!temp_contact) {
//      logger(log_verification_error, basic)
//        << "Error: Contact '" << temp_contactsmember->contact_name
//        << "' specified in contact group '" << cg->group_name
//        << "' is not defined anywhere!";
//      errors++;
//    }
//
//    // Save a pointer to this contact group for faster contact/group
//    // membership lookups later.
//    else
//      add_object_to_objectlist(&temp_contact->contactgroups_ptr, cg);
//
//    // Save the contact pointer for later.
//    temp_contactsmember->contact_ptr = temp_contact;
//  }

  // Check for illegal characters in contact group name.
  if (contains_illegal_object_chars()) {
    logger(log_verification_error, basic)
      << "Error: The name of contact group '" << cg->group_name
      << "' contains one or more illegal characters.";
    errors++;
  }
//  if (contains_illegal_object_chars(cg->group_name) == true) {
//    logger(log_verification_error, basic)
//      << "Error: The name of contact group '" << cg->group_name
//      << "' contains one or more illegal characters.";
//    errors++;
//  }

  // Add errors.
  if (e)
    *e += errors;

  return (errors == 0);
}

std::string& contactgroup::get_name() const {
  return _name;
}
