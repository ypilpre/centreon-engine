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
#include "com/centreon/engine/commands/command.hh"
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
 * Destructor.
 */
contact::~contact() {}

/**************************************
*                                     *
*           Base properties           *
*                                     *
**************************************/

/**
 *  Get a single address.
 *
 *  @param[in] index  Address index (starting from 0).
 *
 *  @return The requested address.
 */
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
 *  Get the contact's modified attributes.
 *
 *  @return A bitmask, representing modified attributes.
 */
unsigned long contact::get_modified_attributes() const {
  return (_modified_attributes);
}

/**
 *  Set the contact's modified attributes.
 *
 *  @param[in] attr  Modified attributes.
 */
void contact::set_modified_attributes(unsigned long attr) {
  _modified_attributes = attr;
  return ;
}

/**
 *  Return the contact name.
 *
 *  @return A reference to the name.
 */
std::string const& contact::get_name() const {
  return (_name);
}

/**
 *  Set the contact name.
 *
 *  @param[in] name  New name.
 */
void contact::set_name(std::string const& name) {
  _name = name;
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
 *  Add new host notification command to contact.
 *
 *  @param[in,out] cmd   Command that will be used by contact for
 *                       host notification.
 *  @param[in]     args  Optional command arguments.
 */
void contact::add_host_notification_command(
                commands::command* cmd,
                std::string const& args) {
  _host_notification_commands.push_back(std::make_pair(cmd, args));
  return ;
}

/**
 *  Clear host notification commands.
 */
void contact::clear_host_notification_commands() {
  _host_notification_commands.clear();
  return ;
}

/**
 *  Get host notification commands list.
 *
 *  @return A list of commands.
 */
std::list<std::pair<commands::command*, std::string> > const& contact::get_host_notification_commands() const {
  return (_host_notification_commands);
}

/**
 *  Check if host notifications are enabled for this contact.
 *
 *  @return True if notifications are enabled.
 */
bool contact::get_host_notifications_enabled() const {
  return (_host_notifications_enabled);
}

/**
 *  Enable or disable host notifications for this contact.
 *
 *  @param[in] enable  True to enable.
 */
void contact::set_host_notifications_enabled(bool enable) {
  _host_notifications_enabled = enable;
  return ;
}

/**
 *  Check if notification is enabled for a specific state.
 *
 *  @param[in] state  State to check.
 *
 *  @return True if notification is enabled for state.
 */
bool contact::get_host_notify_on(
                notifications::notifier::action_on state) const {
  return (_host_notified_states & state);
}

/**
 *  Enable or disable notification on a host state.
 *
 *  @param[in] state   Target state.
 *  @param[in] enable  True to enable.
 */
void contact::set_host_notify_on(
                notifications::notifier::action_on state,
                bool enable) {
  if (enable)
    _host_notified_states |= state;
  else
    _host_notified_states &= ~state;
  return ;
}

/**
 *  Get the host notification period.
 *
 *  @return A pointer to the host notification period.
 */
timeperiod_struct* contact::get_host_notification_period() const {
  return (_host_notification_period);
}

/**
 *  Set the host notification period.
 *
 *  @param[in] tp  Pointer to the new host notification period.
 */
void contact::set_host_notification_period(timeperiod* tp) {
  _host_notification_period = tp;
  return ;
}

/**
 *  Get the last time a host notification was sent for this contact.
 *
 *  @return A timestamp.
 */
time_t contact::get_last_host_notification() const {
  return (_last_host_notification);
}

/**
 *  Set the last time a host notification was sent.
 *
 *  @param[in] t  Timestamp.
 */
void contact::set_last_host_notification(time_t t) {
  _last_host_notification = t;
  return ;
}

/**
 *  Get the modified host attributes.
 *
 *  @return A bitmask.
 */
unsigned long contact::get_modified_host_attributes() const {
  return (_modified_host_attributes);
}

/**
 *  Set the modified host attributes.
 *
 *  @param[in] attr  Modified host attributes.
 */
void contact::set_modified_host_attributes(unsigned long attr) {
  _modified_host_attributes = attr;
  return ;
}

/**************************************
*                                     *
*   Service notification properties   *
*                                     *
**************************************/

/**
 *  Add new service notification command to contact.
 *
 *  @param[in,out] cmd   Command that will be used by contact for
 *                       service notification.
 *  @param[in]     args  Optional command arguments.
 */
void contact::add_service_notification_command(
                commands::command* cmd,
                std::string const& args) {
  _service_notification_commands.push_back(std::make_pair(cmd, args));
  return ;
}

/**
 *  Clear service notification commands.
 */
void contact::clear_service_notification_commands() {
  _service_notification_commands.clear();
  return ;
}

/**
 *  Get service notification commands list.
 *
 *  @return A list of commands.
 */
std::list<std::pair<commands::command*, std::string> > const& contact::get_service_notification_commands() const {
  return (_service_notification_commands);
}

/**
 *  Check if service notifications are enabled for this service.
 *
 *  @return True if notifications are enabled.
 */
bool contact::get_service_notifications_enabled() const {
  return (_service_notifications_enabled);
}

/**
 *  Enable or disable service notifications for this contact.
 *
 *  @param[in] enable  True to enable.
 */
void contact::set_service_notifications_enabled(bool enable) {
  _service_notifications_enabled = enable;
  return ;
}

/**
 *  Check if notification is enabled for a specific state.
 *
 *  @param[in] state  State to check.
 *
 *  @return True if notification is enabled for state.
 */
bool contact::get_service_notify_on(
                notifications::notifier::action_on state) const {
  return (_service_notified_states & state);
}

/**
 *  Enable or disable notification on a service state.
 *
 *  @param[in] state   Target state.
 *  @param[in] enable  True to enable.
 */
void contact::set_service_notify_on(
                notifications::notifier::action_on state,
                bool enable) {
  if (enable)
    _service_notified_states |= state;
  else
    _service_notified_states &= ~state;
  return ;
}

/**
 *  Get the service notification period.
 *
 *  @return Pointer to the notification period.
 */
timeperiod_struct* contact::get_service_notification_period() const {
  return _service_notification_period;
}

/**
 *  Set service notification period.
 *
 *  @param[in] tp  Pointer to the new service notification period.
 */
void contact::set_service_notification_period(timeperiod* tp) {
  _service_notification_period = tp;
  return ;
}

/**
 *  Get the last time a service notification was sent.
 *
 *  @return Timestamp.
 */
time_t contact::get_last_service_notification() const {
  return (_last_service_notification);
}

/**
 *  Set the last time a service notification was sent.
 *
 *  @param[in] t  Timestamp.
 */
void contact::set_last_service_notification(time_t t) {
  _last_service_notification = t;
  return ;
}

/**
 *  Get modified service attributes.
 *
 *  @return A bitmask.
 */
unsigned long contact::get_modified_service_attributes() const {
  return (_modified_service_attributes);
}

/**
 *  Set the service modified attributes.
 *
 *  @param[in] attr  Service modified attributes.
 */
void contact::set_modified_service_attributes(unsigned long attr) {
  _modified_service_attributes = attr;
  return ;
}

/**************************************
*                                     *
*           Contact groups            *
*                                     *
**************************************/

/**
 *  Add a contact group to this contact.
 *
 *  @param[in] cg  Pointer to contact group.
 */
void contact::add_contactgroup(contactgroup* cg) {
  _contact_groups.push_back(cg);
  return ;
}

/**
 *  Clear contact group list.
 */
void contact::clear_contactgroups() {
  _contact_groups.clear();
  return ;
}

/**
 *  Get the contact group list.
 *
 *  @return List of pointers to contact groups.
 */
std::list<contactgroup*> const& contact::get_contactgroups() const {
  return (_contact_groups);
}

/**************************************
*                                     *
*          Custom variables           *
*                                     *
**************************************/

/**
 *  Clear all custom variables.
 */
void contact::clear_custom_variables() {
  for (customvar_set::iterator it(_vars.begin()), end(_vars.end());
       it != end;)
    it = _vars.erase(it);
  return ;
}

/**
 *  Get custom variables.
 *
 *  @return Custom variables.
 */
customvar_set const& contact::get_customvars() const {
  return (_vars);
}

/**
 *  Set a custom variable.
 *
 *  @param[in] var  Custom variable.
 */
void contact::set_customvar(customvar const& var) {
  _vars[var.get_name()] = var;
  return ;
}
