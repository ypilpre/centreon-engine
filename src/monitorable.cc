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

#include "com/centreon/engine/monitorable.hh"

using namespace com::centreon::engine;

/**
 *  Default constructor.
 */
monitorable::monitorable()
  : _id(0),
    _initial_state(0),
    _retain_nonstate_info(true),
    _retain_state_info(true) {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
monitorable::monitorable(monitorable const& other)
  : notifications::notifier(other) {
  _internal_copy(other);
}

/**
 *  Destructor.
 */
monitorable::~monitorable() {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
monitorable& monitorable::operator=(monitorable const& other) {
  if (this != &other) {
    notifications::notifier::operator=(other);
    _internal_copy(other);
  }
  return (*this);
}

/**
 *  Get action URL.
 *
 *  @return Action URL.
 */
std::string const& monitorable::get_action_url() const {
  return (_action_url);
}

/**
 *  Set action URL.
 *
 *  @param[in] action_url  New action URL.
 */
void monitorable::set_action_url(std::string const& action_url) {
  _action_url = action_url;
  return ;
}

/**
 *  Clear custom variables.
 */
void monitorable::clear_customvars() {
  _vars.clear();
  return ;
}

/**
 *  Get custom variables.
 *
 *  @return Set of this object's custom variables.
 */
customvar_set const& monitorable::get_customvars() const {
  return (_vars);
}

/**
 *  Set custom variable.
 *
 *  @param[in] var  New custom variable.
 */
void monitorable::set_customvar(customvar const& var) {
  _vars[var.get_name()] = var;
  return ;
}

/**
 *  Get display name.
 *
 *  @return Display name.
 */
std::string const& monitorable::get_display_name() const {
  return (_display_name);
}

/**
 *  Set display name.
 *
 *  @param[in] display  New display name.
 */
void monitorable::set_display_name(std::string const& display) {
  _display_name = display;
  return ;
}

/**
 *  Get host name.
 *
 *  @return This object's host name.
 */
std::string const& monitorable::get_host_name() const {
  return (_host_name);
}

/**
 *  Get icon image.
 *
 *  @return Icon image.
 */
std::string const& monitorable::get_icon_image() const {
  return (_icon_image);
}

/**
 *  Set icon image.
 *
 *  @param[in] image  New image.
 */
void monitorable::set_icon_image(std::string const& image) {
  _icon_image = image;
  return ;
}

/**
 *  Get alternative icon image.
 *
 *  @return Alternative icon image.
 */
std::string const& monitorable::get_icon_image_alt() const {
  return (_icon_image_alt);
}

/**
 *  Set alternative icon image.
 *
 *  @param[in] image  New image.
 */
void monitorable::set_icon_image_alt(std::string const& image) {
  _icon_image_alt = image;
  return ;
}

/**
 *  Get object ID.
 *
 *  @return Object ID.
 */
unsigned int monitorable::get_id() const {
  return (_id);
}

/**
 *  Set object ID.
 *
 *  @param[in] id  Object ID.
 */
void monitorable::set_id(unsigned int id) {
  _id = id;
  return ;
}

/**
 *  Get initial state.
 *
 *  @return Initial state.
 */
int monitorable::get_initial_state() const {
  return (_initial_state);
}

/**
 *  Set initial state.
 *
 *  @param[in] state  New initial state.
 */
void monitorable::set_initial_state(int state) {
  _initial_state = state;
  return ;
}

/**
 *  Get notes.
 *
 *  @return Notes.
 */
std::string const& monitorable::get_notes() const {
  return (_notes);
}

/**
 *  Set notes.
 *
 *  @param[in] notes  New notes.
 */
void monitorable::set_notes(std::string const& notes) {
  _notes = notes;
  return ;
}

/**
 *  Get notes URL.
 *
 *  @return Notes URL.
 */
std::string const& monitorable::get_notes_url() const {
  return (_notes_url);
}

/**
 *  Set notes URL.
 *
 *  @param[in] notes_url  New notes URL.
 */
void monitorable::set_notes_url(std::string const& notes_url) {
  _notes_url = notes_url;
  return ;
}

/**
 *  Check if non-state retention is enabled for this object.
 *
 *  @return True if non-state retention is enabled.
 */
bool monitorable::get_retain_nonstate_info() const {
  return (_retain_nonstate_info);
}

/**
 *  Enable or disable non-state retention for this object.
 *
 *  @param[in] retain  True to retain non-state information.
 */
void monitorable::set_retain_nonstate_info(bool retain) {
  _retain_nonstate_info = retain;
  return ;
}

/**
 *  Check if state retention is enabled for this object.
 *
 *  @return True if state retention is enabled.
 */
bool monitorable::get_retain_state_info() const {
  return (_retain_state_info);
}

/**
 *  Enable or disable state retention for this object.
 *
 *  @param[in] retain  True to retain state information.
 */
void monitorable::set_retain_state_info(bool retain) {
  _retain_state_info = retain;
  return ;
}

/**
 *  Copy internal data members.
 *
 *  @param[in] other  Object to copy.
 */
void monitorable::_internal_copy(monitorable const& other) {
  _action_url = other._action_url;
  _display_name = other._display_name;
  _host_name = other._host_name;
  _icon_image = other._icon_image;
  _icon_image_alt = other._icon_image_alt;
  _id = other._id;
  _initial_state = other._initial_state;
  _notes = other._notes;
  _notes_url = other._notes_url;
  _retain_nonstate_info = other._retain_nonstate_info;
  _retain_state_info = other._retain_state_info;
  _vars = other._vars;
  return ;
}
