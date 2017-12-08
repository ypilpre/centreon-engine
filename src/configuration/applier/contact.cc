/*
** Copyright 2011-2015,2017 Centreon
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

#include <algorithm>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/objectlist.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

/**
 *  Check if the contact group name matches the configuration object.
 */
class         contactgroup_name_comparator {
public:
              contactgroup_name_comparator(
                std::string const& contactgroup_name) {
    _contactgroup_name = contactgroup_name;
  }

  bool        operator()(shared_ptr<configuration::contactgroup> cg) {
    return (_contactgroup_name == cg->contactgroup_name());
 }

private:
  std::string _contactgroup_name;
};

/**
 *  Default constructor.
 */
applier::contact::contact() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::contact::contact(applier::contact const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::contact::~contact() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::contact& applier::contact::operator=(
                                      applier::contact const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new contact.
 *
 *  @param[in] obj  The new contact to add into the monitoring engine.
 */
void applier::contact::add_object(configuration::contact const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new contact '" << obj.contact_name() << "'.";

  // Add contact to the global configuration set.
  config->contacts().insert(obj);

  // Create contact.
  engine::contact*
    c(engine::contact::add_contact(
        obj.contact_name(),
        obj.alias(),
        obj.email(),
        obj.pager(),
        obj.address(),
        obj.service_notification_period(),
        obj.host_notification_period(),
        static_cast<bool>(
          obj.service_notification_options() & service::ok),
        static_cast<bool>(
          obj.service_notification_options() & service::critical),
        static_cast<bool>(
          obj.service_notification_options() & service::warning),
        static_cast<bool>(
          obj.service_notification_options() & service::unknown),
        static_cast<bool>(
          obj.service_notification_options() & service::flapping),
        static_cast<bool>(
          obj.service_notification_options() & service::downtime),
        static_cast<bool>(
          obj.host_notification_options() & host::up),
        static_cast<bool>(
          obj.host_notification_options() & host::down),
        static_cast<bool>(
          obj.host_notification_options() & host::unreachable),
        static_cast<bool>(
          obj.host_notification_options() & host::flapping),
        static_cast<bool>(
          obj.host_notification_options() & host::downtime),
        obj.host_notifications_enabled(),
        obj.service_notifications_enabled(),
        obj.can_submit_commands(),
        obj.retain_status_information(),
        obj.retain_nonstatus_information(),
        obj.timezone()));
  if (!c)
    throw (engine_error() << "Could not register contact '"
           << obj.contact_name() << "'");

  // Add all the host notification commands.
  for (list_string::const_iterator
         it(obj.host_notification_commands().begin()),
         end(obj.host_notification_commands().end());
       it != end;
       ++it)
    try {
      c->add_host_notification_command(*it);
    }
    catch (std::exception const& e) {
      throw (engine_error()
        << "contact: Could not add host notification command '"
        << *it << "' to contact '" << obj.contact_name() << "' :"
        << e.what());
    }

  // Add all the service notification commands.
  for (list_string::const_iterator
	 it(obj.service_notification_commands().begin()),
	 end(obj.service_notification_commands().end());
       it != end;
       ++it)
    try {
      c->add_service_notification_command(*it);
    }
    catch (std::exception const& e) {
      throw (engine_error()
	     << "contact: Could not add service notification command '"
	     << *it << "' to contact '" << obj.contact_name() << "' :"
             << e.what());
    }

  // Add all custom variables.
  for (map_customvar::const_iterator
           it(obj.customvariables().begin()),
           end(obj.customvariables().end());
       it != end;
       ++it)
    try {
      c->set_customvar(customvar(it->first, it->second));
    }
    catch (std::exception const& e) {
      throw (engine_error()
	     << "Could not add custom variable '" << it->first
	     << "' to contact '" << obj.contact_name() << "' :"
             << e.what());
    }
}

/**
 *  @brief Expand a contact.
 *
 *  During expansion, the contact will be added to its contact groups.
 *  These will be modified in the state.
 *
 *  @param[in,out] s  Configuration state.
 */
void applier::contact::expand_objects(configuration::state& s) {
  // Browse all contacts.
  for (configuration::set_contact::iterator
         it_contact(s.contacts().begin()),
         end_contact(s.contacts().end());
       it_contact != end_contact;
       ++it_contact)
    // Browse current contact's groups.
    for (set_string::const_iterator
           it_group(it_contact->contactgroups().begin()),
           end_group(it_contact->contactgroups().end());
         it_group != end_group;
         ++it_group) {
      // Find contact group.
      configuration::set_contactgroup::iterator
        group(s.contactgroups_find(*it_group));
      if (group == s.contactgroups().end())
        throw (engine_error() << "Could not add contact '"
               << it_contact->contact_name()
               << "' to non-existing contact group '" << *it_group
               << "'");

      // Remove contact group from state.
      configuration::contactgroup backup(*group);
      s.contactgroups().erase(group);

      // Add contact to group members.
      backup.members().insert(it_contact->contact_name());

      // Reinsert contact group.
      s.contactgroups().insert(backup);
    }

  return ;
}

/**
 *  Modified contact.
 *
 *  @param[in] obj  The new contact to modify into the monitoring engine.
 */
void applier::contact::modify_object(
                         configuration::contact const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying contact '" << obj.contact_name() << "'.";

  // Find old configuration.
  set_contact::iterator it_cfg(config->contacts_find(obj.key()));
  if (it_cfg == config->contacts().end())
    throw (engine_error() << "Cannot modify non-existing contact '"
           << obj.contact_name() << "'");

  // Find contact object.
  umap<std::string, shared_ptr<engine::contact> >::iterator
    it_obj(applier::state::instance().contacts_find(obj.key()));
  if (it_obj == applier::state::instance().contacts().end())
    throw (engine_error() << "Could not modify non-existing "
           << "contact object '" << obj.contact_name() << "'");
  engine::contact* c(it_obj->second.get());

  // Update the global configuration set.
  configuration::contact old_cfg(*it_cfg);
  config->contacts().erase(it_cfg);
  config->contacts().insert(obj);

  // Modify contact.
  c->update_config(obj);

  // Host notification commands.
  if (obj.host_notification_commands()
      != old_cfg.host_notification_commands()) {
    c->clear_host_notification_commands();

    for (list_string::const_iterator
           it(obj.host_notification_commands().begin()),
           end(obj.host_notification_commands().end());
         it != end;
         ++it)
      try {
        c->add_host_notification_command(*it);
      }
      catch (std::exception const& e) {
        throw (engine_error()
               << "Could not add host notification command '"
               << *it << "' to contact '" << obj.contact_name()
               << "' :" << e.what());
      }
  }

  // Service notification commands.
  if (obj.service_notification_commands()
      != old_cfg.service_notification_commands()) {
    c->clear_service_notification_commands();

    for (list_string::const_iterator
           it(obj.service_notification_commands().begin()),
           end(obj.service_notification_commands().end());
         it != end;
         ++it)
      try {
        c->add_service_notification_command(*it);
      }
      catch (std::exception const& e) {
        throw (engine_error()
               << "Could not add service notification command '"
               << *it << "' to contact '" << obj.contact_name()
               << "' :" << e.what());
      }
  }

  // Custom variables.
  if (std::operator!=(obj.customvariables(), old_cfg.customvariables())) {
    c->clear_custom_variables();

    for (map_customvar::const_iterator
           it(obj.customvariables().begin()),
           end(obj.customvariables().end());
         it != end;
         ++it)
      try {
        c->set_customvar(customvar(it->first, it->second));
      }
      catch (std::exception const& e) {
        throw (engine_error()
               << "Could not add custom variable '" << it->first
               << "' to contact '" << obj.contact_name() << "' :"
               << e.what());
      }
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_contact_data(
    NEBTYPE_CONTACT_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    c,
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);
}

/**
 *  Remove old contact.
 *
 *  @param[in] obj  The new contact to remove from the monitoring engine.
 */
void applier::contact::remove_object(
                         configuration::contact const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing contact '" << obj.contact_name() << "'.";

  // Find contact.
  umap<std::string, shared_ptr<engine::contact> >::iterator
    it(applier::state::instance().contacts_find(obj.key()));
  if (it != applier::state::instance().contacts().end()) {
    engine::contact* cntct(it->second.get());

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_contact_data(
      NEBTYPE_CONTACT_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      cntct,
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);

    // Erase contact object (this will effectively delete the object).
    applier::state::instance().contacts().erase(it);
  }

  // Remove contact from the global configuration set.
  config->contacts().erase(obj);

  return ;
}

/**
 *  Resolve a contact.
 *
 *  @param[in,out] obj  Object to resolve.
 */
void applier::contact::resolve_object(
                         configuration::contact const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving contact '" << obj.contact_name() << "'.";

  // Find contact.
  contact_map::iterator
    it(applier::state::instance().contacts_find(obj.contact_name()));
  if (it == applier::state::instance().contacts().end())
    throw (engine_error()
           << "Cannot resolve non-existing contact '"
           << obj.contact_name() << "'");

  // Remove contact group links.
  it->second->get_contactgroups().clear();

  // Resolve contact.
  if (!it->second->check(&config_warnings, &config_errors))
    throw (engine_error() << "Cannot resolve contact '"
        << obj.contact_name() << "'");
}

/**
 *  Do nothing.
 */
void applier::contact::unresolve_objects() {
  return ;
}
