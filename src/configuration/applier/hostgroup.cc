/*
** Copyright 2011-2013,2015,2017-2018 Centreon
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
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/hostgroup.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/hostgroup.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/not_found.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::hostgroup::hostgroup() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::hostgroup::hostgroup(applier::hostgroup const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::hostgroup::~hostgroup() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::hostgroup& applier::hostgroup::operator=(
                      applier::hostgroup const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new hostgroup.
 *
 *  @param[in] obj  The new hostgroup to add into the monitoring engine.
 */
void applier::hostgroup::add_object(
                           configuration::hostgroup const& obj) {
  std::string const& name(obj.hostgroup_name());
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new hostgroup '" << obj.hostgroup_name() << "'.";

  // Check if the host group already exists.
  umap<std::string, shared_ptr<engine::hostgroup> >::const_iterator
    it(applier::state::instance().hostgroups().find(name));
  if (it != configuration::applier::state::instance().hostgroups().end())
    throw (engine_error() << "Hostgroup '" << name
           << "' has already been defined");

  // Add host group to the global configuration set.
  config->hostgroups().insert(obj);

  // Create host group.
  shared_ptr<engine::hostgroup>
    hg(new engine::hostgroup());

  // Self properties.
  hg->set_name(name);
  if (!obj.alias().empty())
    hg->set_alias(obj.alias());
  else
    hg->set_alias(name);

  hg->set_notes(obj.notes());
  hg->set_notes_url(obj.notes_url());
  hg->set_action_url(obj.action_url());
  hg->set_id(obj.hostgroup_id());

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_group(
    NEBTYPE_HOSTGROUP_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    static_cast<void*>(hg.get()),
    &tv);

  // Add new items to the configuration state.
  applier::state::instance().hostgroups().insert(
    std::make_pair(name, hg));
}

/**
 *  Expand all host groups.
 *
 *  @param[in,out] s  State being applied.
 */
void applier::hostgroup::expand_objects(configuration::state& s) {
  // Resolve groups.
  _resolved.clear();
  for (configuration::set_hostgroup::const_iterator
         it(s.hostgroups().begin()),
         end(s.hostgroups().end());
       it != end;
       ++it)
    _resolve_members(s, *it);

  // Save resolved groups in the configuration set.
  s.hostgroups().clear();
  for (resolved_set::const_iterator
         it(_resolved.begin()),
         end(_resolved.end());
       it != end;
       ++it)
    s.hostgroups().insert(it->second);
}

/**
 *  Modified hostgroup.
 *
 *  @param[in] obj  The new hostgroup to modify into the monitoring
 *                  engine.
 */
void applier::hostgroup::modify_object(
                           configuration::hostgroup const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying hostgroup '" << obj.hostgroup_name() << "'";

  // Find old configuration.
  set_hostgroup::iterator
    it_cfg(config->hostgroups_find(obj.key()));
  if (it_cfg == config->hostgroups().end())
    throw (engine_error() << "Could not modify non-existing "
           << "host group '" << obj.hostgroup_name() << "'");

  // Find host group object.
  umap<std::string, shared_ptr<engine::hostgroup> >::iterator
    it_obj(applier::state::instance().hostgroups().find(obj.key()));
  if (it_obj == applier::state::instance().hostgroups().end())
    throw (engine_error() << "Error: Could not modify non-existing "
           << "host group object '" << obj.hostgroup_name() << "'");
  engine::hostgroup* hg(it_obj->second.get());

  // Update the global configuration set.
  configuration::hostgroup old_cfg(*it_cfg);
  config->hostgroups().erase(it_cfg);
  config->hostgroups().insert(obj);

  // Modify properties.
  modify_if_different(*hg, action_url, obj.action_url());
  modify_if_different(*hg, alias, obj.alias());
  modify_if_different(*hg, notes, obj.notes());
  modify_if_different(*hg, notes_url, obj.notes_url());
  modify_if_different(*hg, id, obj.hostgroup_id());

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_group(
    NEBTYPE_HOSTGROUP_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hg,
    &tv);

  return ;
}

/**
 *  Remove old hostgroup.
 *
 *  @param[in] obj  The new hostgroup to remove from the monitoring
 *                  engine.
 */
