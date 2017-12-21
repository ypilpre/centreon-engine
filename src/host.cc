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

#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/service.hh"

using namespace com::centreon::engine;

/**
 *  Constructor.
 *
 *  @param[in] cfg  Base configuration.
 */
host::host(configuration::host const& cfg)
// XXX
{}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
host::host(host const& other) : monitorable(other) {
  _internal_copy(other);
}

/**
 *  Destructor.
 */
host::~host() {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
host& host::operator=(host const& other) {
  if (this != &other)
    _internal_copy(other);
  return (*this);
}

/**************************************
*                                     *
*           Configuration             *
*                                     *
**************************************/

/**
 *  Get the ip address.
 *
 *  @return a string representing the ip address.
 */
std::string const& host::get_address() const {
  return (_address);
}

/**
 *  Get the alias of this host.
 *
 *  @return The alias of this host.
 */
std::string const& host::get_alias() const {
  return (_alias);
}

/**
 *  Get circular path checking status.
 *
 *  @return Circular path checking status.
 */
int host::get_circular_path_checked() const {
  return (_circular_path_checked);
}

/**
 *  Set circular path checking status.
 *
 *  @param[in] check_level  Circular check status.
 */
void host::set_circular_path_checked(int check_level) {
  _circular_path_checked = check_level;
  return ;
}

/**
 *  Get initial state.
 *
 *  @return Initial state.
 */
int host::get_initial_state() const {
  return (_initial_state);
}

/**
 *  Check if host should be stalked on down states.
 *
 *  @return True if host should be stalked.
 */
bool host::get_stalk_on_down() const {
  return (_stalk_on_down);
}

/**
 *  Check if host should be stalked on unreachable states.
 *
 *  @return True if host should be stalked.
 */
bool host::get_stalk_on_unreachable() const {
  return (_stalk_on_unreachable);
}

/**
 *  Check if host should be stalked on up states.
 *
 *  @return True if host should be stalked.
 */
bool host::get_stalk_on_up() const {
  return (_stalk_on_up);
}

/**************************************
*                                     *
*      Links with other objects       *
*                                     *
**************************************/

/**
 *  Add child to this host.
 *
 *  @param[in] hst  Child of this host.
 */
void host::add_child(shared_ptr<host> hst) {
  // XXX
}

/**
 *  Clear child list of this host.
 */
void host::clear_children() {
  // XXX
}

/**
 *  Get children.
 *
 *  @return List of children.
 */
std::list<shared_ptr<host> > const& host::get_children() const {
  return (_children);
}

/**
 *  Add host group to this host.
 *
 *  @param[in] hg  Host group.
 */
void host::add_group(hostgroup_struct* hg) {
  // XXX
}

/**
 *  Clear group list of this host.
 */
void host::clear_groups() {
  // XXX
}

/**
 *  Get groups of this host.
 *
 *  @return Groups of this host.
 */
hostgroup_set const& host::get_groups() const {
  // XXX
}

/**
 *  Add a parent to this host.
 *
 *  @param[in] hst  New parent.
 */
void host::add_parent(shared_ptr<host> hst) {
  _parents.push_back(hst);
  return ;
}

/**
 *  Clear parent list of this host.
 */
void host::clear_parents() {
  // XXX
}

/**
 *  Get parents.
 *
 *  @return List of parents.
 */
std::list<shared_ptr<host> > const& host::get_parents() const {
  return (_parents);
}

/**
 *  Add a service to this host.
 *
 *  @param[in] svc  Service.
 */
void host::add_service(shared_ptr<service> svc) {
  _total_service_check_interval += svc->get_normal_check_interval();
  // XXX
}

/**
 *  Clear the service list of this host.
 */
void host::clear_services() {
  _total_service_check_interval = 0;
  // XXX
}

/**
 *  Get the list of services of this host.
 *
 *  @return List of services of this host.
 */
std::list<shared_ptr<service> > const& host::get_services() const {
  // XXX
  static std::list<shared_ptr<service> > retval;
  return retval;
}

/**
 *  Get the total service check interval.
 *
 *  @return Total service check interval.
 */
int host::get_total_service_check_interval() const {
  return (_total_service_check_interval);
}

/**************************************
*                                     *
*           State runtime             *
*                                     *
**************************************/

/**
 *  Get last time host was down.
 *
 *  @return Last time host was down.
 */
time_t host::get_last_time_down() const {
  return (_last_time_down);
}

/**
 *  Set last time host was down.
 *
 *  @param[in] last_time  Last time host was down.
 */
void host::set_last_time_down(time_t last_time) {
  _last_time_down = last_time;
  return ;
}

/**
 *  Get last time host was unreachable.
 *
 *  @return Last time host was unreachable.
 */
time_t host::get_last_time_unreachable() const {
  return (_last_time_unreachable);
}

/**
 *  Set last time host was unreachable.
 *
 *  @param[in] last_time  Last time host was unreachable.
 */
void host::set_last_time_unreachable(time_t last_time) {
  _last_time_unreachable = last_time;
  return ;
}

/**
 *  Get last time host was up.
 *
 *  @return Last time host was up.
 */
time_t host::get_last_time_up() const {
  return (_last_time_up);
}

/**
 *  Set last time host was up.
 *
 *  @param[in] last_time  Last time host was up.
 */
void host::set_last_time_up(time_t last_time) {
  _last_time_up = last_time;
  return ;
}

/**
 *  Check if host's current check should be rescheduled.
 *
 *  @return True if check should be rescheduled.
 */
bool host::get_should_reschedule_current_check() const {
  return (_should_reschedule_current_check);
}

/**
 *  Set if host's current check should be rescheduled.
 *
 *  @param[in] reschedule  True if check should be rescheduled.
 */
void host::set_should_reschedule_current_check(bool reschedule) {
  _should_reschedule_current_check = reschedule;
  return ;
}

/**************************************
*                                     *
*           Flap detection            *
*                                     *
**************************************/

/**
 *  Check if flap detection is enabled for UP state.
 *
 *  @return True if flap detection is enabled for UP state.
 */
bool host::get_flap_detection_on_up() const {
  return (_flap_detection_on_up);
}

/**
 *  Check if flap detection is enabled for DOWN state.
 *
 *  @return True if flap detection is enabled for DOWN state.
 */
bool host::get_flap_detection_on_down() const {
  return (_flap_detection_on_down);
}

/**
 *  Check if flap detection is enabled for UNREACHABLE state.
 *
 *  @return True if flap detection is enabled for UNREACHABLE state.
 */
bool host::get_flap_detection_on_unreachable() const {
  return (_flap_detection_on_unreachable);
}

/**
 *  Get last historical state update.
 *
 *  @return Last historical state update.
 */
time_t host::get_last_historical_state_update() const {
  return (_last_historical_state_update);
}

/**
 *  Set last historical state update.
 *
 *  @param[in] last_update  Last historical state update.
 */
void host::set_last_historical_state_update(time_t last_update) {
  _last_historical_state_update = last_update;
  return ;
}

/**************************************
*                                     *
*            Notification             *
*                                     *
**************************************/

/**
 *  Check if host should notify on down states.
 *
 *  @return True if host should notify.
 */
bool host::get_notify_on_down() const {
  return (_notify_on_down);
}

/**
 *  Set whether or not host should notify on down states.
 *
 *  @param[in] notify  True to notify.
 */
void host::set_notify_on_down(bool notify) {
  _notify_on_down = notify;
  return ;
}

/**
 *  Check if host should notify on unreachable states.
 *
 *  @return True if host should notify.
 */
bool host::get_notify_on_unreachable() const {
  return (_notify_on_unreachable);
}

/**
 *  Set whether or not host should notify on unreachable states.
 *
 *  @param[in] notify  True to notify.
 */
void host::set_notify_on_unreachable(bool notify) {
  _notify_on_unreachable = notify;
  return ;
}


/**************************************
*                                     *
*          Private methods            *
*                                     *
**************************************/

/**
 *  Copy internal data members.
 *
 *  @param[in] other  Object to copy.
 */
void host::_internal_copy(host const& other) {
  _children = other._children;
  _initial_state = other._initial_state;
  _last_time_down = other._last_time_down;
  _last_time_unreachable = other._last_time_unreachable;
  _last_time_up = other._last_time_up;
  _notify_on_down = other._notify_on_down;
  _notify_on_unreachable = other._notify_on_unreachable;
  _parents = other._parents;
  _should_reschedule_current_check = other._should_reschedule_current_check;
  _stalk_on_down = other._stalk_on_down;
  _stalk_on_unreachable = other._stalk_on_unreachable;
  _stalk_on_up = other._stalk_on_up;
  return ;
}

void host::_checkable_macro_builder(nagios_macros& mac) {
  // Save pointer to host.
  mac.host_ptr = this;
  mac.hostgroup_ptr = NULL;
}
