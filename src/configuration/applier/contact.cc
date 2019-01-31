/*
** Copyright 2011-2015,2017-2018 Centreon
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
#include "com/centreon/engine/not_found.hh"
#include "com/centreon/engine/objects/objectlist.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

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

  // Create contact.
  shared_ptr< ::contact> c;
  try {
    c = new ::contact();
    // Base properties.
    c->set_addresses(obj.address());
    c->set_alias(obj.alias());
    c->set_can_submit_commands(obj.can_submit_commands());
    c->set_email(obj.email());
    c->set_name(obj.contact_name());
    c->set_pager(obj.pager());
    c->set_retain_status_information(obj.retain_status_information());
    c->set_retain_nonstatus_information(obj.retain_nonstatus_information());
    c->set_timezone(obj.timezone());
    // Host notification properties.
    c->set_host_notifications_enabled(obj.host_notifications_enabled());
    c->set_host_notify_on(
      ::notifications::notifiable::ON_RECOVERY,
      obj.host_notification_options() & configuration::host::up);
    c->set_host_notify_on(
      ::notifications::notifiable::ON_DOWN,
      obj.host_notification_options() & configuration::host::down);
    c->set_host_notify_on(
      ::notifications::notifiable::ON_UNREACHABLE,
      obj.host_notification_options() & configuration::host::unreachable);
    c->set_host_notify_on(
      ::notifications::notifiable::ON_FLAPPING,
      obj.host_notification_options() & configuration::host::flapping);
    c->set_host_notify_on(
      ::notifications::notifiable::ON_DOWNTIME,
      obj.host_notification_options() & configuration::host::downtime);
    // Service notification properties.
    c->set_service_notifications_enabled(obj.service_notifications_enabled());
    c->set_service_notify_on(
      ::notifications::notifiable::ON_RECOVERY,
      obj.service_notification_options() & configuration::service::ok);
    c->set_service_notify_on(
      ::notifications::notifiable::ON_WARNING,
      obj.service_notification_options() & configuration::service::warning);
    c->set_service_notify_on(
      ::notifications::notifiable::ON_CRITICAL,
      obj.service_notification_options() & configuration::service::critical);
    c->set_service_notify_on(
      ::notifications::notifiable::ON_UNKNOWN,
      obj.service_notification_options() & configuration::service::unknown);
    c->set_service_notify_on(
      ::notifications::notifiable::ON_FLAPPING,
      obj.service_notification_options() & configuration::service::flapping);
    c->set_service_notify_on(
      ::notifications::notifiable::ON_DOWNTIME,
      obj.service_notification_options() & configuration::service::downtime);
    // Custom variables.
    for (map_customvar::const_iterator
           it(obj.customvariables().begin()),
           end(obj.customvariables().end());
         it != end;
         ++it) {
      customvar var(it->first, it->second);
      c->set_customvar(var);
    }

    // Add contact to the global configuration set.
    config->contacts().insert(obj);
  }
  catch (std::exception const& e) {
    logger(logging::log_config_error, logging::basic)
      << "Error: " << e.what();
    throw (engine_error() << "Could not register contact '"
           << obj.contact_name() << "'");
  }

  // Add new items to the configuration state.
  applier::state::instance().contacts().insert(
    std::make_pair(obj.contact_name(), c));

  // Add all custom variables.
  for (map_customvar::const_iterator
         it(obj.customvariables().begin()),
         end(obj.customvariables().end());
       it != end;
       ++it) {
    customvar var(it->first, it->second);
    c->set_customvar(var);
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_contact_data(
    NEBTYPE_CONTACT_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    c.get(),
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);

  return ;
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
  umap<std::string, shared_ptr< ::contact> >::iterator
    it_obj(applier::state::instance().contacts().find(obj.key()));
  if (it_obj == applier::state::instance().contacts().end())
    throw (engine_error() << "Could not modify non-existing "
           << "contact object '" << obj.contact_name() << "'");
  engine::contact* c(it_obj->second.get());

  // Update the global configuration set.
  configuration::contact old_cfg(*it_cfg);
  config->contacts().erase(it_cfg);
  config->contacts().insert(obj);

  // Modify contact.
  modify_if_different(
    *c,
    alias,
    obj.alias().empty() ? obj.contact_name() : obj.alias());
  modify_if_different(*c, email, obj.email());
  modify_if_different(*c, pager, obj.pager());
  c->set_host_notify_on(
    ::notifications::notifiable::ON_RECOVERY,
    obj.host_notification_options() & configuration::host::up);
  c->set_host_notify_on(
    ::notifications::notifiable::ON_DOWN,
    obj.host_notification_options() & configuration::host::down);
  c->set_host_notify_on(
    ::notifications::notifiable::ON_UNREACHABLE,
    obj.host_notification_options() & configuration::host::unreachable);
  c->set_host_notify_on(
    ::notifications::notifiable::ON_FLAPPING,
    obj.host_notification_options() & configuration::host::flapping);
  c->set_host_notify_on(
    ::notifications::notifiable::ON_DOWNTIME,
    obj.host_notification_options() & configuration::host::downtime);
  c->set_service_notify_on(
    ::notifications::notifiable::ON_RECOVERY,
    obj.service_notification_options() & configuration::service::ok);
  c->set_service_notify_on(
    ::notifications::notifiable::ON_WARNING,
    obj.service_notification_options() & configuration::service::warning);
  c->set_service_notify_on(
    ::notifications::notifiable::ON_CRITICAL,
    obj.service_notification_options() & configuration::service::critical);
  c->set_service_notify_on(
    ::notifications::notifiable::ON_UNKNOWN,
    obj.service_notification_options() & configuration::service::unknown);
  c->set_service_notify_on(
    ::notifications::notifiable::ON_FLAPPING,
    obj.service_notification_options() & configuration::service::flapping);
  c->set_service_notify_on(
    ::notifications::notifiable::ON_DOWNTIME,
    obj.service_notification_options() & configuration::service::downtime);
  modify_if_different(
    *c,
    host_notifications_enabled,
    obj.host_notifications_enabled());
  modify_if_different(
    *c,
    service_notifications_enabled,
    obj.service_notifications_enabled());
  modify_if_different(
    *c,
    can_submit_commands,
    obj.can_submit_commands());
  modify_if_different(
    *c,
    retain_status_information,
    obj.retain_status_information());
  modify_if_different(
    *c,
    retain_nonstatus_information,
    obj.retain_nonstatus_information());
  modify_if_different(
    *c,
    timezone,
    obj.timezone());
  modify_if_different(*c, addresses, obj.address());

  // Custom variables.
  if (obj.customvariables() != old_cfg.customvariables()) {
    // Delete old custom variables.
    timeval tv(get_broker_timestamp(NULL));
    for (customvar_set::const_iterator
           it(c->get_customvars().begin()),
           end(c->get_customvars().end());
         it != end;
         ++it)
      broker_custom_variable(
        NEBTYPE_CONTACTCUSTOMVARIABLE_DELETE,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        c,
        it->second.get_name().c_str(),
        it->second.get_value().c_str(),
        &tv);
    c->clear_custom_variables();

    // Add custom variables.
    for (map_customvar::const_iterator
           it(obj.customvariables().begin()),
           end(obj.customvariables().end());
         it != end;
         ++it) {
      c->set_customvar(customvar(it->first, it->second));
      broker_custom_variable(
        NEBTYPE_CONTACTCUSTOMVARIABLE_ADD,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        c,
        it->first.c_str(),
        it->second.c_str(),
        &tv);
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

  return ;
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
    it(applier::state::instance().contacts().find(obj.key()));
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
  // Failure flag.
  bool failure(false);

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving contact '" << obj.contact_name() << "'.";

  try {
    // Find contact.
    engine::contact& cntct(
      *applier::state::instance().contacts_find(obj.key()));

    // Remove old links.
    cntct.clear_service_notification_commands();
    cntct.clear_host_notification_commands();
    cntct.set_service_notification_period(NULL);
    cntct.set_host_notification_period(NULL);
    cntct.clear_contactgroups();

    if (obj.service_notifications_enabled()) {
      // Resolve service notification commands.
      if (obj.service_notification_commands().empty()) {
        logger(logging::log_verification_error, logging::basic)
          << "Error: Contact '" << cntct.get_name()
          << "' has no service notification commands defined!";
        ++config_errors;
        failure = true;
      }
      else
        for (list_string::const_iterator
               it(obj.service_notification_commands().begin()),
               end(obj.service_notification_commands().end());
             it != end;
             ++it) {
          std::string buf(*it);
          size_t index(buf.find(buf, '!'));
          std::string command_name(buf.substr(0, index));
          shared_ptr<commands::command> cmd;
          try {
            cmd = find_command(command_name);
          }
          catch (not_found const& e) {
            (void)e;
            logger(logging::log_verification_error, logging::basic)
              << "Error: Service notification command '"
              << command_name << "' specified for contact '"
              << cntct.get_name() << "' is not defined anywhere!";
            ++config_errors;
            failure = true;
          }
          cntct.add_service_notification_command(cmd.get(), *it);
        }

      // Resolve service notification timeperiod.
      cntct.set_service_notification_period(NULL);
      if (obj.service_notification_period().empty()) {
        logger(logging::log_verification_error, logging::basic)
          << "Warning: Contact '" << cntct.get_name()
          << "' has no service notification time period defined!";
        ++config_warnings;
      }
      else {
        try {
          cntct.set_service_notification_period(
                  &find_timeperiod(obj.service_notification_period()));
        }
        catch (not_found const& e) {
          (void)e;
          logger(logging::log_verification_error, logging::basic)
            << "Error: Service notification period '"
            << obj.service_notification_period()
            << "' specified for contact '" << cntct.get_name()
            << "' is not defined anywhere!";
          ++config_errors;
          failure = true;
        }
      }
    }

    if (obj.host_notifications_enabled()) {
      // Resolve host notification commands.
      if (obj.host_notification_commands().empty()) {
        logger(logging::log_verification_error, logging::basic)
          << "Error: Contact '" << cntct.get_name() << "' has no host "
          "notification commands defined!";
        ++config_errors;
        failure = true;
      }
      else
        for (list_string::const_iterator
               it(obj.host_notification_commands().begin()),
               end(obj.host_notification_commands().end());
             it != end;
             ++it) {
          std::string buf(*it);
          size_t index(buf.find('!'));
          std::string command_name(buf.substr(0, index));
          shared_ptr<commands::command> cmd;
          try {
            cmd = find_command(command_name);
          }
          catch (not_found const& e) {
            (void)e;
            logger(logging::log_verification_error, logging::basic)
              << "Error: Host notification command '" << command_name
              << "' specified for contact '" << cntct.get_name()
              << "' is not defined anywhere!";
            ++config_errors;
            failure = true;
          }
          cntct.add_host_notification_command(cmd.get(), *it);
        }

      // Resolve host notification timeperiod.
      if (obj.host_notification_period().empty()) {
        logger(logging::log_verification_error, logging::basic)
          << "Warning: Contact '" << cntct.get_name()
          << "' has no host notification time period defined!";
        ++config_warnings;
      }
      else {
        try {
          cntct.set_host_notification_period(
                  &find_timeperiod(obj.host_notification_period()));
        }
        catch (not_found const& e) {
          (void)e;
          logger(logging::log_verification_error, logging::basic)
            << "Error: Host notification period '"
            << obj.host_notification_period()
            << "' specified for contact '" << cntct.get_name()
            << "' is not defined anywhere!";
          ++config_errors;
          failure = true;
        }
      }
    }

    // Check for sane host recovery options.
    if (cntct.get_host_notify_on(notifications::notifiable::ON_RECOVERY)
        && !cntct.get_host_notify_on(notifications::notifiable::ON_DOWN)
        && !cntct.get_host_notify_on(notifications::notifiable::ON_UNREACHABLE)) {
      logger(logging::log_verification_error, logging::basic)
        << "Warning: Host recovery notification option for contact '"
        << cntct.get_name() << "' doesn't make any sense - specify down"
           " and/or unreachable options as well";
      ++config_warnings;
    }

    // Check for sane service recovery options.
    if (cntct.get_service_notify_on(notifications::notifiable::ON_RECOVERY)
        && !cntct.get_service_notify_on(notifications::notifiable::ON_CRITICAL)
        && !cntct.get_service_notify_on(notifications::notifiable::ON_WARNING)) {
      logger(logging::log_verification_error, logging::basic)
        << "Warning: Service recovery notification option for contact '"
        << cntct.get_name() << "' doesn't make any sense - "
           "specify critical and/or warning options as well";
      ++config_warnings;
    }

    // Check for illegal characters in contact name.
    if (contains_illegal_object_chars(cntct.get_name().c_str())) {
      logger(logging::log_verification_error, logging::basic)
        << "Error: The name of contact '" << cntct.get_name()
        << "' contains one or more illegal characters.";
      ++config_errors;
      failure = true;
    }

    if (failure)
      throw (error() << "please check logs above");
  }
  catch (std::exception const& e) {
    throw (engine_error() << "Could not resolve contact '"
           << obj.contact_name() << "': " << e.what());
  }
}

/**
 *  Do nothing.
 */
void applier::contact::unresolve_objects() {
  return ;
}