void applier::hostgroup::remove_object(
                           configuration::hostgroup const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing host group '" << obj.hostgroup_name() << "'";

  // Find host group.
  umap<std::string, shared_ptr<engine::hostgroup> >::iterator
    it(applier::state::instance().hostgroups().find(obj.key()));
  if (it != applier::state::instance().hostgroups().end()) {
    engine::hostgroup* grp(it->second.get());

    // Remove host group from its list.
    //FIXME DBR
    //unregister_object<engine::hostgroup>(&hostgroup_list, grp);

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group(
      NEBTYPE_HOSTGROUP_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      grp,
      &tv);

    // Erase host group object (will effectively delete the object).
    applier::state::instance().hostgroups().erase(it);
  }

  // Remove host group from the global configuration set.
  config->hostgroups().erase(obj);
}

/**
 *  Resolve a host group.
 *
 *  @param[in] obj  Object to resolved.
 */
void applier::hostgroup::resolve_object(
                           configuration::hostgroup const& obj) {
  // Failure flag.
  bool failure(false);

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving host group '" << obj.hostgroup_name() << "'";

  try {
    // Find host group.
    engine::hostgroup& hg(
      *applier::state::instance().hostgroups_find(obj.key()).get());

    // Check for illegal characters in hostgroup name.
    if (contains_illegal_object_chars(hg.get_name().c_str())) {
      logger(log_verification_error, basic)
        << "Error: The name of hostgroup '" << hg.get_name()
        << "' contains one or more illegal characters.";
      ++config_errors;
      failure = true;
    }

    // Remove old links.
    for (umap<std::string, engine::host*>::iterator
           it(hg.get_members().begin()),
           end(hg.get_members().end());
         it != end;
         ++it) {
      timeval tv(get_broker_timestamp(NULL));
      broker_group_member(
        NEBTYPE_HOSTGROUPMEMBER_DELETE,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        it->second,
        &hg,
        &tv);
    }
    hg.clear_members();

    // Check all group members.
    for (set_string::const_iterator
           it(obj.members().begin()),
           end(obj.members().end());
         it != end;
         ++it) {
      try {
        hg.add_member(applier::state::instance().hosts_find(*it).get());
      }
      catch (not_found const& e) {
        (void)e;
        logger(logging::log_verification_error, logging::basic)
          << "Error: Member '" << *it << "' of host group '"
          << hg.get_name() << "' is not defined anywhere!";
        ++config_errors;
        failure = true;
      }
    }

    // Throw exception in case of failure.
    if (failure)
      throw (error() << "please check logs above");
  }
  catch (std::exception const& e) {
    throw (engine_error() << "Could not resolve non-existing "
           << "host group '" << obj.hostgroup_name() << "'");
  }
}

/**
 *  Do nothing.
 */
void applier::hostgroup::unresolve_objects() {
  return ;
}

/**
 *  Resolve members of a host group.
 *
 *  @param[in]     s    Configuration being applied.
 *  @param[in,out] obj  Hostgroup object.
 */
void applier::hostgroup::_resolve_members(
                           configuration::state& s,
                           configuration::hostgroup const& obj) {
  // Only process if hostgroup has not been resolved already.
  if (_resolved.find(obj.key()) == _resolved.end()) {
    // Logging.
    logger(logging::dbg_config, logging::more)
      << "Resolving members of host group '"
      << obj.hostgroup_name() << "'";

    // Mark object as resolved.
    configuration::hostgroup& resolved_obj(_resolved[obj.key()]);

    // Insert base members.
    resolved_obj = obj;
    resolved_obj.hostgroup_members().clear();

    // Add hostgroup members.
    for (set_string::const_iterator
           it(obj.hostgroup_members().begin()),
           end(obj.hostgroup_members().end());
         it != end;
         ++it) {
      // Find hostgroup entry.
      set_hostgroup::iterator it2(s.hostgroups_find(*it));
      if (it2 == s.hostgroups().end())
        throw (engine_error()
               << "Error: Could not add non-existing host group member '"
               << *it << "' to host group '"
               << obj.hostgroup_name() << "'");

      // Resolve hostgroup member.
      _resolve_members(s, *it2);

      // Add hostgroup member members to members.
      configuration::hostgroup& resolved_group(_resolved[*it]);
      resolved_obj.members().insert(
                               resolved_group.members().begin(),
                               resolved_group.members().end());
    }
  }

  return ;
}
