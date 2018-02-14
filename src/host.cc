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
#include "com/centreon/engine/service.hh"

using namespace com::centreon::engine;

/**
 *  Default constructor.
 */
host::host()
  : _circular_path_checked(0),
    _flap_detection_on_down(false),
    _flap_detection_on_unreachable(false),
    _flap_detection_on_up(false),
    _have_2d_coords(false),
    _have_3d_coords(false),
    _last_historical_state_update(0),
    _last_time_down(0),
    _last_time_unreachable(0),
    _last_time_up(0),
    _should_reschedule_current_check(false),
    _stalk_on_down(false),
    _stalk_on_unreachable(false),
    _stalk_on_up(false),
    _total_service_check_interval(0),
    _x_2d(0),
    _y_2d(0),
    _x_3d(0),
    _y_3d(0),
    _z_3d(0) {}

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
  if (this != &other) {
    monitorable::operator=(other);
    _internal_copy(other);
  }
  return (*this);
}

/**************************************
*                                     *
*           Configuration             *
*                                     *
**************************************/

/**
 *  Get the address of the host.
 *
 *  @return A string representing the host's address.
 */
std::string const& host::get_address() const {
  return (_address);
}

/**
 *  Set the address of the host.
 *
 *  @param[in] address  New host address.
 */
