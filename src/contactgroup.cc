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
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contactgroup.hh"
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/contactgroup.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "find.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;


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
 *  Constructor from a configuration contactgroup
 *
 * @param obj Configuration contactgroup
 */
contactgroup::contactgroup(configuration::contactgroup const& obj)
  : _name(obj.contactgroup_name()),
    _alias((obj.alias().empty()) ? obj.contactgroup_name() : obj.alias()) {

  // Make sure we have the data we need.
  if (_name.empty())
    throw (engine_error() << "contactgroup: Contact group name is empty");

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_group(
    NEBTYPE_CONTACTGROUP_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this,
    &tv);
}

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
contactgroup::~contactgroup() {}

bool contactgroup::check(int* w, int* e) {
  (void)w;
  int errors(0);

  // Check all the group members.
  for (umap<std::string, shared_ptr<contact> >::iterator
         it(get_members().begin()),
         end(get_members().end());
         it != end;
         ++it) {
    shared_ptr<contact> ctct;
    umap<std::string, shared_ptr<contact> >::const_iterator
      itf(configuration::applier::state::instance().contacts().find(it->first));
    if (itf != configuration::applier::state::instance().contacts().end())
      ctct = itf->second;
    if (!ctct.get()) {
      logger(log_verification_error, basic)
        << "Error: Contact '" << it->first
        << "' specified in contact group '" << get_name()
        << "' is not defined anywhere!";
      errors++;
    }

    // Save the contact pointer for later.
    it->second = ctct;
  }

  // Check for illegal characters in contact group name.
  if (contains_illegal_object_chars()) {
    logger(log_verification_error, basic)
      << "Error: The name of contact group '" << get_name()
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

std::string const& contactgroup::get_name() const {
  return _name;
}

umap<std::string, shared_ptr<contact> > const& contactgroup::get_members() const {
  return _members;
}

umap<std::string, shared_ptr<contact> >& contactgroup::get_members() {
  return _members;
}

bool contactgroup::contains_illegal_object_chars() const {
  if (_name.empty() || !illegal_object_chars)
    return false;
  return (_name.find(illegal_object_chars) != std::string::npos);
}

std::string const& contactgroup::get_alias() const {
  return _alias;
}

void contactgroup::add_contact(std::string const& contact_name) {
  // Make sure we have the data we need.
  if (contact_name.empty())
    throw (engine_error() << "Error: Contact name is empty");

  _members[contact_name] = shared_ptr<contact>(0);

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_group_member(
    NEBTYPE_CONTACTGROUPMEMBER_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    NULL, // FIXME DBR this should be the contactmember almost empty we don't
          // build anymore.
    this,
    &tv);
}

void contactgroup::update_config(configuration::contactgroup const& obj) {
  configuration::applier::modify_if_different(
    _alias, obj.alias().empty() ? obj.contactgroup_name() : obj.alias());
}

bool contactgroup::contains_member(std::string const& name) const {
  umap<std::string, shared_ptr<contact> >::const_iterator it(
    get_members().find(name));
  return (it != get_members().end());
}
