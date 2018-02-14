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

#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/macros/grab_service.hh"
#include "com/centreon/engine/macros/clear_service.hh"
#include "com/centreon/engine/macros/clear_servicegroup.hh"
#include "com/centreon/engine/service.hh"

using namespace com::centreon::engine;

/**
 *  Default constructor.
 */
service::service()
  : _host(NULL),
    _flap_detection_on_critical(false),
    _flap_detection_on_ok(false),
    _flap_detection_on_unknown(false),
    _flap_detection_on_warning(false),
    _last_time_critical(0),
    _last_time_ok(0),
    _last_time_unknown(0),
    _last_time_warning(0),
    _stalk_on_critical(false),
    _stalk_on_ok(false),
    _stalk_on_unknown(false),
    _stalk_on_warning(false),
    _volatile(false) {}

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
  if (this != &other) {
    monitorable::operator=(other);
    _internal_copy(other);
  }
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
 *  Set service description.
 *
 *  @param[in] description  New service description.
 */
void service::set_description(std::string const& description) {
  _description = description;
  return ;
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
 *  Set whether or not service should be stalked on critical state.
 *
 *  @param[in] stalk  True to stalk.
 */
void service::set_stalk_on_critical(bool stalk) {
  _stalk_on_critical = stalk;
  return ;
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
 *  Set whether or not service should be stalked on ok state.
 *
 *  @param[in] stalk  True to stalk.
 */
void service::set_stalk_on_ok(bool stalk) {
  _stalk_on_ok = stalk;
  return ;
}

/**
 *  Check if this service should be stalked on unknown state.
 *
 *  @return If this service should be stalked on unknown state.
 */
bool service::get_stalk_on_unknown() const {
  return (_stalk_on_unknown);
}

/**
 *  Set whether or not service should be stalked on unknown state.
 *
 *  @param[in] stalk  True to stalk.
 */
void service::set_stalk_on_unknown(bool stalk) {
  _stalk_on_unknown = stalk;
  return ;
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
 *  Set whether or not service should be stalked on warning state.
 *
 *  @param[in] stalk  True to stalk.
 */
void service::set_stalk_on_warning(bool stalk) {
  _stalk_on_warning = stalk;
  return ;
}

/**
 *  Check if this warning is volatile.
 *
 *  @return True if service is volatile.
 */
bool service::get_volatile() const {
  return (_volatile);
}

/**
 *  Mark service as volatile or not.
 *
 *  @param[in] is_volatile  True if service is volatile.
 */
void service::set_volatile(bool is_volatile) {
  _volatile = is_volatile;
  return ;
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
 *  Get service's host name.
 *
 *  @return Host name if service has an host. Empty otherwise.
 */
std::string service::get_host_name() const {
  return (_host ? _host->get_name() : "");
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
void service::add_group(servicegroup_struct* sg) {
  _groups.push_back(sg);
  return ;
}

/**
 *  Clear groups of this service.
 */
void service::clear_groups() {
  _groups.clear();
  return ;
}

/**
 *  Get groups of this service.
 *
 *  @return Groups of this service.
 */
servicegroup_set const& service::get_groups() const {
  return (_groups);
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
 *  Enable or not flap detection for OK state.
 *
 *  @param[in] detection  True to enable.
 */
void service::set_flap_detection_on_ok(bool detection) {
  _flap_detection_on_ok = detection;
  return ;
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
 *  Enable or not flap detection for WARNING state.
 *
 *  @param[in] detection  True to enable.
 */
void service::set_flap_detection_on_warning(bool detection) {
  _flap_detection_on_warning = detection;
  return ;
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
 *  Enable or not flap detection for UNKNOWN state.
 *
 *  @param[in] detection  True to enable.
 */
void service::set_flap_detection_on_unknown(bool detection) {
  _flap_detection_on_unknown = detection;
  return ;
}

/**
 *  Check if flap detection is enabled for CRITICAL state.
 *
 *  @return True if flap detection is enabled for CRITICAL state.
 */
bool service::get_flap_detection_on_critical() const {
  return (_flap_detection_on_critical);
}

/**
 *  Enable or not flap detection for CRITICAL state.
 *
 *  @param[in] detection  True to enable.
 */
void service::set_flap_detection_on_critical(bool detection) {
  _flap_detection_on_critical = detection;
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
  _flap_detection_on_critical = other._flap_detection_on_critical;
  _flap_detection_on_ok = other._flap_detection_on_ok;
  _flap_detection_on_unknown = other._flap_detection_on_unknown;
  _flap_detection_on_warning = other._flap_detection_on_warning;
  _last_time_critical = other._last_time_critical;
  _last_time_ok = other._last_time_ok;
  _last_time_unknown = other._last_time_unknown;
  _last_time_warning = other._last_time_warning;
  _stalk_on_critical = other._stalk_on_critical;
  _stalk_on_ok = other._stalk_on_ok;
  _stalk_on_unknown = other._stalk_on_unknown;
  _stalk_on_warning = other._stalk_on_warning;
  _volatile = other._volatile;
  return ;
}

void service::_checkable_macro_builder(nagios_macros& mac) {
  clear_service_macros_r(&mac);
  clear_servicegroup_macros_r(&mac);

  // Save pointer for later.
  mac.service_ptr = this;
  mac.servicegroup_ptr = NULL;
}

bool service::is_host() const {
  return false;
}
