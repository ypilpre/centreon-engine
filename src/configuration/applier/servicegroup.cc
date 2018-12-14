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
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/servicegroup.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/not_found.hh"
#include "com/centreon/engine/servicegroup.hh"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::servicegroup::servicegroup() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::servicegroup::servicegroup(
                         applier::servicegroup const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::servicegroup::~servicegroup() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::servicegroup& applier::servicegroup::operator=(
                         applier::servicegroup const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new servicegroup.
 *
 *  @param[in] obj  The new servicegroup to add into the monitoring
 *                  engine.
 */
void applier::servicegroup::add_object(
                              configuration::servicegroup const& obj) {
  std::string const& name(obj.servicegroup_name());
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new servicegroup '" << obj.servicegroup_name() << "'.";

  // Check if the service group already exists.
  umap<std::string, shared_ptr<engine::servicegroup> >::const_iterator
    it(applier::state::instance().servicegroups().find(name));
  if (it != configuration::applier::state::instance().servicegroups().end())
    throw (engine_error() << "Servicegroup '" << name
           << "' has already been defined");

  // Add service group to the global configuration set.
  config->servicegroups().insert(obj);

  // Create service group.
  shared_ptr<engine::servicegroup>
    sg(new engine::servicegroup());

  // Self properties.
  sg->set_name(name);
  if (!obj.alias().empty())
    sg->set_alias(obj.alias());
  else
    sg->set_alias(name);

  sg->set_notes(obj.notes());
  sg->set_notes_url(obj.notes_url());
  sg->set_action_url(obj.action_url());
  sg->set_id(obj.servicegroup_id());

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_group(
    NEBTYPE_SERVICEGROUP_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    static_cast<void*>(sg.get()),
    &tv);

  // Add new items to the configurations state.
  applier::state::instance().servicegroups().insert(
    std::make_pair(name, sg));
//  // Apply resolved services on servicegroup.
//  for (set_pair_string::const_iterator
//         it(obj.members().begin()),
//         end(obj.members().end());
//       it != end;
//       ++it)
//    if (!add_service_to_servicegroup(
//           sg,
//           it->first.c_str(),
//           it->second.c_str()))
//      throw (engine_error() << "Could not add service member '"
//             << it->second << "' of host '" << it->first
//             << "' to service group '" << obj.servicegroup_name()
//             << "'");
//
//  return ;
}

/**
 *  Expand all service groups.
 *
 *  @param[in,out] s  State being applied.
 */
void applier::servicegroup::expand_objects(configuration::state& s) {
  // Resolve groups.
  _resolved.clear();
  for (configuration::set_servicegroup::const_iterator
         it(s.servicegroups().begin()),
         end(s.servicegroups().end());
       it != end;
       ++it)
    _resolve_members(*it, s);

  // Save resolved groups in the configuration set.
  s.servicegroups().clear();
  for (resolved_set::const_iterator
         it(_resolved.begin()),
         end(_resolved.end());
       it != end;
       ++it)
    s.servicegroups().insert(it->second);

  return ;
}

/**
 *  Modify servicegroup.
 *
 *  @param[in] obj  The new servicegroup to modify into the monitoring
 *                  engine.
 */
void applier::servicegroup::modify_object(
                              configuration::servicegroup const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying servicegroup '" << obj.servicegroup_name() << "'";

  // Find old configuration.
  set_servicegroup::iterator
    it_cfg(config->servicegroups_find(obj.key()));
  if (it_cfg == config->servicegroups().end())
    throw (engine_error() << "Could not modify non-existing "
           << "service group '" << obj.servicegroup_name() << "'");

  // Find service group object.
  umap<std::string, shared_ptr<engine::servicegroup> >::iterator
    it_obj(applier::state::instance().servicegroups().find(obj.key()));
  if (it_obj == applier::state::instance().servicegroups().end())
    throw (engine_error() << "Could not modify non-existing "
           << "service group object '" << obj.servicegroup_name()
           << "'");
  engine::servicegroup* sg(it_obj->second.get());

  // Update the global configuration set.
  configuration::servicegroup old_cfg(*it_cfg);
  config->servicegroups().erase(it_cfg);
  config->servicegroups().insert(obj);

  // Modify properties.
  modify_if_different(*sg, action_url, obj.action_url());
  modify_if_different(*sg, alias, obj.alias());
  modify_if_different(*sg, notes, obj.notes());
  modify_if_different(*sg, notes_url, obj.notes_url());
  modify_if_different(*sg, id, obj.servicegroup_id());

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_group(
    NEBTYPE_SERVICEGROUP_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    sg,
    &tv);

  return ;
}

/**
 *  Remove old servicegroup.
 *
 *  @param[in] obj  The new servicegroup to remove from the monitoring
 *                  engine.
 */
void applier::servicegroup::remove_object(
                              configuration::servicegroup const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing service group '" << obj.servicegroup_name() << "'";

  // Find service group.
  umap<std::string, shared_ptr<engine::servicegroup> >::iterator
    it(applier::state::instance().servicegroups().find(obj.key()));
  if (it != applier::state::instance().servicegroups().end()) {
    engine::servicegroup* grp(it->second.get());

    // Remove service dependency from its list.
    // FIXME DBR
    //unregister_object<engine::servicegroup>(&servicegroup_list, grp);

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group(
      NEBTYPE_SERVICEGROUP_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      grp,
      &tv);

    // Erase service group object (will effectively delete the object).
    applier::state::instance().servicegroups().erase(it);
  }

  // Remove service group from the global configuration state.
  config->servicegroups().erase(obj);

  return ;
}

/**
 *  Resolve a servicegroup.
 *
 *  @param[in,out] obj  Servicegroup object.
 */
void applier::servicegroup::resolve_object(
                              configuration::servicegroup const& obj) {
  // Failure flag.
  bool failure(false);

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving service group '" << obj.servicegroup_name() << "'";

  try {
    // Find service group.
    engine::servicegroup& sg(
      *applier::state::instance().servicegroups_find(obj.key()).get());

    // Check for illegal characters in servicegroup name.
    if (contains_illegal_object_chars(sg.get_name().c_str())) {
      logger(log_verification_error, basic)
        << "Error: The name of servicegroup '" << sg.get_name()
        << "' contains one or more illegal characters.";
      ++config_errors;
      failure = true;
    }

    // Remove old links.
    for (umap<std::pair<std::string, std::string>, engine::service*>::iterator
           it(sg.get_members().begin()),
           end(sg.get_members().end());
         it != end;
         ++it) {
      timeval tv(get_broker_timestamp(NULL));
      broker_group_member(
        NEBTYPE_SERVICEGROUPMEMBER_DELETE,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        it->second,
        &sg,
        &tv);
    }
    sg.clear_members();

    // Check all group members.
    for (set_pair_string::const_iterator
           it(obj.members().begin()),
           end(obj.members().end());
         it != end;
         ++it) {
      try {
        sg.add_member(applier::state::instance().services_find(*it).get());
      }
      catch (not_found const& e) {
        (void)e;
        logger(logging::log_verification_error, logging::basic)
          << "Error: Member '" << it->first << "/" << it->second
          << "' of service group '" << sg.get_name()
          << "' is not defined anywhere!";
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
           << "service group '" << obj.servicegroup_name() << "'");
  }
}

/**
 *  Do nothing.
 */
void applier::servicegroup::unresolve_objects() {
  return ;
}

/**
 *  Resolve members of a service group.
 *
 *  @param[in,out] obj  Service group object.
 *  @param[in]     s    Configuration being applied.
 */
void applier::servicegroup::_resolve_members(
                              configuration::servicegroup const& obj,
                              configuration::state const& s) {
  // Only process if servicegroup has not been resolved already.
  if (_resolved.find(obj.key()) == _resolved.end()) {
    // Logging.
    logger(logging::dbg_config, logging::more)
      << "Resolving members of service group '"
      << obj.servicegroup_name() << "'";

    // Mark object as resolved.
    configuration::servicegroup& resolved_obj(_resolved[obj.key()]);

    // Insert base members.
    resolved_obj = obj;
    resolved_obj.servicegroup_members().clear();

    // Add servicegroup members.
    for (set_string::const_iterator
           it(obj.servicegroup_members().begin()),
           end(obj.servicegroup_members().end());
         it != end;
         ++it) {
      // Find servicegroup entry.
      set_servicegroup::iterator it2(s.servicegroups_find(*it));
      if (it2 == s.servicegroups().end())
        throw (engine_error()
               << "Could not add non-existing service group member '"
               << *it << "' to service group '"
               << obj.servicegroup_name() << "'");

      // Resolve servicegroup member.
      _resolve_members(*it2, s);

      // Add servicegroup member members to members.
      for (set_pair_string::const_iterator
             it3(it2->members().begin()),
             end3(it2->members().end());
           it3 != end3;
           ++it3)
        resolved_obj.members().insert(*it3);
    }
  }

  return ;
}
