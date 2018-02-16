/*
** Copyright 2011-2013,2018 Centreon
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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/hostgroup.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/hostgroup.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       hostgroup const& obj1,
       hostgroup const& obj2) throw () {
  return (is_equal(obj1.group_name, obj2.group_name)
          && is_equal(obj1.alias, obj2.alias)
          && obj1.members == obj2.members
          //&& is_equal(obj1.members, obj2.members)
          && is_equal(obj1.notes, obj2.notes)
          && is_equal(obj1.notes_url, obj2.notes_url)
          && is_equal(obj1.action_url, obj2.action_url));
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool operator!=(
       hostgroup const& obj1,
       hostgroup const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump hostgroup content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The hostgroup to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, hostgroup const& obj) {
  os << "hostgroup {\n"
    "  group_name: " << chkstr(obj.group_name) << "\n"
    "  alias:      " << chkstr(obj.alias) << "\n"
    "  members:    " << chkobj(obj.members) << "\n"
    "  notes:      " << chkstr(obj.notes) << "\n"
    "  notes_url:  " << chkstr(obj.notes_url) << "\n"
    "  action_url: " << chkstr(obj.action_url) << "\n"
    "}\n";
  return (os);
}

/**
 *  Add a new host group to the list in memory.
 *
 *  @param[in] name       Host group name.
 *  @param[in] alias      Host group alias.
 *  @param[in] notes      Notes.
 *  @param[in] notes_url  URL.
 *  @param[in] action_url Action URL.
 *
 *  @return New host group.
 */
hostgroup* add_hostgroup(
             char const* name,
             char const* alias,
             char const* notes,
             char const* notes_url,
             char const* action_url) {
  // Make sure we have the data we need.
  if (!name || !name[0]) {
    logger(log_config_error, basic)
      << "Error: Hostgroup name is NULL";
    return (NULL);
  }

  // Check if the host group already exist.
  std::string id(name);
  if (is_hostgroup_exist(id)) {
    logger(log_config_error, basic)
      << "Error: Hostgroup '" << name << "' has already been defined";
    return (NULL);
  }

  // Allocate memory.
  shared_ptr<hostgroup> obj(new hostgroup, deleter::hostgroup);
  memset(obj.get(), 0, sizeof(hostgroup));

  try {
    // Duplicate vars.
    obj->group_name = string::dup(name);
    obj->alias = string::dup(alias ? alias : name);
    if (action_url)
      obj->action_url = string::dup(action_url);
    if (notes)
      obj->notes = string::dup(notes);
    if (notes_url)
      obj->notes_url = string::dup(notes_url);

    // Add new items to the configuration state.
    state::instance().hostgroups()[id] = obj;

    // Add new items to the list.
    obj->next = hostgroup_list;
    hostgroup_list = obj.get();
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Tests whether a host is a member of a particular hostgroup.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] group Target host group.
 *  @param[in] hst   Target host.
 *
 *  @return true or false.
 */
int is_host_member_of_hostgroup(hostgroup* group, host* hst) {
  if (!group || !hst)
    return (false);

  for (umap<std::string, shared_ptr<host> >::iterator
         it(group->members.begin()),
         end(group->members.end());
       it != end;
       ++it)
    if (it->second.get() == hst)
      return true;
  return false;
}

/**
 *  Get if hostgroup exist.
 *
 *  @param[in] name The hostgroup name.
 *
 *  @return True if the hostgroup is found, otherwise false.
 */
bool engine::is_hostgroup_exist(std::string const& name) throw () {
  umap<std::string, shared_ptr<hostgroup_struct> >::const_iterator
    it(state::instance().hostgroups().find(name));
  return (it != state::instance().hostgroups().end());
}

/**
 *  Add a new host to a host group.
 *
 *  @param[in] temp_hostgroup Host group object.
 *  @param[in] host_name      Host name.
 *
 *  @return Host group membership.
 */
host* com::centreon::engine::add_host_to_hostgroup(
                               hostgroup_struct* grp,
                               std::string const& host_name) {
  // Make sure we have the data we need.
  if (!grp || host_name.empty()) {
    logger(log_config_error, basic)
      << "Error: Hostgroup is NULL or host name is empty";
    return (NULL);
  }

  shared_ptr< ::host> obj;
  try {
    obj = configuration::applier::state::instance().hosts_find(host_name);
    grp->members[host_name] = obj;

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group_member(
      NEBTYPE_HOSTGROUPMEMBER_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      grp,
      &tv);
  }
  catch (...) {
    grp->members.erase(host_name);
    return (NULL);
  }

  return (obj.get());
}
