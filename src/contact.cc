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
#include "com/centreon/engine/commands/command.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/deleter/customvariablesmember.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/engine/objects/customvariablesmember.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "find.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::notifications;


/**************************************                                         
*                                     *                                         
*           Public Methods            *                                         
*                                     *                                         
**************************************/                                         

/**
 *  Wrapper to the contact constructor
 *
 *  @param[in] name contact name
 *  @param[in] alias contact alias
 *  @param[in] email contact email
 *  @param[in] pager contact pager
 *  @param[in] addresses contact addresses as strings array
 *  @param[in] svc_notification_period contact service notification period name
 *  @param[in] host_notification_period contact host notification period name
 *  @param[in] notify_service_ok should this contact notified for service ok
 *  @param[in] notify_service_critical should this contact notified for service critical
 *  @param[in] notify_service_warning should this contact notified for service warning
 *  @param[in] notify_service_unknown should this contact notified for service unknown
 *  @param[in] notify_service_flapping should this contact notified for service flapping
 *  @param[in] notify_service_downtime should this contact notified for service downtime
 *  @param[in] notify_host_up should this contact notified for host up
 *  @param[in] notify_host_down should this contact notified for host down
 *  @param[in] notify_host_unreachable should this contact notified for host unreachable
 *  @param[in] notify_host_flapping should this contact notified for host flapping
 *  @param[in] notify_host_downtime should this contact notified for host downtime
 *  @param[in] host_notifications_enabled should this contact notified for hosts
 *  @param[in] service_notifications_enabled should this contact notified for services
 *  @param[in] can_submit_commands
 *  @param[in] retain_status_information
 *  @param[in] retain_nonstatus_information
 *  @param[in] timezone
 *
 *  Creates a new contact. If the given name is empty, NULL is returned.
 *  If a contact with the same name already exists, NULL is returned.
 *
 *  @return A pointer to the newly created contact.
 */