void host::set_address(std::string const& address) {
  _address = address;
  return ;
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
 *  Set the alias.
 *
 *  @param[in] alias  New alias.
 */
void host::set_alias(std::string const& alias) {
  _alias = alias;
  return ;
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
 *  Get host name.
 *
 *  @return Host name.
 */
std::string const& host::get_name() const {
  return (_name);
}

/**
 *  Set host name.
 *
 *  @param[in] name  New host name.
 */
void host::set_name(std::string const& name) {
  _name = name;
  return ;
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
 *  Set whether host should be stalked on down states.
 *
 *  @param[in] stalk  True if host should be stalked.
 */
void host::set_stalk_on_down(bool stalk) {
  _stalk_on_down = stalk;
  return ;
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
 *  Set whether host should be stalked on unreachable states.
 *
 *  @param[in] stalk  True if host should be stalked.
 */
void host::set_stalk_on_unreachable(bool stalk) {
  _stalk_on_unreachable = stalk;
  return ;
}

/**
 *  Check if host should be stalked on up states.
 *
 *  @return True if host should be stalked.
 */
bool host::get_stalk_on_up() const {
  return (_stalk_on_up);
}

/**
 *  Set whether host should be stalked on up states.
 *
 *  @param[in] stalk  True if host should be stalked.
 */
void host::set_stalk_on_up(bool stalk) {
  _stalk_on_up = stalk;
  return ;
}

/**
 *  Get status map image.
 *
 *  @return Status map image.
 */
std::string const& host::get_statusmap_image() const {
  return (_statusmap_image);
}

/**
 *  Set status map image.
 *
 *  @param[in] image  Status map image.
 */
void host::set_statusmap_image(std::string const& image) {
  _statusmap_image = image;
  return ;
}

/**
 *  Get VRML image.
 *
 *  @return VRML image.
 */
std::string const& host::get_vrml_image() const {
  return (_vrml_image);
}

/**
 *  Set VRML image.
 *
 *  @param[in] image  VRML image.
 */
void host::set_vrml_image(std::string const& image) {
  _vrml_image = image;
  return ;
}

/**************************************
*                                     *
*             Coordinates             *
*                                     *
**************************************/

/**
 *  Check if host has 2D coordinates.
 *
 *  @return True if host has 2D coordinates.
 */
bool host::get_have_2d_coords() const {
  return (_have_2d_coords);
}

/**
 *  Set if host has 2D coordinates.
 *
 *  @param[in] have_coords  True if host has 2D coordinates.
 */
void host::set_have_2d_coords(bool have_coords) {
  _have_2d_coords = have_coords;
}

/**
 *  Check if host has 3D coordinates.
 *
 *  @return True if host has 3D coordinates.
 */
bool host::get_have_3d_coords() const {
  return (_have_3d_coords);
}

/**
 *  Set if host has 3D coordinates.
 *
 *  @param[in] have_coords  True if host has 3D coordinates.
 */
void host::set_have_3d_coords(bool have_coords) {
  _have_3d_coords = have_coords;
  return ;
}

/**
 *  Get x coordinate (2D).
 *
 *  @return X coordinate.
 */
int host::get_x_2d() const {
  return (_x_2d);
}

/**
 *  Set x coordinate (2D).
 *
 *  @param[in] x_2d  X coordinate.
 */
void host::set_x_2d(int x) {
  _x_2d = x;
  return ;
}

/**
 *  Get y coordinate (2D).
 *
 *  @return Y coordinate.
 */
int host::get_y_2d() const {
  return (_y_2d);
}

/**
 *  Set y coordinate (2D).
 *
 *  @param[in] y  Y coordinate.
 */
void host::set_y_2d(int y) {
  _y_2d = y;
  return ;
}

/**
 *  Get x coordinate (3D).
 *
 *  @return X coordinate.
 */
int host::get_x_3d() const {
  return (_x_3d);
}

/**
 *  Set x coordinate (3D).
 *
 *  @param[in] x  X coordinate.
 */
void host::set_x_3d(int x) {
  _x_3d = x;
  return ;
}

/**
 *  Get y coordinate (3D).
 *
 *  @return Y coordinate.
 */
int host::get_y_3d() const {
  return (_y_3d);
}

/**
 *  Set y coordinate (3D).
 *
 *  @param[in] y  Y coordinate.
 */
void host::set_y_3d(int y) {
  _y_3d = y;
  return ;
}

/**
 *  Get z coordinate (3D).
 *
 *  @return Z coordinate.
 */
int host::get_z_3d() const {
  return (_z_3d);
}

/**
 *  Set z coordinate (3D).
 *
 *  @param[in] z  Z coordinate.
 */
void host::set_z_3d(int z) {
  _z_3d = z;
  return ;
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
void host::add_child(host* hst) {
  _children.push_back(hst);
  return ;
}

/**
 *  Clear child list of this host.
 */
void host::clear_children() {
  _children.clear();
  return ;
}

/**
 *  Get children.
 *
 *  @return List of children.
 */
std::list<host*> const& host::get_children() const {
  return (_children);
}

/**
 *  Add host group to this host.
 *
 *  @param[in] hg  Host group.
 */
void host::add_group(hostgroup_struct* hg) {
  _groups.push_back(hg);
  return ;
}

/**
 *  Clear group list of this host.
 */
void host::clear_groups() {
  _groups.clear();
  return ;
}

/**
 *  Get groups of this host.
 *
 *  @return Groups of this host.
 */
hostgroup_set const& host::get_groups() const {
  return (_groups);
}

/**
 *  Add a parent to this host.
 *
 *  @param[in] hst  New parent.
 */
void host::add_parent(host* hst) {
  _parents.push_back(hst);
  return ;
}

/**
 *  Clear parent list of this host.
 */
void host::clear_parents() {
  _parents.clear();
  return ;
}

/**
 *  Get parents.
 *
 *  @return List of parents.
 */
std::list<host*> const& host::get_parents() const {
  return (_parents);
}

/**
 *  Add a service to this host.
 *
 *  @param[in] svc  Service.
 */
void host::add_service(service* svc) {
  _services.push_back(svc);
  _total_service_check_interval += svc->get_normal_check_interval();
  return ;
}

/**
 *  Clear the service list of this host.
 */
void host::clear_services() {
  _services.clear();
  _total_service_check_interval = 0;
  return ;
}

/**
 *  Get the list of services of this host.
 *
 *  @return List of services of this host.
 */
std::list<service*> const& host::get_services() const {
  return (_services);
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
 *  Enable or disable flap detection for UP state.
 *
 *  @param[in] detection  True to enable flap detection.
 */
void host::set_flap_detection_on_up(bool detection) {
  _flap_detection_on_up = detection;
  return ;
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
 *  Enable or disable flap detection for DOWN state.
 *
 *  @param[in] detection  True to enable flap detection.
 */
void host::set_flap_detection_on_down(bool detection) {
  _flap_detection_on_down = detection;
  return ;
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
 *  Enable or disable flap detection for UNREACHABLE state.
 *
 *  @param[in] detection  True to enable flap detection.
 */
void host::set_flap_detection_on_unreachable(bool detection) {
  _flap_detection_on_unreachable = detection;
  return ;
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
*          Private methods            *
*                                     *
**************************************/

/**
 *  Copy internal data members.
 *
 *  @param[in] other  Object to copy.
 */
void host::_internal_copy(host const& other) {
  _address = other._address;
  _alias = other._alias;
  _children = other._children;
  _circular_path_checked = other._circular_path_checked;
  _flap_detection_on_down = other._flap_detection_on_down;
  _flap_detection_on_unreachable = other._flap_detection_on_unreachable;
  _flap_detection_on_up = other._flap_detection_on_up;
  _have_2d_coords = other._have_2d_coords;
  _have_3d_coords = other._have_3d_coords;
  _last_historical_state_update = other._last_historical_state_update;
  _last_time_down = other._last_time_down;
  _last_time_unreachable = other._last_time_unreachable;
  _last_time_up = other._last_time_up;
  _name = other._name;
  _parents = other._parents;
  _services = other._services;
  _should_reschedule_current_check = other._should_reschedule_current_check;
  _stalk_on_down = other._stalk_on_down;
  _stalk_on_unreachable = other._stalk_on_unreachable;
  _stalk_on_up = other._stalk_on_up;
  _statusmap_image = other._statusmap_image;
  _total_service_check_interval = other._total_service_check_interval;
  _vrml_image = other._vrml_image;
  _x_2d = other._x_2d;
  _y_2d = other._y_2d;
  _x_3d = other._x_3d;
  _y_3d = other._y_3d;
  _z_3d = other._z_3d;
  return ;
}

void host::_checkable_macro_builder(nagios_macros& mac) {
  // Save pointer to host.
  mac.host_ptr = this;
  mac.hostgroup_ptr = NULL;
}

bool host::is_host() const {
  return true;
}
