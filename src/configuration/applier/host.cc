/*
** Copyright 2011-2018 Centreon
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

#include <algorithm>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/member.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/scheduler.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/not_found.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;
using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::host::host() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::host::host(applier::host const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::host::~host() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::host& applier::host::operator=(applier::host const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new host.
 *
 *  @param[in] obj  The new host to add into the monitoring engine.
 */
void applier::host::add_object(
                      configuration::host const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new host '" << obj.host_name() << "'.";

  // Create host.
  shared_ptr< ::host> h;
  try {
    h = new ::host();
    // Self properties.
    h->set_address(obj.address());
    h->set_alias(obj.alias());
    h->set_flap_detection_on_down(
      obj.flap_detection_options() & configuration::host::down);
    h->set_flap_detection_on_unreachable(
      obj.flap_detection_options() & configuration::host::unreachable);
    h->set_flap_detection_on_up(
      obj.flap_detection_options() & configuration::host::up);
    h->set_have_2d_coords(obj.have_coords_2d());
    h->set_have_3d_coords(obj.have_coords_3d());
    h->set_stalk_on_down(
      obj.stalking_options() & configuration::host::down);
    h->set_stalk_on_unreachable(
      obj.stalking_options() & configuration::host::unreachable);
    h->set_stalk_on_up(
      obj.stalking_options() & configuration::host::up);
    h->set_statusmap_image(obj.statusmap_image());
    h->set_vrml_image(obj.vrml_image());
    h->set_x_2d(obj.coords_2d().x());
    h->set_y_2d(obj.coords_2d().y());
    h->set_x_3d(obj.coords_3d().x());
    h->set_y_3d(obj.coords_3d().y());
    h->set_z_3d(obj.coords_3d().z());
    // Inherited from monitorable.
    h->set_action_url(obj.action_url());
    h->set_display_name(obj.display_name());
    h->set_icon_image(obj.icon_image());
    h->set_icon_image_alt(obj.icon_image_alt());
    h->set_id(obj.host_id());
    h->set_initial_state(obj.initial_state());
    h->set_name(obj.host_name());
    h->set_notes(obj.notes());
    h->set_notes_url(obj.notes_url());
    h->set_retain_nonstate_info(obj.retain_nonstatus_information());
    h->set_retain_state_info(obj.retain_status_information());
    for (map_customvar::const_iterator
           it(obj.customvariables().begin()),
           end(obj.customvariables().end());
         it != end;
         ++it)
      h->set_customvar(customvar(it->first, it->second));
    // Inherited from notifier.
    h->set_acknowledgement_timeout(obj.get_acknowledgement_timeout());
    h->set_notifications_enabled(obj.notifications_enabled());
    h->set_notify_on(
      ::host::ON_DOWNTIME,
      obj.notification_options() & configuration::host::downtime);
    h->set_notify_on(
      ::host::ON_FLAPPING,
      obj.notification_options() & configuration::host::flapping);
    h->set_notify_on(
      ::host::ON_RECOVERY,
      obj.notification_options() & configuration::host::up);
    h->set_notify_on(
      ::host::ON_UNREACHABLE,
      obj.notification_options() & configuration::host::unreachable);
    h->set_notification_interval(obj.notification_interval());
    h->set_first_notification_delay(obj.first_notification_delay());
    h->set_recovery_notification_delay(obj.recovery_notification_delay());
    // Inherited from checkable.
    h->set_active_checks_enabled(obj.checks_active());
    h->set_event_handler_enabled(obj.event_handler_enabled());
    h->set_flap_detection_enabled(obj.flap_detection_enabled());
    h->set_freshness_checks_enabled(obj.check_freshness());
    h->set_freshness_threshold(obj.freshness_threshold());
    h->set_high_flap_threshold(obj.high_flap_threshold());
    h->set_low_flap_threshold(obj.low_flap_threshold());
    h->set_max_attempts(obj.max_check_attempts());
    h->set_normal_check_interval(obj.check_interval());
    h->set_ocp_enabled(obj.obsess_over_host());
    h->set_passive_checks_enabled(obj.checks_passive());
    h->set_process_perfdata(obj.process_perf_data());
    h->set_retry_check_interval(obj.retry_interval());
    h->set_timezone(obj.timezone());

    // Add host to global configuration set.
    config->hosts().insert(obj);
  }
  catch (std::exception const& e) {
    logger(logging::log_config_error, logging::basic)
      << "Error: " << e.what();
    throw (engine_error()
           << "Could not register host '" << obj.host_name() << "'");
  }

  // Add new items to the configuration state.
  configuration::applier::state::instance().hosts().insert(
    std::make_pair(obj.host_name(), h));

  // Custom variables.
  for (map_customvar::const_iterator
         it(obj.customvariables().begin()),
         end(obj.customvariables().end());
       it != end;
       ++it) {
    customvar var(it->first, it->second);
    h->set_customvar(var);
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_host_data(
    NEBTYPE_HOST_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    h.get(),
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);
}

/**
 *  @brief Expand a host.
 *
 *  During expansion, the host will be added to its host groups. These
 *  will be modified in the state.
 *
 *  @param[int,out] s   Configuration state.
 */
void applier::host::expand_objects(configuration::state& s) {
  // Browse all hosts.
  for (configuration::set_host::iterator
         it_host(s.hosts().begin()),
         end_host(s.hosts().end());
       it_host != end_host;
       ++it_host)
    // Browse current host's groups.
    for (set_string::const_iterator
           it_group(it_host->hostgroups().begin()),
           end_group(it_host->hostgroups().end());
         it_group != end_group;
         ++it_group) {
      // Find host group.
      configuration::set_hostgroup::iterator
        group(s.hostgroups_find(*it_group));
      if (group == s.hostgroups().end())
        throw (engine_error() << "Could not add host '"
               << it_host->host_name() << "' to non-existing host group '"
               << *it_group << "'");

      // Remove host group from state.
      configuration::hostgroup backup(*group);
      s.hostgroups().erase(group);

      // Add host to group members.
      backup.members().insert(it_host->host_name());

      // Reinsert host group.
      s.hostgroups().insert(backup);
    }

  return ;
}

/**
 *  Modified host.
 *
 *  @param[in] obj  The new host to modify into the monitoring engine.
 */
void applier::host::modify_object(
                      configuration::host const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying host '" << obj.host_name() << "'.";

  // Find the configuration object.
  set_host::iterator it_cfg(config->hosts_find(obj.key()));
  if (it_cfg == config->hosts().end())
    throw (engine_error() << "Cannot modify non-existing host '"
           << obj.host_name() << "'");

  // Find host object.
  umap<std::string, shared_ptr< ::host> >::iterator
    it_obj(applier::state::instance().hosts().find(obj.key()));
  if (it_obj == applier::state::instance().hosts().end())
    throw (engine_error() << "Could not modify non-existing "
           << "host object '" << obj.host_name() << "'");
  ::host* h(it_obj->second.get());

  // Update the global configuration set.
  configuration::host obj_old(*it_cfg);
  config->hosts().erase(it_cfg);
  config->hosts().insert(obj);

  // Modify properties.
  modify_if_different(*h, display_name, obj.display_name());
  modify_if_different(
    *h,
    alias,
    (obj.alias().empty() ? obj.host_name() : obj. alias()));
  modify_if_different(*h, address, obj.address());
  modify_if_different(*h, initial_state, obj.initial_state());
  modify_if_different(*h, normal_check_interval, obj.check_interval());
  modify_if_different(*h, retry_check_interval, obj.retry_interval());
  modify_if_different(*h, max_attempts, obj.max_check_attempts());
  h->set_notify_on(
    ::host::ON_RECOVERY,
    obj.notification_options() & configuration::host::up);
  h->set_notify_on(
    ::host::ON_DOWN,
    obj.notification_options() & configuration::host::down);
  h->set_notify_on(
    ::host::ON_UNREACHABLE,
    obj.notification_options() & configuration::host::unreachable);
  h->set_notify_on(
    ::host::ON_FLAPPING,
    obj.notification_options() & configuration::host::flapping);
  h->set_notify_on(
    ::host::ON_DOWNTIME,
    obj.notification_options() & configuration::host::downtime);
  modify_if_different(
    *h,
    notification_interval,
    obj.notification_interval());
  modify_if_different(
    *h,
    first_notification_delay,
    obj.first_notification_delay());
  modify_if_different(
    *h,
    notifications_enabled,
    obj.notifications_enabled());
  modify_if_different(*h, active_checks_enabled, obj.checks_active());
  modify_if_different(*h, passive_checks_enabled, obj.checks_passive());
  modify_if_different(
    *h,
    event_handler_enabled,
    obj.event_handler_enabled());
  modify_if_different(
    *h,
    flap_detection_enabled,
    obj.flap_detection_enabled());
  modify_if_different(*h, low_flap_threshold, obj.low_flap_threshold());
  modify_if_different(
    *h,
    high_flap_threshold,
    obj.high_flap_threshold());
  modify_if_different(
    *h,
    flap_detection_on_up,
    static_cast<bool>(
      obj.flap_detection_options() & configuration::host::up));
  modify_if_different(
    *h,
    flap_detection_on_down,
    static_cast<bool>(
      obj.flap_detection_options() & configuration::host::down));
  modify_if_different(
    *h,
    flap_detection_on_unreachable,
    static_cast<bool>(
      obj.flap_detection_options() & configuration::host::unreachable));
  modify_if_different(
    *h,
    stalk_on_up,
    static_cast<bool>(
      obj.stalking_options() & configuration::host::up));
  modify_if_different(
    *h,
    stalk_on_down,
    static_cast<bool>(
      obj.stalking_options() & configuration::host::down));
  modify_if_different(
    *h,
    stalk_on_unreachable,
    static_cast<bool>(
      obj.stalking_options() & configuration::host::unreachable));
  modify_if_different(
    *h,
    process_perfdata,
    obj.process_perf_data());
  modify_if_different(
    *h,
    freshness_checks_enabled,
    obj.check_freshness());
  modify_if_different(
    *h,
    freshness_threshold,
    obj.freshness_threshold());
  modify_if_different(*h, notes, obj.notes());
  modify_if_different(*h, notes_url, obj.notes_url());
  modify_if_different(*h, action_url, obj.action_url());
  modify_if_different(*h, icon_image, obj.icon_image());
  modify_if_different(*h, icon_image_alt, obj.icon_image_alt());
  modify_if_different(*h, vrml_image, obj.vrml_image());
  modify_if_different(*h, statusmap_image, obj.statusmap_image());
  modify_if_different(*h, x_2d, obj.coords_2d().x());
  modify_if_different(*h, y_2d, obj.coords_2d().y());
  modify_if_different(*h, have_2d_coords, obj.have_coords_2d());
  modify_if_different(*h, x_3d, obj.coords_3d().x());
  modify_if_different(*h, y_3d, obj.coords_3d().y());
  modify_if_different(*h, z_3d, obj.coords_3d().z());
  modify_if_different(*h, have_3d_coords, obj.have_coords_3d());
  modify_if_different(
    *h,
    retain_state_info,
    obj.retain_status_information());
  modify_if_different(
    *h,
    retain_nonstate_info,
    obj.retain_nonstatus_information());
  modify_if_different(*h, ocp_enabled, obj.obsess_over_host());
  modify_if_different(*h, timezone, obj.timezone());
  modify_if_different(*h, id, obj.host_id());
  modify_if_different(
    *h,
    acknowledgement_timeout,
    obj.get_acknowledgement_timeout());
  modify_if_different(
    *h,
    recovery_notification_delay,
    obj.recovery_notification_delay());

  // Contacts.
  if (obj.contacts() != obj_old.contacts()) {
    // Remove contacts.
    h->clear_contacts();

    // Add contacts to host.
    for (set_string::const_iterator
           it(obj.contacts().begin()),
           end(obj.contacts().end());
         it != end;
         ++it) {
      contact_map::iterator cntct(state::instance().contacts().find(*it));
      if (cntct == state::instance().contacts().end())
        throw (engine_error() << "Could not add contact '"
               << *it << "' to host '" << obj.host_name() << "'");
      h->add_contact(cntct->second.get());
    }
  }

  // Contact groups.
  if (obj.contactgroups() != obj_old.contactgroups()) {
    // Remove old contact groups.
    h->clear_contactgroups();

    // Add contact groups to host.
    for (set_string::const_iterator
           it(obj.contactgroups().begin()),
           end(obj.contactgroups().end());
         it != end;
         ++it) {
      contactgroup_map::iterator
        grp(state::instance().contactgroups().find(*it));
      if (grp == state::instance().contactgroups().end())
        throw (engine_error() << "Could not add contact group '"
               << *it << "' to host '" << obj.host_name() << "'");
      h->add_contactgroup(grp->second.get());
    }
  }

  // Custom variables.
  if (obj.customvariables() != obj_old.customvariables()) {
    // Delete old custom variables.
    timeval tv(get_broker_timestamp(NULL));
    for (customvar_set::const_iterator
           it(h->get_customvars().begin()),
           end(h->get_customvars().end());
         it != end;
         ++it)
      broker_custom_variable(
        NEBTYPE_HOSTCUSTOMVARIABLE_DELETE,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        h,
        it->second.get_name().c_str(),
        it->second.get_value().c_str(),
        &tv);
    h->clear_customvars();

    // Add custom variables.
    for (map_customvar::const_iterator
           it(obj.customvariables().begin()),
           end(obj.customvariables().end());
         it != end;
         ++it) {
      h->set_customvar(customvar(it->first, it->second));
      broker_custom_variable(
        NEBTYPE_HOSTCUSTOMVARIABLE_ADD,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        h,
        it->first.c_str(),
        it->second.c_str(),
        &tv);
    }
  }

  // Parents.
  if (obj.parents() != obj_old.parents()) {
    // Delete old parents.
    {
      timeval tv(get_broker_timestamp(NULL));
      for (host_set::const_iterator
             it(h->get_parents().begin()),
             end(h->get_parents().end());
           it != end;
           ++it)
        broker_relation_data(
          NEBTYPE_PARENT_DELETE,
          NEBFLAG_NONE,
          NEBATTR_NONE,
          *it,
          NULL,
          h,
          NULL,
          &tv);
    }
    h->clear_parents();
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_host_data(
    NEBTYPE_HOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    h,
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);

  return ;
}

/**
 *  Remove old host.
 *
 *  @param[in] obj The new host to remove from the monitoring engine.
 */
void applier::host::remove_object(
                      configuration::host const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing host '" << obj.host_name() << "'.";

  // Find host.
  umap<std::string, shared_ptr< ::host> >::iterator
    it(applier::state::instance().hosts().find(obj.key()));
  if (it != applier::state::instance().hosts().end()) {
    ::host* hst(it->second.get());

    // Remove host comments.
    engine::comment::delete_all_host_comments(obj.host_name());

    // Remove host downtimes.
    //FIXME DBR: this function does not exist anymore
//    delete_downtime_by_hostname_service_description_start_time_comment(
//      obj.host_name().c_str(),
//      NULL,
//      (time_t)0,
//      NULL);

    // Remove events related to this host.
    applier::scheduler::instance().remove_host(obj);

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_host_data(
      NEBTYPE_HOST_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      hst,
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);

    // Erase host object (will effectively delete the object).
    applier::state::instance().hosts().erase(it);
  }

  // Remove host from the global configuration set.
  config->hosts().erase(obj);

  return ;
}

/**
 *  Resolve a host.
 *
 *  @param[in] obj  Host object.
 */
void applier::host::resolve_object(
                      configuration::host const& obj) {
  // Failure flag.
  bool failure(false);

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving host '" << obj.host_name() << "'.";

  try {
    // Find host.
    ::host& hst(
              *applier::state::instance().hosts_find(obj.key()).get());

    // Make sure host has at least one service associated with it.
    // XXX

    // Resolve check command.
    try {
      resolve_check_command(hst, obj.check_command());
    }
    catch (not_found const& e) {
      (void)e;
      logger(logging::log_verification_error, logging::basic)
        << "Error: Host check command '" << obj.check_command()
        << "' specified for host '" << hst.get_name()
        << "' is not defined anywhere!";
      ++config_errors;
      failure = true;
    }

    // Resolve check period.
    if (!obj.check_period().empty()) {
      try {
        resolve_check_period(hst, obj.check_period());
      }
      catch (not_found const& e) {
        (void)e;
        logger(logging::log_verification_error, logging::basic)
          << "Error: Check period '" << obj.check_period()
          << "' specified for host '" << hst.get_name()
          << "' is not defined anywhere!";
        ++config_errors;
        failure = true;
      }
    }
    else {
      logger(logging::log_verification_error, logging::basic)
        << "Warning: Host '" << hst.get_name()
        << "' has no check time period defined!";
      ++config_warnings;
    }

    // Resolve event handler.
    if (!obj.event_handler().empty()) {
      try {
        // Get command.
        resolve_event_handler(hst, obj.event_handler());
      }
      catch (not_found const& e) {
        (void)e;
        logger(logging::log_verification_error, logging::basic)
          << "Error: Event handler command '" << obj.event_handler()
          << "' specified for host '" << hst.get_name()
          << "' not defined anywhere";
        ++config_errors;
        failure = true;
      }
    }

    // Resolve parents.
    for (set_string::const_iterator
           it(obj.parents().begin()),
           end(obj.parents().end());
         it != end;
         ++it)
      try {
        hst.add_parent(
          configuration::applier::state::instance().hosts_find(
            *it).get());
      }
      catch (not_found const& e) {
        (void)e;
        logger(logging::log_verification_error, logging::basic)
          << "Error: '" << *it << "' is not a valid parent for host '"
          << hst.get_name() << "'!";
        ++config_errors;
        failure = true;
      }

    // Resolve contacts.
    for (set_string::const_iterator
           it(obj.contacts().begin()),
           end(obj.contacts().end());
         it != end;
         ++it)
      try {
        hst.add_contact(
          configuration::applier::state::instance().contacts_find(
                                                      *it).get());
      }
      catch (not_found const& e) {
        (void)e;
        logger(logging::log_verification_error, logging::basic)
          << "Error: Contact '" << *it << "' specified in host '"
          << hst.get_name() << "' is not defined anywhere!";
        ++config_errors;
        failure = true;
      }

    // Resolve contact groups.
    for (set_string::const_iterator
           it(obj.contactgroups().begin()),
           end(obj.contactgroups().end());
         it != end;
         ++it)
      try {
        hst.add_contactgroup(
          configuration::applier::state::instance().contactgroups_find(
                                                      *it).get());
      }
      catch (not_found const& e) {
        (void)e;
        logger(logging::log_verification_error, logging::basic)
          << "Error: Contact group '" << *it << "' specified in host '"
          << hst.get_name() << "' is not defined anywhere!";
        ++config_errors;
        failure = true;
      }

    // Resolve notification period.
    if (!obj.notification_period().empty())
      try {
        resolve_notification_period(hst, obj.notification_period());
      }
      catch (not_found const& e) {
        (void)e;
        logger(logging::log_verification_error, logging::basic)
          << "Error: Notification period '" << obj.notification_period()
          << "' specified for host '" << hst.get_name()
          << "' is not defined anywhere!";
        ++config_errors;
        failure = true;
      }

    // Check for sane recovery options.
    if (hst.get_notifications_enabled()
        && hst.get_notify_on(::host::ON_RECOVERY)
        && !hst.get_notify_on(::host::ON_DOWN)
        && !hst.get_notify_on(::host::ON_UNREACHABLE)) {
      logger(logging::log_verification_error, logging::basic)
        << "Warning: Recovery notification option in host '"
        << hst.get_name() << "' definition doesn't make any sense - "
           "specify down and/or unreachable options as well";
      ++config_warnings;
    }

    // Check for illegal characters in host name.
    if (contains_illegal_object_chars(hst.get_name().c_str())) {
      logger(logging::log_verification_error, logging::basic)
        << "Error: The name of host '" << hst.get_name()
        << "' contains one or more illegal characters.";
      ++config_errors;
      failure = true;
    }

    // Throw exception in case of failure.
    if (failure)
      throw (error() << "please check logs above");
  }
  catch (std::exception const& e) {
    throw (engine_error() << "Could not resolve host '"
           << obj.host_name() << "': " << e.what());
  }

  return ;
}

/**
 *  Remove all links to other objects in all host objects.
 */
void applier::host::unresolve_objects() {
  for (umap<std::string, shared_ptr< ::host> >::iterator
         it(applier::state::instance().hosts().begin()),
         end(applier::state::instance().hosts().end());
       it != end;
       ++it) {
    ::host& h(*it->second);
    h.clear_children();
    h.clear_contacts();
    h.clear_contactgroups();
    h.clear_groups();
    h.clear_parents();
    h.clear_services();
    h.set_check_command(NULL);
    h.set_check_command_args("");
    h.set_check_period(NULL);
    h.set_event_handler(NULL);
    h.set_event_handler_args("");
    h.set_notification_period(NULL);
  }
  return ;
}

/**
 *  Resolve host check command.
 *
 *  @param[out] hst  Target host.
 *  @param[in]  cmd  New check command.
 */
void applier::host::resolve_check_command(
                      ::host& hst,
                      std::string const& cmd) {
  std::string command_name(cmd.substr(
                             0,
                             cmd.find_first_of('!')));
  hst.set_check_command(find_command(command_name).get());
  hst.set_check_command_args(cmd);
  return ;
}

/**
 *  Resolve host check period.
 *
 *  @param[out] hst     Target host.
 *  @param[in]  period  New check period.
 */
void applier::host::resolve_check_period(
                      ::host& hst,
                      std::string const& period) {
  hst.set_check_period(
    configuration::applier::state::instance().timeperiods_find(
      period).get());
  return ;
}

/**
 *  Resolve event handler.
 *
 *  @param[out] hst  Target host.
 *  @param[in]  cmd  New event handler.
 */
void applier::host::resolve_event_handler(
                      ::host& hst,
                      std::string const& cmd) {
  std::string command_name(cmd.substr(
                             0,
                             cmd.find_first_of('!')));
  hst.set_event_handler(find_command(command_name).get());
  hst.set_event_handler_args(cmd);
  return ;
}

/**
 *  Resolve notification period.
 *
 *  @param[out] hst     Target host.
 *  @param[in]  period  New notification period.
 */
void applier::host::resolve_notification_period(
                      ::host& hst,
                      std::string const& period) {
  hst.set_notification_period(
    configuration::applier::state::instance().timeperiods_find(
      period).get());
  return ;
}