contact* contact::add_contact(
                char const* name,
                char const* alias,
                char const* email,
                char const* pager,
                char const* const* addresses,
                char const* svc_notification_period,
                char const* host_notification_period,
                int notify_service_ok,
                int notify_service_critical,
                int notify_service_warning,
                int notify_service_unknown,
                int notify_service_flapping,
                int notify_service_downtime,
                int notify_host_up,
                int notify_host_down,
                int notify_host_unreachable,
                int notify_host_flapping,
                int notify_host_downtime,
                int host_notifications_enabled,
                int service_notifications_enabled,
                int can_submit_commands,
                int retain_status_information,
                int retain_nonstatus_information,
                std::string const& timezone) {
  // Make sure we have the data we need.
  if (!name || !name[0]) {
    logger(log_config_error, basic)
      << "Error: Contact name is NULL";
    return (NULL);
  }

  // Check if the contact already exists.
  umap<std::string, shared_ptr<contact> >::const_iterator
    it(configuration::applier::state::instance().contacts().find(name));
  if (it == configuration::applier::state::instance().contacts().end()) {
    logger(log_config_error, basic)
      << "Error: Contact '" << name << "' has already been defined";
    return (NULL);
  }

  // Allocate memory for a new contact.
  shared_ptr<contact> obj(new contact(
                            name,
                            alias,
                            email,
                            pager,
                            addresses,
                            svc_notification_period,
                            host_notification_period,
                            notify_service_ok,
                            notify_service_critical,
                            notify_service_warning,
                            notify_service_unknown,
                            notify_service_flapping,
                            notify_service_downtime,
                            notify_host_up,
                            notify_host_down,
                            notify_host_unreachable,
                            notify_host_flapping,
                            notify_host_downtime,
                            host_notifications_enabled,
                            service_notifications_enabled,
                            can_submit_commands,
                            retain_status_information,
                            retain_nonstatus_information,
                            timezone));

  try {
    // Add new items to the configuration state.
    configuration::applier::state::instance().contacts()[name] = obj;

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_contact_data(
      NEBTYPE_CONTACT_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Constructor.
 */
contact::contact() {}

/**
 * Constructor.
 */
contact::contact(
           char const* name,
           char const* alias,
           char const* email,
           char const* pager,
           char const* const* addresses,
           char const* svc_notification_period,
           char const* host_notification_period,
           int notify_service_ok,
           int notify_service_critical,
           int notify_service_warning,
           int notify_service_unknown,
           int notify_service_flapping,
           int notify_service_downtime,
           int notify_host_up,
           int notify_host_down,
           int notify_host_unreachable,
           int notify_host_flapping,
           int notify_host_downtime,
           int host_notifications_enabled,
           int service_notifications_enabled,
           int can_submit_commands,
           int retain_status_information,
           int retain_nonstatus_information,
           std::string const& timezone)
  : _name(name), _email(email),
    _host_notification_period_name(host_notification_period),
    _host_notification_period(0),
    _service_notification_period(0),
    _service_notification_period_name(svc_notification_period),
    _pager(pager),
    _can_submit_commands(can_submit_commands > 0),
    _host_notifications_enabled(host_notifications_enabled > 0),
    _modified_attributes(MODATTR_NONE),
    _modified_host_attributes(MODATTR_NONE),
    _modified_service_attributes(MODATTR_NONE),
    _notify_on_host_down(notify_host_down > 0),
    _notify_on_host_downtime(notify_host_downtime > 0),
    _notify_on_host_flapping(notify_host_flapping > 0),
    _notify_on_host_recovery(notify_host_up > 0),
    _notify_on_host_unreachable(notify_host_unreachable > 0),
    _notify_on_service_critical(notify_service_critical > 0),
    _notify_on_service_downtime(notify_service_downtime > 0),
    _notify_on_service_flapping(notify_service_flapping > 0),
    _notify_on_service_recovery(notify_service_ok > 0),
    _notify_on_service_unknown(notify_service_unknown > 0),
    _notify_on_service_warning(notify_service_warning > 0),
    _retain_nonstatus_information(retain_nonstatus_information > 0),
    _retain_status_information(retain_status_information > 0),
    _service_notifications_enabled(service_notifications_enabled > 0),
    _timezone(timezone) {

  if (!alias || !alias[0])
    _alias = name;
  else
    _alias = alias;

  for (unsigned int x(0); x < MAX_CONTACT_ADDRESSES; ++x)
    _address.push_back(addresses[x]);

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_contact_data(
      NEBTYPE_CONTACT_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      this,
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
 * Copy constructor.
 *
 * @param[in] other Object to copy.
 */
contact::contact(contact const& other) {}

/**
 * Assignment operator.
 *
 * @param[in] other Object to copy.
 *
 * @return This object
 */
contact& contact::operator=(contact const& other) {
  return (*this);
}

/**
 * Destructor.
 */
contact::~contact() {
}

/**
 *  Return the contact name
 *
 *  @return a reference to the name
 */
std::string const& contact::get_name() const {
  return _name;
}

/**
 *  Return the contact alias
 *
 *  @return a reference to the alias
 */
std::string const& contact::get_alias() const {
  return _alias;
}

/**
 *  Return the contact email
 *
 *  @return a reference to the email
 */
std::string const& contact::get_email() const {
  return _email;
}

/**
 *  Return the contact pager
 *
 *  @return a reference to the pager
 */
std::string const& contact::get_pager() const {
  return _pager;
}

/**
 *  Checks contact
 *
 * @param[out] w Number of warnings returned by the check
 * @param[out] e Number of errors returned by the check
 *
 * @return true if no errors are returned.
 */
bool contact::check(int* w, int* e) {
  int warnings(0);
  int errors(0);

  /* check service notification commands */
  if (get_service_notification_commands().empty()) {
    logger(log_verification_error, basic)
      << "Error: Contact '" << get_name() << "' has no service "
      "notification commands defined!";
    errors++;
  }
  else
    for (umap<std::string, shared_ptr<command_struct> >::iterator
           it(get_service_notification_commands().begin()),
           end(get_service_notification_commands().end());
           it != end;
           ++it) {
      std::string buf(it->first);
      size_t index(buf.find(buf, '!'));
      std::string command_name(buf.substr(0, index));
      command_struct* temp_command = find_command(command_name.c_str());

      if (temp_command == NULL) {
        logger(log_verification_error, basic)
          << "Error: Service notification command '"
          << command_name << "' specified for contact '"
          << get_name() << "' is not defined anywhere!";
	errors++;
      }

      /* save pointer to the command for later */
      it->second = temp_command;
    }

  /* check host notification commands */
  if (get_host_notification_commands().empty()) {
    logger(log_verification_error, basic)
      << "Error: Contact '" << get_name() << "' has no host "
      "notification commands defined!";
    errors++;
  }
  else
    for (umap<std::string, shared_ptr<command> >::iterator
           it(get_host_notification_commands().begin()),
           end(get_host_notification_commands().end());
           it != end;
           ++it) {
      std::string buf(it->first);
      size_t index(buf.find('!'));
      std::string command_name(buf.substr(0, index));
      command_struct* cmd = find_command(command_name.c_str());

      if (cmd == NULL) {
        logger(log_verification_error, basic)
          << "Error: Host notification command '" << command_name
          << "' specified for contact '" << get_name()
          << "' is not defined anywhere!";
	errors++;
      }

      /* save pointer to the command for later */
      it->second = cmd;
    }

  /* check service notification timeperiod */
  if (get_service_notification_period() == NULL) {
    logger(log_verification_error, basic)
      << "Warning: Contact '" << get_name() << "' has no service "
      "notification time period defined!";
    warnings++;
  }
  else {
    timeperiod* temp_timeperiod(
      find_timeperiod(get_service_notification_period_name().c_str()));
    if (temp_timeperiod == NULL) {
      logger(log_verification_error, basic)
        << "Error: Service notification period '"
        << get_service_notification_period_name()
        << "' specified for contact '" << get_name()
        << "' is not defined anywhere!";
      errors++;
    }
    set_service_notification_period(temp_timeperiod);
  }

  /* check host notification timeperiod */
  if (get_host_notification_period() == NULL) {
    logger(log_verification_error, basic)
      << "Warning: Contact '" << get_name() << "' has no host "
      "notification time period defined!";
    warnings++;
  }
  else {
    timeperiod* temp_timeperiod
      = find_timeperiod(get_host_notification_period_name().c_str());
    if (temp_timeperiod == NULL) {
      logger(log_verification_error, basic)
        << "Error: Host notification period '"
        << get_host_notification_period_name()
        << "' specified for contact '" << get_name()
        << "' is not defined anywhere!";
      errors++;
    }

    /* save the pointer to the host notification timeperiod for later */
    set_host_notification_period(temp_timeperiod);
  }

  /* check for sane host recovery options */
  if (notify_on_host_recovery()
      && !notify_on_host_down()
      && !notify_on_host_unreachable()) {
    logger(log_verification_error, basic)
      << "Warning: Host recovery notification option for contact '"
      << get_name() << "' doesn't make any sense - specify down "
      "and/or unreachable options as well";
    warnings++;
  }

  /* check for sane service recovery options */
  if (notify_on_service_recovery()
      && !notify_on_service_critical()
      && !notify_on_service_warning()) {
    logger(log_verification_error, basic)
      << "Warning: Service recovery notification option for contact '"
      << get_name() << "' doesn't make any sense - specify critical "
      "and/or warning options as well";
    warnings++;
  }

  /* check for illegal characters in contact name */
  if (contains_illegal_object_chars()) {
    logger(log_verification_error, basic)
      << "Error: The name of contact '" << get_name()
      << "' contains one or more illegal characters.";
    errors++;
  }

  if (w != NULL)
    *w += warnings;
  if (e != NULL)
    *e += errors;
  return (errors == 0);
}

/**
 *  host_notification_commands getter
 *
 *  @return an unordered map indexed by names of the host notification commands
 */
umap<std::string, shared_ptr<command> >& contact::get_host_notification_commands() {
  return _host_notification_commands;
}

/**
 *  host_notification_commands getter
 *
 *  @return an unordered map indexed by names of the host notification commands
 */
umap<std::string, shared_ptr<command> > const& contact::get_host_notification_commands() const {
  return _host_notification_commands;
}

/**
 *  service_notification_commands getter
 *
 *  @return an unordered map indexed by names of the service notification commands
 */
umap<std::string, shared_ptr<command> >& contact::get_service_notification_commands() {
  return _service_notification_commands;
}

/**
 *  service_notification_commands getter
 *
 *  @return an unordered map indexed by names of the service notification commands
 */
umap<std::string, shared_ptr<command> > const& contact::get_service_notification_commands() const {
  return _service_notification_commands;
}

void contact::set_host_notification_period(timeperiod* tp) {
  _host_notification_period = tp;
}

void contact::set_service_notification_period(timeperiod* tp) {
  _service_notification_period = tp;
}

bool contact::notify_on_service_critical() const {
  return (_service_notified_states & notifier::ON_CRITICAL);
}

bool contact::notify_on_service_recovery() const {
  return (_service_notified_states & notifier::ON_RECOVERY);
}

bool contact::notify_on_service_warning() const {
  return (_service_notified_states & notifier::ON_WARNING);
}

bool contact::notify_on_host_recovery() const {
  return (_host_notified_states & (1 << notifier::ON_RECOVERY));
}

bool contact::notify_on_host_down() const {
  return (_host_notified_states & (1 << notifier::ON_DOWN));
}

bool contact::notify_on_host_unreachable() const {
  return (_host_notified_states & (1 << notifier::ON_UNREACHABLE));
}

bool contact::contains_illegal_object_chars() const {
  if (_name.empty() || !illegal_object_chars)
    return false;
  return (_name.find(illegal_object_chars) != std::string::npos);
}

std::list<shared_ptr<contactgroup> > const& contact::get_contactgroups() const {
  return _contact_groups;
}

std::list<shared_ptr<contactgroup> >& contact::get_contactgroups() {
  return _contact_groups;
}

customvariablesmember_struct const* contact::get_custom_variables() const {
  return _custom_variables;
}

customvariablesmember_struct* contact::get_custom_variables() {
  return _custom_variables;
}

std::string const& contact::get_timezone() const {
  return _timezone;
}

std::string const& contact::get_address(int index) const {
  return _address[index];
}

/**
 *  Update contact status info.
 *
 *  @param aggregated_dump
 */
void contact::update_status(int aggregated_dump) {
  /* send data to event broker (non-aggregated dumps only) */
  if (!aggregated_dump)
    broker_contact_status(
      NEBTYPE_CONTACTSTATUS_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      this,
      NULL);
}

unsigned long contact::get_modified_attributes() const {
  return _modified_attributes;
}

void contact::set_modified_attributes(unsigned long attr) {
  _modified_attributes = attr;
}

unsigned long contact::get_modified_host_attributes() const {
  return _modified_host_attributes;
}

void contact::set_modified_host_attributes(unsigned long attr) {
  _modified_host_attributes = attr;
}

unsigned long contact::get_modified_service_attributes() const {
  return _modified_service_attributes;
}

void contact::set_modified_service_attributes(unsigned long attr) {
  _modified_service_attributes = attr;
}

time_t contact::get_last_host_notification() const {
  return _last_host_notification;
}

void contact::set_last_host_notification(time_t t) {
  _last_host_notification = t;
}

time_t contact::get_last_service_notification() const {
  return _last_service_notification;
}

void contact::set_last_service_notification(time_t t) {
  _last_service_notification = t;
}

timeperiod_struct* contact::get_host_notification_period() const {
  return _host_notification_period;
}

timeperiod_struct* contact::get_service_notification_period() const {
  return _service_notification_period;
}

bool contact::is_host_notifications_enabled() const {
  return _host_notifications_enabled;
}

void contact::set_host_notifications_enabled(bool enabled) {
  _host_notifications_enabled = enabled;
}

bool contact::is_service_notifications_enabled() const {
  return _service_notifications_enabled;
}

void contact::set_service_notifications_enabled(bool enabled) {
  _service_notifications_enabled = enabled;
}

std::string const& contact::get_host_notification_period_name() const {
  return _host_notification_period_name;
}

void contact::set_host_notification_period_name(std::string const& name) {
  _host_notification_period_name = name;
}

std::string const& contact::get_service_notification_period_name() const {
  return _service_notification_period_name;
}

void contact::set_service_notification_period_name(std::string const& name) {
  _service_notification_period_name = name;
}

void contact::add_host_notification_command(char const* command_name) {
  // Make sure we have the data we need.
  if (!command_name || !command_name[0]) {
    throw (engine_error()
             << "Error: Host notification command is NULL");
  }

  _host_notification_commands[command_name] = shared_ptr<command_struct>(0);
}

void contact::add_service_notification_command(char const* command_name) {
  // Make sure we have the data we need.
  if (!command_name || !command_name[0]) {
    throw (engine_error()
             << "Error: Service notification command is NULL");
  }

  _service_notification_commands[command_name] = shared_ptr<command_struct>(0);
}

/**
 *  Adds a custom variable to this contact.
 *
 *  @param[in] varname  Custom variable name.
 *  @param[in] varvalue Custom variable value.
 *
 */
void contact::add_custom_variable(char const* varname, char const* varvalue) {
  // Add custom variable to contact.
  customvariablesmember* retval(add_custom_variable_to_object(
                                  &_custom_variables,
                                  varname,
                                  varvalue));

  if (retval == NULL) {
    throw (engine_error()
             << "Error: Unable to add custom variable to contact");
  }
  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));

  broker_custom_variable(
    NEBTYPE_CONTACTCUSTOMVARIABLE_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this,
    varname,
    varvalue,
    &tv);
}

bool contact::update_custom_variable(
  std::string const& varname,
  std::string const& varvalue) {

  for (customvariablesmember* m(_custom_variables); m; m = m->next) {
    if (varname == m->variable_name) {
      if (varvalue != m->variable_value) {
        string::setstr(m->variable_value, varvalue);
        m->has_been_modified = true;
      }
      return true;
    }
  }
  return false;
}

void contact::update_config(configuration::contact const& obj) {
  configuration::applier::modify_if_different(
    _alias, obj.alias().empty() ? obj.contact_name() : obj.alias());
  configuration::applier::modify_if_different(_email, obj.email());
  configuration::applier::modify_if_different(_pager, obj.pager());
  configuration::applier::modify_if_different(_address, obj.address());
  configuration::applier::modify_if_different(
    _notify_on_service_unknown,
    static_cast<bool>(obj.service_notification_options() & configuration::service::unknown));
  configuration::applier::modify_if_different(
    _notify_on_service_warning,
   static_cast<bool>(obj.service_notification_options() & configuration::service::warning));
  configuration::applier::modify_if_different(
    _notify_on_service_critical,
    static_cast<bool>(obj.service_notification_options() & configuration::service::critical));
  configuration::applier::modify_if_different(
    _notify_on_service_recovery,
    static_cast<bool>(obj.service_notification_options() & configuration::service::ok));
  configuration::applier::modify_if_different(
    _notify_on_service_flapping,
    static_cast<bool>(obj.service_notification_options() & configuration::service::flapping));
  configuration::applier::modify_if_different(
    _notify_on_service_downtime,
    static_cast<bool>(obj.service_notification_options() & configuration::service::downtime));
  configuration::applier::modify_if_different(
    _notify_on_host_down,
    static_cast<bool>(obj.host_notification_options() & configuration::host::down));
  configuration::applier::modify_if_different(
    _notify_on_host_unreachable,
    static_cast<bool>(obj.host_notification_options() & configuration::host::unreachable));
  configuration::applier::modify_if_different(
    _notify_on_host_recovery,
    static_cast<bool>(obj.host_notification_options() & configuration::host::up));
  configuration::applier::modify_if_different(
    _notify_on_host_flapping,
    static_cast<bool>(obj.host_notification_options() & configuration::host::flapping));
  configuration::applier::modify_if_different(
    _notify_on_host_downtime,
    static_cast<bool>(obj.host_notification_options() & configuration::host::downtime));
  configuration::applier::modify_if_different(
    _host_notification_period_name, obj.host_notification_period());
  configuration::applier::modify_if_different(
    _service_notification_period_name, obj.service_notification_period());
  configuration::applier::modify_if_different(
    _host_notifications_enabled, obj.host_notifications_enabled());
  configuration::applier::modify_if_different(
    _service_notifications_enabled, obj.service_notifications_enabled());
  configuration::applier::modify_if_different(
    _can_submit_commands, obj.can_submit_commands());
  configuration::applier::modify_if_different(
    _retain_status_information, obj.retain_status_information());
  configuration::applier::modify_if_different(
    _retain_nonstatus_information, obj.retain_nonstatus_information());
  configuration::applier::modify_if_different(
    _timezone, obj.timezone());
}

void contact::clear_host_notification_commands() {
  _host_notification_commands.clear();
}

void contact::clear_service_notification_commands() {
  _service_notification_commands.clear();
}

void contact::clear_custom_variables() {
  // Browse all custom vars.
  customvariablesmember* m(_custom_variables);
  _custom_variables = NULL;
  while (m) {
    // Point to next custom var.
    customvariablesmember* to_delete(m);
    m = m->next;

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_custom_variable(
      NEBTYPE_CONTACTCUSTOMVARIABLE_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      this,
      to_delete->variable_name,
      to_delete->variable_value,
      &tv);

    // Delete custom variable.
    deleter::customvariablesmember(to_delete);
  }
}

bool contact::operator<(contact const& other) {
  if (_name.compare(other._name) < 0)
    return true;
  if (_alias.compare(other._alias) < 0)
    return true;
  return false;
}

bool contact::get_retain_status_information() const {
  return _retain_status_information;
}

bool contact::get_retain_nonstatus_information() const {
  return _retain_nonstatus_information;
}

/* enables host notifications for a contact */
void contact::enable_host_notifications() {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (is_host_notifications_enabled())
    return;

  /* set the attribute modified flag */
  _modified_host_attributes |= attr;

  /* enable the host notifications... */
  set_host_notifications_enabled(true);

  /* send data to event broker */
  broker_adaptive_contact_data(
    NEBTYPE_ADAPTIVECONTACT_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this,
    CMD_NONE,
    MODATTR_NONE,
    get_modified_attributes(),
    attr,
    get_modified_host_attributes(),
    MODATTR_NONE,
    get_modified_service_attributes(),
    NULL);

  /* update the status log to reflect the new contact state */
  update_status(false);
}

/* disables host notifications for a contact */
void contact::disable_host_notifications() {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (!is_host_notifications_enabled())
    return;

  /* set the attribute modified flag */
  _modified_host_attributes |= attr;

  /* enable the host notifications... */
  set_host_notifications_enabled(false);

  /* send data to event broker */
  broker_adaptive_contact_data(
    NEBTYPE_ADAPTIVECONTACT_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this,
    CMD_NONE,
    MODATTR_NONE,
    get_modified_attributes(),
    attr,
    get_modified_host_attributes(),
    MODATTR_NONE,
    get_modified_service_attributes(),
    NULL);

  /* update the status log to reflect the new contact state */
  update_status(false);
}

/* enables service notifications for a contact */
void contact::enable_service_notifications() {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (is_service_notifications_enabled())
    return;

  /* set the attribute modified flag */
  _modified_service_attributes |= attr;

  /* enable the host notifications... */
  set_service_notifications_enabled(true);

  /* send data to event broker */
  broker_adaptive_contact_data(
    NEBTYPE_ADAPTIVECONTACT_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this,
    CMD_NONE,
    MODATTR_NONE,
    get_modified_attributes(),
    MODATTR_NONE,
    get_modified_host_attributes(),
    attr,
    get_modified_service_attributes(),
    NULL);

  /* update the status log to reflect the new contact state */
  update_status(false);
}

/* disables service notifications for a contact */
void contact::disable_service_notifications() {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (!is_service_notifications_enabled())
    return;

  /* set the attribute modified flag */
  _modified_service_attributes |= attr;

  /* enable the host notifications... */
  set_service_notifications_enabled(false);

  /* send data to event broker */
  broker_adaptive_contact_data(
    NEBTYPE_ADAPTIVECONTACT_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this,
    CMD_NONE,
    MODATTR_NONE,
    get_modified_attributes(),
    MODATTR_NONE,
    get_modified_host_attributes(),
    attr,
    get_modified_service_attributes(),
    NULL);

  /* update the status log to reflect the new contact state */
  update_status(false);
}
