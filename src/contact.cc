/*
** Copyright 2017-2018 Centreon
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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/not_found.hh"
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/engine/objects/timeperiod.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::notifications;


/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Constructor.
 */
contact::contact() {}

/**
 *  Constructor from a configuration contact
 *
 * @param obj Configuration contact
 */
contact::contact(configuration::contact const& obj)
  : _name(obj.contact_name()),
    _alias((obj.alias().empty()) ? obj.contact_name() : obj.alias()),
    _email(obj.email()),
    _pager(obj.pager()),
    _addresses(obj.address()),
    _service_notified_states(
        ((obj.service_notification_options() & configuration::service::ok)
         ? notifier::ON_RECOVERY : 0)
      | ((obj.service_notification_options() & configuration::service::critical)
         ? notifier::ON_CRITICAL : 0)
      | ((obj.service_notification_options() & configuration::service::warning)
         ? notifier::ON_WARNING : 0)
      | ((obj.service_notification_options() & configuration::service::unknown)
         ? notifier::ON_UNKNOWN : 0)
      | ((obj.service_notification_options() & configuration::service::flapping)
         ? notifier::ON_FLAPPING : 0)
      | ((obj.service_notification_options() & configuration::service::downtime)
         ? notifier::ON_DOWNTIME : 0)),
    _host_notified_states(
        ((obj.host_notification_options() & configuration::host::up)
          ? notifier::ON_RECOVERY : 0)
      | ((obj.host_notification_options() & configuration::host::down)
          ? notifier::ON_DOWN : 0)
      | ((obj.host_notification_options() & configuration::host::unreachable)
          ? notifier::ON_UNREACHABLE : 0)
      | ((obj.host_notification_options() & configuration::host::flapping)
          ? notifier::ON_FLAPPING : 0)
      | ((obj.host_notification_options() & configuration::host::downtime)
          ? notifier::ON_DOWNTIME : 0)),
    _host_notifications_enabled(obj.host_notifications_enabled()),
    _modified_attributes(MODATTR_NONE),
    _modified_host_attributes(MODATTR_NONE),
    _modified_service_attributes(MODATTR_NONE),
    _retain_nonstatus_information(obj.retain_nonstatus_information()),
    _retain_status_information(obj.retain_status_information()),
    _service_notifications_enabled(obj.service_notifications_enabled()),
    _timezone(obj.timezone()) {

  // Make sure we have the data we need.
  if (_name.empty())
    throw (engine_error() << "contact: Contact name is empty");

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

/**************************************
*                                     *
*           Base properties           *
*                                     *
**************************************/

/**
 *  Return the contact name
 *
 *  @return a reference to the name
 */
std::string const& contact::get_name() const {
  return (_name);
}

/**
 *  Return the contact alias
 *
 *  @return a reference to the alias
 */
std::string const& contact::get_alias() const {
  return (_alias);
}

/**
 *  Set alias.
 *
 *  @param[in] alias  New alias.
 */
void contact::set_alias(std::string const& alias) {
  _alias = alias;
  return ;
}

/**
 *  Check if contact can submit commands.
 *
 *  @return True if contact can submit commands.
 */
bool contact::get_can_submit_commands() const {
  return (_can_submit_commands);
}

/**
 *  (Dis)Allow a contact to submit commands.
 *
 *  @param[in] can_submit  True to enable contact to send commands.
 */
void contact::set_can_submit_commands(bool can_submit) {
  _can_submit_commands = can_submit;
  return ;
}

/**
 *  Return the contact email
 *
 *  @return a reference to the email
 */
std::string const& contact::get_email() const {
  return (_email);
}

/**
 *  Set contact email.
 *
 *  @param[in] email  New email.
 */
void contact::set_email(std::string const& email) {
  _email = email;
  return ;
}

/**
 *  Return the contact pager
 *
 *  @return a reference to the pager
 */
std::string const& contact::get_pager() const {
  return (_pager);
}

/**
 *  Set the pager.
 *
 *  @param[in] pager  New pager.
 */
void contact::set_pager(std::string const& pager) {
  _pager = pager;
  return ;
}

/**
 *  Check if status info should be retained.
 *
 *  @return True if status info should be retained.
 */
bool contact::get_retain_status_information() const {
  return (_retain_status_information);
}

/**
 *  Retain (or not) status info.
 *
 *  @param[in] retain  True to retain status info.
 */
void contact::set_retain_status_information(bool retain) {
  _retain_status_information = retain;
  return ;
}

/**
 *  Check if non-status info should be retained.
 *
 *  @return True if non-status info should be retained.
 */
bool contact::get_retain_nonstatus_information() const {
  return (_retain_nonstatus_information);
}

/**
 *  Retain (or not) non-status info.
 *
 *  @param[in] retain  True to retain non-status info.
 */
void contact::set_retain_nonstatus_information(bool retain) {
  _retain_nonstatus_information = retain;
  return ;
}

/**
 *  Get timezone.
 *
 *  @return Contact timezone.
 */
std::string const& contact::get_timezone() const {
  return (_timezone);
}

/**
 *  Set timezone.
 *
 *  @param[in] timezone  New contact timezone.
 */
void contact::set_timezone(std::string const& timezone) {
  _timezone = timezone;
  return ;
}

/**************************************
*                                     *
*     Host notification properties    *
*                                     *
**************************************/

/**
 *  Get host notified states.
 *
 *  @return Host notified states.
 */
unsigned int contact::get_host_notified_states() const {
  return (_host_notified_states);
}

/**
 *  Set host notified states.
 *
 *  @param[in] notified_states  New host notified states.
 */
void contact::set_host_notified_states(unsigned int notified_states) {
  _host_notified_states = notified_states;
  return ;
}

/**************************************
*                                     *
*   Service notification properties   *
*                                     *
**************************************/

/**
 *  Get service notified states.
 *
 *  @return Service notified states.
 */
unsigned int contact::get_service_notified_states() const {
  return (_service_notified_states);
}

/**
 *  Set service notified states.
 *
 *  @param[in] notified_states  New service notified states.
 */
void contact::set_service_notified_states(unsigned int notified_states) {
  _service_notified_states = notified_states;
  return ;
}

/**
 *  host_notification_commands getter
 *
 *  @return an unordered map indexed by names of the host notification commands
 */
command_map& contact::get_host_notification_commands() {
  return _host_notification_commands;
}

/**
 *  host_notification_commands getter
 *
 *  @return an unordered map indexed by names of the host notification commands
 */
command_map const& contact::get_host_notification_commands() const {
  return _host_notification_commands;
}

/**
 *  service_notification_commands getter
 *
 *  @return an unordered map indexed by names of the service notification commands
 */
command_map& contact::get_service_notification_commands() {
  return _service_notification_commands;
}

/**
 *  service_notification_commands getter
 *
 *  @return an unordered map indexed by names of the service notification commands
 */
command_map const& contact::get_service_notification_commands() const {
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
  return (_host_notified_states & notifier::ON_RECOVERY);
}

bool contact::notify_on_host_down() const {
  return (_host_notified_states & notifier::ON_DOWN);
}

bool contact::notify_on_host_unreachable() const {
  return (_host_notified_states & notifier::ON_UNREACHABLE);
}

void contact::clear_contactgroups() {
  _contact_groups.clear();
  return ;
}

std::list<shared_ptr<contactgroup> > const& contact::get_contactgroups() const {
  return (_contact_groups);
}

customvar_set const& contact::get_customvars() const {
  return _vars;
}

void contact::set_customvar(customvar const& var) {
  bool add(false);

  if (_vars.find(var.get_name()) == _vars.end())
    add = true;

  _vars[var.get_name()] = var;

  if (add) {
    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));

    broker_custom_variable(
        NEBTYPE_CONTACTCUSTOMVARIABLE_ADD,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        this,
        var.get_name().c_str(),
        var.get_value().c_str(),
        &tv);
  }
}

