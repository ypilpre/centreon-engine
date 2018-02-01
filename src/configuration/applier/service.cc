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
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/scheduler.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
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
applier::service::service() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::service::service(applier::service const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::service::~service() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::service& applier::service::operator=(
                                      applier::service const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new service.
 *
 *  @param[in] obj  The new service to add into the monitoring engine.
 */
void applier::service::add_object(
                         configuration::service const& obj) {
  // Check service.
  if (obj.hosts().size() != 1)
    throw (engine_error() << "Could not create service '"
           << obj.service_description()
           << "' with multiple hosts defined");
  else if (!obj.hostgroups().empty())
    throw (engine_error() << "Could not create service '"
           << obj.service_description()
           << "' with multiple host groups defined");

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new service '" << obj.service_description()
    << "' of host '" << *obj.hosts().begin() << "'.";

  // Add service to the global configuration set.

  // Create service.
  shared_ptr< ::service> svc;
  try {
    svc = new ::service();
    // Self properties.
    svc->set_description(obj.service_description());
    svc->set_stalk_on_critical(
      obj.stalking_options() & configuration::service::critical);
    svc->set_stalk_on_ok(
      obj.stalking_options() & configuration::service::ok);
    svc->set_stalk_on_unknown(
      obj.stalking_options() & configuration::service::unknown);
    svc->set_stalk_on_warning(
      obj.stalking_options() & configuration::service::warning);
    svc->set_volatile(obj.is_volatile());
    svc->set_flap_detection_on_critical(
      obj.flap_detection_options() & configuration::service::critical);
    svc->set_flap_detection_on_ok(
      obj.flap_detection_options() & configuration::service::ok);
    svc->set_flap_detection_on_warning(
      obj.flap_detection_options() & configuration::service::warning);
    svc->set_flap_detection_on_unknown(
      obj.flap_detection_options() & configuration::service::unknown);
    // Inherited from monitorable.
    svc->set_action_url(obj.action_url());
    svc->set_display_name(obj.display_name());
    svc->set_icon_image(obj.icon_image());
    svc->set_icon_image_alt(obj.icon_image_alt());
    svc->set_id(obj.service_id());
    svc->set_initial_state(obj.initial_state());
    svc->set_notes(obj.notes());
    svc->set_notes_url(obj.notes_url());
    svc->set_retain_nonstate_info(obj.retain_nonstatus_information());
    svc->set_retain_state_info(obj.retain_status_information());
    for (map_customvar::const_iterator
           it(obj.customvariables().begin()),
           end(obj.customvariables().end());
         it != end;
         ++it)
      svc->set_customvar(customvar(it->first, it->second));
    // Inherited from notifier.
    svc->set_acknowledgement_timeout(obj.get_acknowledgement_timeout());
    svc->set_notifications_enabled(obj.notifications_enabled());
    svc->set_notify_on(
      ::service::ON_FLAPPING,
      obj.notification_options() & configuration::service::downtime);
    svc->set_notify_on(
      ::service::ON_FLAPPING,
      obj.notification_options() & configuration::service::flapping);
    svc->set_notify_on(
      ::service::ON_RECOVERY,
      obj.notification_options() & configuration::service::ok);
    svc->set_notify_on(
      ::service::ON_CRITICAL,
      obj.notification_options() & configuration::service::critical);
    svc->set_notify_on(
      ::service::ON_UNKNOWN,
      obj.notification_options() & configuration::service::unknown);
    svc->set_notify_on(
      ::service::ON_WARNING,
      obj.notification_options() & configuration::service::warning);
    svc->set_notification_interval(obj.notification_interval());
    svc->set_first_notification_delay(obj.first_notification_delay());
    svc->set_recovery_notification_delay(obj.recovery_notification_delay());
    // Inherited from checkable.
    svc->set_active_checks_enabled(obj.checks_active());
    svc->set_event_handler_enabled(obj.event_handler_enabled());
    svc->set_flap_detection_enabled(obj.flap_detection_enabled());
    svc->set_freshness_checks_enabled(obj.check_freshness());
    svc->set_freshness_threshold(obj.freshness_threshold());
    svc->set_high_flap_threshold(obj.high_flap_threshold());
    svc->set_low_flap_threshold(obj.low_flap_threshold());
    svc->set_max_attempts(obj.max_check_attempts());
    svc->set_normal_check_interval(obj.check_interval());
    svc->set_ocp_enabled(obj.obsess_over_service());
    svc->set_passive_checks_enabled(obj.checks_passive());
    svc->set_process_perfdata(obj.process_perf_data());
    svc->set_retry_check_interval(obj.retry_interval());
    svc->set_timezone(obj.timezone());

    // Add service to global configuration set.
    config->services().insert(obj);
  }
  catch (std::exception const& e) {
    logger(logging::log_config_error, logging::basic)
      << "Error: " << e.what();
    throw (engine_error()
           << "Could not register service '" << obj.service_description()
           << "' of host '" << *obj.hosts().begin() << "'");
  }

  // Add new items to the configuration state.
  configuration::applier::state::instance().services().insert(
    std::make_pair(
      std::make_pair(*obj.hosts().begin(), obj.service_description()),
      svc));

  // Add custom variables.
  for (map_customvar::const_iterator
         it(obj.customvariables().begin()),
         end(obj.customvariables().end());
       it != end;
       ++it) {
    customvar var(it->first, it->second);
    svc->set_customvar(var);
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_service_data(
    NEBTYPE_SERVICE_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc.get(),
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);

  return ;
}

/**
 *  Expand a service object.
 *
 *  @param[in,out] s  State being applied.
 */
void applier::service::expand_objects(configuration::state& s) {
  // Browse all services.
  configuration::set_service expanded;
  for (configuration::set_service::iterator
         it_svc(s.services().begin()),
         end_svc(s.services().end());
       it_svc != end_svc;
       ++it_svc) {
    // Expand service to instances.
    std::set<std::string> target_hosts;

    // Hosts members.
    target_hosts = it_svc->hosts();

    // Host group members.
    for (set_string::const_iterator
           it(it_svc->hostgroups().begin()),
           end(it_svc->hostgroups().end());
         it != end;
         ++it) {
      // Find host group.
      set_hostgroup::iterator it2(s.hostgroups_find(*it));
      if (it2 == s.hostgroups().end())
        throw (engine_error() << "Could not find host group '"
               << *it << "' on which to apply service '"
               << it_svc->service_description() << "'");

      // Check host group and user configuration.
      if (it2->members().empty()
          && !s.allow_empty_hostgroup_assignment())
        throw (engine_error() << "Could not expand host group '"
               << *it << "' specified in service '"
               << it_svc->service_description() << "'");

      // Add host group members.
      target_hosts.insert(
                     it2->members().begin(),
                     it2->members().end());
    }

    // Browse all target hosts.
    for (std::set<std::string>::const_iterator
           it(target_hosts.begin()),
           end(target_hosts.end());
         it != end;
         ++it) {
      // Create service instance.
      configuration::service svc(*it_svc);
      svc.hostgroups().clear();
      svc.hosts().clear();
      svc.hosts().insert(*it);

      // Expand memberships.
      _expand_service_memberships(svc, s);

      // Inherits special vars.
      _inherits_special_vars(svc, s);

      // Insert object.
      expanded.insert(svc);
    }
  }

  // Set expanded services in configuration state.
  s.services().swap(expanded);

  return ;
}

/**
 *  Modified service.
 *
 *  @param[in] obj  The new service to modify into the monitoring
 *                  engine.
 */
void applier::service::modify_object(
                         configuration::service const& obj) {
  std::string const& host_name(*obj.hosts().begin());
  std::string const& service_description(obj.service_description());

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying new service '" << service_description
    << "' of host '" << host_name << "'.";

  // Find the configuration object.
  set_service::iterator it_cfg(config->services_find(obj.key()));
  if (it_cfg == config->services().end())
    throw (engine_error() << "Cannot modify non-existing "
           "service '" << service_description << "' of host '"
           << host_name << "'");

  // Find service object.
  umap<std::pair<std::string, std::string>, shared_ptr< ::service> >::iterator
    it_obj(applier::state::instance().services().find(obj.key()));
  if (it_obj == applier::state::instance().services().end())
    throw (engine_error() << "Could not modify non-existing "
           << "service object '" << service_description
           << "' of host '" << host_name << "'");
  ::service* s(it_obj->second.get());

  // Update the global configuration set.
  configuration::service obj_old(*it_cfg);
  config->services().erase(it_cfg);
  config->services().insert(obj);

  // Modify properties.
  modify_if_different(*s, display_name, obj.display_name());
  modify_if_different(
    *s,
    event_handler_enabled,
    obj.event_handler_enabled());
  modify_if_different(*s, initial_state, obj.initial_state());
  modify_if_different(*s, normal_check_interval, obj.check_interval());
  modify_if_different(*s, retry_check_interval, obj.retry_interval());
  modify_if_different(*s, max_attempts, obj.max_check_attempts());
  modify_if_different(
    *s,
    notification_interval,
    obj.notification_interval());
  modify_if_different(
    *s,
    first_notification_delay,
    obj.first_notification_delay());
  s->set_notify_on(
    ::service::ON_FLAPPING,
    obj.notification_options() & configuration::service::downtime);
  s->set_notify_on(
    ::service::ON_FLAPPING,
    obj.notification_options() & configuration::service::flapping);
  s->set_notify_on(
    ::service::ON_RECOVERY,
    obj.notification_options() & configuration::service::ok);
  s->set_notify_on(
    ::service::ON_CRITICAL,
    obj.notification_options() & configuration::service::critical);
  s->set_notify_on(
    ::service::ON_UNKNOWN,
    obj.notification_options() & configuration::service::unknown);
  s->set_notify_on(
    ::service::ON_WARNING,
    obj.notification_options() & configuration::service::warning);
  modify_if_different(
    *s,
    stalk_on_ok,
    static_cast<bool>(
      obj.stalking_options() & configuration::service::ok));
  modify_if_different(
    *s,
    stalk_on_warning,
    static_cast<bool>(
      obj.stalking_options() & configuration::service::warning));
  modify_if_different(
    *s,
    stalk_on_unknown,
    static_cast<bool>(
      obj.stalking_options() & configuration::service::unknown));
  modify_if_different(
    *s,
    stalk_on_critical,
    static_cast<bool>(
      obj.stalking_options() & configuration::service::critical));
  modify_if_different(
    *s,
    flap_detection_enabled,
    obj.flap_detection_enabled());
  modify_if_different(
    *s,
    low_flap_threshold,
    obj.low_flap_threshold());
  modify_if_different(
    *s,
    high_flap_threshold,
    obj.high_flap_threshold());
  modify_if_different(
    *s,
    flap_detection_on_ok,
    static_cast<bool>(
      obj.flap_detection_options() & configuration::service::ok));
  modify_if_different(
    *s,
    flap_detection_on_warning,
    static_cast<bool>(
      obj.flap_detection_options() & configuration::service::warning));
  modify_if_different(
    *s,
    flap_detection_on_unknown,
    static_cast<bool>(
      obj.flap_detection_options() & configuration::service::unknown));
  modify_if_different(
    *s,
    flap_detection_on_critical,
    static_cast<bool>(
      obj.flap_detection_options() & configuration::service::critical));
  modify_if_different(*s, process_perfdata, obj.process_perf_data());
  modify_if_different(
    *s,
    freshness_checks_enabled,
    obj.check_freshness());
  modify_if_different(
    *s,
    freshness_threshold,
    obj.freshness_threshold());
  modify_if_different(
    *s,
    passive_checks_enabled,
    obj.checks_passive());
  modify_if_different(
    *s,
    active_checks_enabled,
    obj.checks_active());
  modify_if_different(
    *s,
    retain_state_info,
    obj.retain_status_information());
  modify_if_different(
    *s,
    retain_nonstate_info,
    obj.retain_nonstatus_information());
  modify_if_different(
    *s,
    notifications_enabled,
    obj.notifications_enabled());
  modify_if_different(
    *s,
    ocp_enabled,
    obj.obsess_over_service());
  modify_if_different(*s, notes, obj.notes());
  modify_if_different(*s, notes_url, obj.notes_url());
  modify_if_different(*s, action_url, obj.action_url());
  modify_if_different(*s, icon_image, obj.icon_image());
  modify_if_different(*s, icon_image_alt, obj.icon_image_alt());
  modify_if_different(*s, volatile, obj.is_volatile());
  modify_if_different(*s, timezone, obj.timezone());
  modify_if_different(*s, id, obj.service_id());
  modify_if_different(
    *s,
    acknowledgement_timeout,
    obj.get_acknowledgement_timeout());
  modify_if_different(
    *s,
    recovery_notification_delay,
    obj.recovery_notification_delay());

  // Contacts.
  if (obj.contacts() != obj_old.contacts()) {
    // Remove contacts.
    s->clear_contacts();

    // Add contacts to host.
    for (set_string::const_iterator
           it(obj.contacts().begin()),
           end(obj.contacts().end());
         it != end;
         ++it) {
      contact_map::iterator cntct(state::instance().contacts().find(*it));
      if (cntct == state::instance().contacts().end())
        throw (engine_error() << "Could not add contact '"
               << *it << "' to service '" << obj.service_description()
               << "' on host '" << *obj.hosts().begin() << "'");
      s->add_contact(cntct->second.get());
    }
  }

  // Contact groups.
  if (obj.contactgroups() != obj_old.contactgroups()) {
    // Remove old contact groups.
    s->clear_contactgroups();

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
               << *it << "' to service '" << obj.service_description()
               << "' on host '" << *obj.hosts().begin() << "'");
      s->add_contactgroup(grp->second.get());
    }
  }

  // Custom variables.
  if (obj.customvariables() != obj_old.customvariables()) {
    // Delete old custom variables.
    timeval tv(get_broker_timestamp(NULL));
    for (customvar_set::const_iterator
           it(s->get_customvars().begin()),
           end(s->get_customvars().end());
         it != end;
         ++it)
      broker_custom_variable(
        NEBTYPE_SERVICECUSTOMVARIABLE_DELETE,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        s,
        it->second.get_name().c_str(),
        it->second.get_value().c_str(),
        &tv);
    s->clear_customvars();

    // Add custom variables.
    for (map_customvar::const_iterator
           it(obj.customvariables().begin()),
           end(obj.customvariables().end());
         it != end;
         ++it) {
      s->set_customvar(customvar(it->first, it->second));
      broker_custom_variable(
        NEBTYPE_SERVICECUSTOMVARIABLE_ADD,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        s,
        it->first.c_str(),
        it->second.c_str(),
        &tv);
    }
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_service_data(
    NEBTYPE_SERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    s,
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);

  return ;
}

/**
 *  Remove old service.
 *
 *  @param[in] obj  The new service to remove from the monitoring
 *                  engine.
 */
void applier::service::remove_object(
                         configuration::service const& obj) {
  std::string const& host_name(*obj.hosts().begin());
  std::string const& service_description(obj.service_description());

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing service '" << service_description
    << "' of host '" << host_name << "'.";

  // Find service.
  std::pair<std::string, std::string>
    id(std::make_pair(host_name, service_description));
  umap<std::pair<std::string, std::string>, shared_ptr< ::service> >::iterator
    it(applier::state::instance().services().find(obj.key()));
  if (it != applier::state::instance().services().end()) {
    ::service* svc(it->second.get());

    // Remove service comments.
    comment::delete_all_service_comments(host_name, service_description);

    // Remove service downtimes.
    //FIXME DBR: does not exist anymore
//    delete_downtime_by_hostname_service_description_start_time_comment(
//      host_name.c_str(),
//      service_description.c_str(),
//      (time_t)0,
//      NULL);

    // Remove events related to this service.
    applier::scheduler::instance().remove_service(obj);

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_service_data(
      NEBTYPE_SERVICE_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      svc,
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);

    // Remove service object (will effectively delete the object).
    applier::state::instance().services().erase(it);
  }

  // Remove service from the global configuration set.
  config->services().erase(obj);

  return ;
}

/**
 *  Resolve a service.
 *
 *  @param[in] obj  Service object.
 */
void applier::service::resolve_object(
                         configuration::service const& obj) {
  // Failure flag.
  bool failure(false);

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving service '" << obj.service_description()
    << "' of host '" << *obj.hosts().begin() << "'.";

  try {
    // Find service.
    ::service& svc(
      *applier::state::instance().services_find(obj.key()).get());

    // Find host and adjust its counters.
    try {
      ::host* hst(NULL);
      hst = applier::state::instance().hosts_find(
                                         *obj.hosts().begin()).get();
      svc.set_host(hst);
      hst->add_service(&svc);
    }
    catch (not_found const& e) {
      (void)e;
      logger(logging::log_verification_error, logging::basic)
        << "Error: Host '" << svc.get_host_name()
        << "' specified in service '" << svc.get_description()
        << "' not defined anywhere!";
      ++config_errors;
      failure = true;
    }

    // Resolve check command.
    try {
      // Set resolved command and arguments.
      resolve_check_command(svc, obj.check_command());
    }
    catch (not_found const& e) {
      (void)e;
      logger(logging::log_verification_error, logging::basic)
        << "Error: Service check command '" << obj.check_command()
        << "' specified in service '" << svc.get_description()
        << "' for host '" << svc.get_host_name()
        << "' not defined anywhere!";
      ++config_errors;
      failure = true;
    }

    // Resolve check period.
    if (!obj.check_period().empty()) {
      try {
        resolve_check_period(svc, obj.check_period());
      }
      catch (not_found const& e) {
        (void)e;
        logger(logging::log_verification_error, logging::basic)
          << "Error: Check period '" << obj.check_period()
          << "' specified for service '" << svc.get_description()
          << "' on host '" << svc.get_host_name()
          << "' is not defined anywhere!";
        ++config_errors;
        failure = true;
      }
    }
    else {
      logger(logging::log_verification_error, logging::basic)
        << "Warning: Service '" << svc.get_description()
        << "' on host '" << svc.get_host_name()
        << "' has no check time period defined!";
      ++config_warnings;
    }

    // Resolve event handler.
    if (!obj.event_handler().empty()) {
      try {
        // Get command.
        resolve_event_handler(svc, obj.event_handler());
      }
      catch (not_found const& e) {
        (void)e;
        logger(logging::log_verification_error, logging::basic)
          << "Error: Event handler command '" << obj.event_handler()
          << "' specified in service '" << svc.get_description()
          << "' for host '" << svc.get_host_name()
          << "' not defined anywhere";
        ++config_errors;
        failure = true;
      }
    }

    // Resolve contacts.
    for (set_string::const_iterator
           it(obj.contacts().begin()),
           end(obj.contacts().end());
         it != end;
         ++it)
      try {
        svc.add_contact(
          configuration::applier::state::instance().contacts_find(
                                                      *it).get());
      }
      catch (not_found const& e) {
        (void)e;
        logger(logging::log_verification_error, logging::basic)
          << "Error: Contact '" << *it << "' specified in service '"
          << svc.get_description() << "' for host '"
          << svc.get_host_name() << "' is not defined anywhere!";
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
        svc.add_contactgroup(
          configuration::applier::state::instance().contactgroups_find(
                                                      *it).get());
      }
      catch (not_found const& e) {
        logger(logging::log_verification_error, logging::basic)
          << "Error: Contact group '" << *it
          << "' specified in service '" << svc.get_description()
          << "' for host '" << svc.get_host_name()
          << "' is not defined anywhere!";
        ++config_errors;
        failure = true;
      }

    // Resolve notification period.
    if (!obj.notification_period().empty())
      try {
        resolve_notification_period(svc, obj.notification_period());
      }
      catch (not_found const& e) {
        (void)e;
        logger(logging::log_verification_error, logging::basic)
          << "Error: Notification period '" << obj.notification_period()
          << "' specified for service '" << svc.get_description()
          << "' on host '" << svc.get_host_name()
          << "' is not defined anywhere!";
        ++config_errors;
        failure = true;
      }

    // Check for sane recovery options.
    if (svc.get_notifications_enabled()
        && svc.get_notify_on(::service::ON_RECOVERY)
        && !svc.get_notify_on(::service::ON_WARNING)
        && !svc.get_notify_on(::service::ON_CRITICAL)) {
      logger(logging::log_verification_error, logging::basic)
        << "Warning: Recovery notification option in service '"
        << svc.get_description() << "' for host '"
        << svc.get_host_name() << "' doesn't make any sense - "
           "specify warning and/or critical options as well";
      ++config_warnings;
    }

    // Check for illegal characters in service description.
    if (contains_illegal_object_chars(svc.get_description().c_str())) {
      logger(logging::log_verification_error, logging::basic)
        << "Error: The description string for service '"
        << svc.get_description() << "' on host '" << svc.get_host_name()
        << "' contains one or more illegal characters.";
      ++config_errors;
      failure = true;
    }

    // Throw exception in case of failure.
    if (failure)
      throw (error() << "please check logs above");
  }
  catch (std::exception const& e) {
    throw (engine_error() << "Could not resolve service '"
           << obj.service_description() << "' of host '"
           << *obj.hosts().begin() << "': " << e.what());
  }

  return ;
}

/**
 *  Remove all links to other objects in all service objects.
 */
void applier::service::unresolve_objects() {
  for (umap<std::pair<std::string, std::string>, shared_ptr< ::service> >::iterator
         it(applier::state::instance().services().begin()),
         end(applier::state::instance().services().end());
       it != end;
       ++it) {
    ::service& s(*it->second);
    s.clear_contacts();
    s.clear_contactgroups();
    s.clear_groups();
    s.set_check_command(NULL);
    s.set_check_command_args("");
    s.set_check_period(NULL);
    s.set_event_handler(NULL);
    s.set_event_handler_args("");
    s.set_host(NULL);
    s.set_notification_period(NULL);
  }
  return ;
}

/**
 *  Resolve service check command.
 *
 *  @param[out] svc  Target service.
 *  @param[in]  cmd  New check command.
 */
void applier::service::resolve_check_command(
                         ::service& svc,
                         std::string const& cmd) {
  std::string command_name(cmd.substr(
                             0,
                             cmd.find_first_of('!')));
  svc.set_check_command(find_command(command_name).get());
  svc.set_check_command_args(cmd);
  return ;
}

/**
 *  Resolve service check period.
 *
 *  @param[out] svc     Target service.
 *  @param[in]  period  New check period.
 */
void applier::service::resolve_check_period(
                         ::service& svc,
                         std::string const& period) {
  svc.set_check_period(
    configuration::applier::state::instance().timeperiods_find(
      period).get());
  return ;
}

/**
 *  Resolve event handler.
 *
 *  @param[out] svc  Target service.
 *  @param[in]  cmd  New event handler.
 */
void applier::service::resolve_event_handler(
                         ::service& svc,
                         std::string const& cmd) {
  std::string command_name(cmd.substr(
                             0,
                             cmd.find_first_of('!')));
  svc.set_event_handler(find_command(command_name).get());
  svc.set_event_handler_args(cmd);
  return ;
}

/**
 *  Resolve notification period.
 *
 *  @param[out] svc     Target service.
 *  @param[in]  period  New notification period.
 */
void applier::service::resolve_notification_period(
                         ::service& svc,
                         std::string const& period) {
  svc.set_notification_period(
    configuration::applier::state::instance().timeperiods_find(
      period).get());
  return ;
}

/**
 *  Expand service instance memberships.
 *
 *  @param[in]  obj Target service.
 *  @param[out] s   Configuration state.
 */
void applier::service::_expand_service_memberships(
                         configuration::service& obj,
                         configuration::state& s) {
  // Browse service groups.
  for (set_string::const_iterator
         it(obj.servicegroups().begin()),
         end(obj.servicegroups().end());
       it != end;
       ++it) {
    // Find service group.
    configuration::set_servicegroup::iterator
      it_group(s.servicegroups_find(*it));
    if (it_group == s.servicegroups().end())
      throw (engine_error() << "Could not add service '"
             << obj.service_description() << "' of host '"
             << *obj.hosts().begin()
             << "' to non-existing service group '" << *it << "'");

    // Remove service group from state.
    configuration::servicegroup backup(*it_group);
    s.servicegroups().erase(it_group);

    // Add service to service members.
    backup.members().insert(std::make_pair(
                                   *obj.hosts().begin(),
                                   obj.service_description()));

    // Reinsert service group.
    s.servicegroups().insert(backup);
  }

  return ;
}

/**
 *  @brief Inherits special variables from host.
 *
 *  These special variables, if not defined are inherited from host.
 *  They are contact_groups, notification_interval and
 *  notification_period.
 *
 *  @param[in,out] obj Target service.
 *  @param[in]     s   Configuration state.
 */
void applier::service::_inherits_special_vars(
                         configuration::service& obj,
                         configuration::state const& s) {
  // Detect if any special variable has not been defined.
  if (!obj.contacts_defined()
      || !obj.contactgroups_defined()
      || !obj.notification_interval_defined()
      || !obj.notification_period_defined()
      || !obj.timezone_defined()) {
    // Find host.
    configuration::set_host::const_iterator
      it(s.hosts_find(*obj.hosts().begin()));
    if (it == s.hosts().end())
      throw (engine_error()
             << "Could not inherit special variables for service '"
             << obj.service_description() << "': host '"
             << *obj.hosts().begin() << "' does not exist");

    // Inherits variables.
    if (!obj.contacts_defined() && !obj.contactgroups_defined()) {
      obj.contacts() = it->contacts();
      obj.contactgroups() = it->contactgroups();
    }
    if (!obj.notification_interval_defined())
      obj.notification_interval(it->notification_interval());
    if (!obj.notification_period_defined())
      obj.notification_period(it->notification_period());
    if (!obj.timezone_defined())
      obj.timezone(it->timezone());
  }

  return ;
}
