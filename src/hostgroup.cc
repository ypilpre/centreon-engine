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

#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/hostgroup.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

/**
 *  Default constructor.
 */
hostgroup::hostgroup() {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
hostgroup::hostgroup(hostgroup const& other) {
  _internal_copy(other);
}

/**
 *  Destructor.
 */
hostgroup::~hostgroup() {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
hostgroup& hostgroup::operator=(hostgroup const& other) {
  if (this != &other)
    _internal_copy(other);
  return (*this);
}

std::string const& hostgroup::get_name() const {
  return _name;
}

void hostgroup::set_name(std::string const& name) {
  _name = name;
}

std::string const& hostgroup::get_alias() const {
  return _alias;
}

void hostgroup::set_alias(std::string const& alias) {
  _alias = alias;
}

std::string const& hostgroup::get_action_url() const {
  return _action_url;
}

void hostgroup::set_action_url(std::string const& action_url) {
  _action_url = action_url;
}

std::string const& hostgroup::get_notes_url() const {
  return _notes_url;
}

void hostgroup::set_notes_url(std::string const& notes_url) {
  _notes_url = notes_url;
}

std::string const& hostgroup::get_notes() const {
  return _notes;
}

void hostgroup::set_notes(std::string const& notes) {
  _notes = notes;
}

unsigned int hostgroup::get_id() const {
  return _id;
}

void hostgroup::set_id(unsigned int id) {
  _id = id;
}

void hostgroup::add_member(host* hst) {
  _members[hst->get_name()] = hst;
}

void hostgroup::clear_members() {
  _members.clear();
}

umap<std::string, host*>& hostgroup::get_members() {
  return _members;
}

umap<std::string, host*> const& hostgroup::get_members() const {
  return _members;
}

void hostgroup::_internal_copy(hostgroup const& other) {
  _alias = other._alias;
  _name = other._name;
  _action_url = other._action_url;
  _notes_url = other._notes_url;
  _notes = other._notes;
  _id = other._id;
  _members = other._members;
}