std::string const& contact::get_address(int index) const {
  return (_addresses[index]);
}

/**
 *  Get all addresses.
 *
 *  @return Array of addresses.
 */
std::vector<std::string> const& contact::get_addresses() const {
  return (_addresses);
}

/**
 *  Set addresses.
 *
 *  @param[in] addresses  New addresses.
 */
void contact::set_addresses(std::vector<std::string> const& addresses) {
  _addresses = addresses;
  return ;
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

bool contact::get_host_notifications_enabled() const {
  return _host_notifications_enabled;
}

void contact::set_host_notifications_enabled(bool enabled) {
  _host_notifications_enabled = enabled;
}

bool contact::get_service_notifications_enabled() const {
  return _service_notifications_enabled;
}

void contact::set_service_notifications_enabled(bool enabled) {
  _service_notifications_enabled = enabled;
}

void contact::add_host_notification_command(std::string const& command_name) {
  // Make sure we have the data we need.
  if (command_name.empty())
    throw (engine_error()
             << "Error: Host notification command is empty");

  _host_notification_commands[command_name] = shared_ptr<command>(0);
}

void contact::add_service_notification_command(std::string const& command_name) {
  // Make sure we have the data we need.
  if (command_name.empty())
    throw (engine_error()
             << "Error: Service notification command is empty");

  _service_notification_commands[command_name] = shared_ptr<command>(0);
}

void contact::clear_host_notification_commands() {
  _host_notification_commands.clear();
}

void contact::clear_service_notification_commands() {
  _service_notification_commands.clear();
}

void contact::clear_custom_variables() {
  // Browse all custom vars.
  for (customvar_set::iterator
         it(_vars.begin()),
         end(_vars.end());
       it != end;) {

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_custom_variable(
      NEBTYPE_CONTACTCUSTOMVARIABLE_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      this,
      it->second.get_name().c_str(),
      it->second.get_value().c_str(),
      &tv);

    it = _vars.erase(it);
  }
}

bool contact::operator<(contact const& other) {
  if (_name.compare(other._name) < 0)
    return true;
  if (_alias.compare(other._alias) < 0)
    return true;
  return false;
}

/* enables host notifications for a contact */
void contact::enable_host_notifications() {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (get_host_notifications_enabled())
    return ;

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
  if (!get_host_notifications_enabled())
    return ;

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
  if (get_service_notifications_enabled())
    return ;

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
  if (!get_service_notifications_enabled())
    return ;

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
