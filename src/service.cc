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

#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/service.hh"

using namespace com::centreon::engine;

/**
 *  Constructor.
 *
 *  @param[in] cfg  Base configuration.
 */
service::service(configuration::service const& cfg)
// XXX
{}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
service::service(service const& other) : monitorable(other) {
  _internal_copy(other);
}

/**
 *  Destructor.
 */
service::~service() {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
service& service::operator=(service const& other) {
  if (this != &other)
    _internal_copy(other);
  return (*this);
}

/**************************************
*                                     *
*            Configuration            *
*                                     *
**************************************/

/**
 *  Get service description.
 *
 *  @return Service description.
 */
std::string const& service::get_description() const {
  return (_description);
}

/**
 *  Check if this service should be stalked on critical state.
 *
 *  @return True if this service should be stalked on critical state.
 */
bool service::get_stalk_on_critical() const {
  return (_stalk_on_critical);
}

/**
 *  Check if this service should be stalked on ok state.
 *
 *  @return True if the service should be stalked on ok state.
 */
bool service::get_stalk_on_ok() const {
  return (_stalk_on_ok);
}

/**
 *  Check if this service should be stalk on unknown state.
 *
 *  @return If this service should be stalked on unknown state.
 */
bool service::get_stalk_on_unknown() const {
  return (_stalk_on_unknown);
}

/**
 *  Check if this service should be stalked on warning state.
 *
 *  @return True if this service should be stalked on warning state.
 */
bool service::get_stalk_on_warning() const {
  return (_stalk_on_warning);
}

/**
 *  Check if this warning is volatile.
 *
 *  @return True if service is volatile.
 */
bool service::get_volatile() const {
  return (_volatile);
}

/**************************************
*                                     *
*      Links with other objects       *
*                                     *
**************************************/

/**
 *  Get service's host.
 *
 *  @return Host that this service is running on.
 */
host* service::get_host() const {
  return (_host);
}

/**
 *  Set service's host.
 *
 *  @param[in] hst  Service's host.
 */
void service::set_host(host* hst) {
  _host = hst;
  return ;
}

/**
 *  Add a service group to this service.
 *
 *  @param[in] sg  Service group.
 */
void service::add_servicegroup(servicegroup_struct* sg) {
  // XXX
}

/**************************************
*                                     *
*            State runtime            *
*                                     *
**************************************/

/**
 *  Get last time service was critical.
 *
 *  @return Last time service was critical.
 */
time_t service::get_last_time_critical() const {
  return (_last_time_critical);
}

/**
 *  Set last time service was critical.
 *
 *  @param[in] last_critical  Last time service was critical.
*/
void service::set_last_time_critical(time_t last_critical) {
  _last_time_critical = last_critical;
  return ;
}

/**
 *  Get last time service was ok.
 *
 *  @return Last time service was ok.
 */
time_t service::get_last_time_ok() const {
  return (_last_time_ok);
}

/**
 *  Set last time service was ok.
 *
 *  @param[in] last_ok  Last time service was ok.
 */
void service::set_last_time_ok(time_t last_ok) {
  _last_time_ok = last_ok;
  return ;
}

/**
 *  Get last time service was unknown.
 *
 *  @return Last time service was unknown.
 */
time_t service::get_last_time_unknown() const {
  return (_last_time_unknown);
}

/**
 *  Set last time service was unknown.
 *
 *  @param[in] last_unknown  Last time service was unknown.
 */
void service::set_last_time_unknown(time_t last_unknown) {
  _last_time_unknown = last_unknown;
  return ;
}

/**
 *  Get last time service was warning.
 *
 *  @return Last time service was warning.
 */
time_t service::get_last_time_warning() const {
  return (_last_time_warning);
}

/**
 *  Set last time service was warning.
 *
 *  @param[in] last_warning  Last time service was warning.
 */
void service::set_last_time_warning(time_t last_warning) {
  _last_time_warning = last_warning;
  return ;
}

/**************************************
*                                     *
*            Flap detection           *
*                                     *
**************************************/

/**
 *  Check if flap detection is enabled for OK state.
 *
 *  @return True if flap detection is enabled for OK state.
 */
bool service::get_flap_detection_on_ok() const {
  return (_flap_detection_on_ok);
}

/**
 *  Check if flap detection is enabled for WARNING state.
 *
 *  @return True if flap detection is enabled for WARNING state.
 */
bool service::get_flap_detection_on_warning() const {
  return (_flap_detection_on_warning);
}

/**
 *  Check if flap detection is enabled for UNKNOWN state.
 *
 *  @return True if flap detection is enabled for UNKNOWN state.
 */
bool service::get_flap_detection_on_unknown() const {
  return (_flap_detection_on_unknown);
}

/**
 *  Check if flap detection is enabled for CRITICAL state.
 *
 *  @return True if flap detection is enabled for CRITICAL state.
 */
bool service::get_flap_detection_on_critical() const {
  return (_flap_detection_on_critical);
}

/**************************************
*                                     *
*             Notification            *
*                                     *
**************************************/

/**
 *  Check if this service should notify on critical states.
 *
 *  @return True if this service should notify on critical states.
 */
bool service::get_notify_on_critical() const {
  return (_notify_on_critical);
}

/**
 *  Set whether this service should notify on critical states.
 *
 *  @param[in] notify  True to notify.
 */
void service::set_notify_on_critical(bool notify) {
  _notify_on_critical = notify;
  return ;
}

/**
 *  Check if this service should notify on unknown states.
 *
 *  @return True if service should notify on unknown states.
 */
bool service::get_notify_on_unknown() const {
  return (_notify_on_unknown);
}

/**
 *  Set whether this service should notify on unknown states.
 *
 *  @param[in] notify  True to notify.
 */
void service::set_notify_on_unknown(bool notify) {
  _notify_on_unknown = notify;
  return ;
}

/**
 *  Check if this service should notify on warning states.
 *
 *  @return True if service should notify on warning states.
 */
bool service::get_notify_on_warning() const {
  return (_notify_on_warning);
}

/**
 *  Set whether this service should notify on warning states.
 *
 *  @param[in] notify  True to notify.
 */
void service::set_notify_on_warning(bool notify) {
  _notify_on_warning = notify;
  return ;
}

/**************************************
*                                     *
*           Private methods           *
*                                     *
**************************************/

/**
 *  Copy internal data members.
 *
 *  @param[in] other  Object to copy.
 */
void service::_internal_copy(service const& other) {
  _description = other._description;
  _host = other._host;
  _last_time_critical = other._last_time_critical;
  _last_time_ok = other._last_time_ok;
  _last_time_unknown = other._last_time_unknown;
  _last_time_warning = other._last_time_warning;
  _notify_on_critical = other._notify_on_critical;
  _notify_on_unknown = other._notify_on_unknown;
  _notify_on_warning = other._notify_on_warning;
  _stalk_on_critical = other._stalk_on_critical;
  _stalk_on_ok = other._stalk_on_ok;
  _stalk_on_unknown = other._stalk_on_unknown;
  _stalk_on_warning = other._stalk_on_warning;
  _volatile = other._volatile;
  return ;
}
