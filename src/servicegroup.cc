/*
** Copyright 2018 Centreon
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

#include "com/centreon/engine/service.hh"
#include "com/centreon/engine/servicegroup.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

/**
 *  Default constructor.
 */
servicegroup::servicegroup() {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
servicegroup::servicegroup(servicegroup const& other) {
  _internal_copy(other);
}

/**
 *  Destructor.
 */
servicegroup::~servicegroup() {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
servicegroup& servicegroup::operator=(servicegroup const& other) {
  if (this != &other)
    _internal_copy(other);
  return (*this);
}

std::string const& servicegroup::get_name() const {
  return _name;
}

void servicegroup::set_name(std::string const& name) {
  _name = name;
}

std::string const& servicegroup::get_alias() const {
  return _alias;
}

void servicegroup::set_alias(std::string const& alias) {
  _alias = alias;
}

std::string const& servicegroup::get_action_url() const {
  return _action_url;
}

void servicegroup::set_action_url(std::string const& action_url) {
  _action_url = action_url;
}

std::string const& servicegroup::get_notes_url() const {
  return _notes_url;
}

void servicegroup::set_notes_url(std::string const& notes_url) {
  _notes_url = notes_url;
}

std::string const& servicegroup::get_notes() const {
  return _notes;
}

void servicegroup::set_notes(std::string const& notes) {
  _notes = notes;
}

unsigned int servicegroup::get_id() const {
  return _id;
}

void servicegroup::set_id(unsigned int id) {
  _id = id;
}

void servicegroup::add_member(service* svc) {
  _members[std::make_pair(svc->get_host_name(), svc->get_description())] = svc;
}

void servicegroup::clear_members() {
  _members.clear();
}

umap<std::pair<std::string, std::string>, service*>& servicegroup::get_members() {
  return _members;
}

umap<std::pair<std::string, std::string>, service*> const& servicegroup::get_members() const {
  return _members;
}

void servicegroup::_internal_copy(servicegroup const& other) {
  _alias = other._alias;
  _name = other._name;
  _action_url = other._action_url;
  _notes_url = other._notes_url;
  _notes = other._notes;
  _id = other._id;
  _members = other._members;
}
