/*
** Copyright 2011-2017 Centreon
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
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/scheduler.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/contactgroupsmember.hh"
#include "com/centreon/engine/deleter/contactsmember.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

/**
 *  Check if the service group name matches the configuration object.
 */
class         servicegroup_name_comparator {
public:
              servicegroup_name_comparator(
                std::string const& servicegroup_name) {
    _servicegroup_name = servicegroup_name;
  }

  bool        operator()(shared_ptr<configuration::servicegroup> sg) {
    return (_servicegroup_name == sg->servicegroup_name());
  }

private:
  std::string _servicegroup_name;
};

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
  try {
    shared_ptr<::service> svc(new ::service(obj));
    config->services().insert(obj);
  }
  catch (std::exception const& e) {
    logger(logging::log_config_error, logging::basic)
      << "Error: " << e.what();
    throw (engine_error()
           << "Could not register service '" << obj.service_description()
           << "' of host '" << *obj.hosts().begin() << "'");
  }

  // service_other_props[std::make_pair(
  //                            *obj.hosts().begin(),
  //                            obj.service_description())].initial_notif_time = 0;
  // service_other_props[std::make_pair(
  //                            *obj.hosts().begin(),
  //                            obj.service_description())].timezone = obj.timezone();
  // service_other_props[std::make_pair(
  //                            *obj.hosts().begin(),
  //                            obj.service_description())].host_id = obj.host_id();
  // service_other_props[std::make_pair(
  //                            *obj.hosts().begin(),
  //                            obj.service_description())].service_id = obj.service_id();
  // service_other_props[std::make_pair(
  //                            *obj.hosts().begin(),
  //                            obj.service_description())].acknowledgement_timeout
  //   = obj.get_acknowledgement_timeout() * config->interval_length();
  // service_other_props[std::make_pair(
  //                            *obj.hosts().begin(),
  //                            obj.service_description())].last_acknowledgement = 0;
  // service_other_props[std::make_pair(
  //                            *obj.hosts().begin(),
  //                            obj.service_description())].recovery_notification_delay = obj.recovery_notification_delay();
  // service_other_props[std::make_pair(
  //                            *obj.hosts().begin(),
  //                            obj.service_description())].recovery_been_sent = true;

  // // Add contacts.
  // for (set_string::const_iterator
  //        it(obj.contacts().begin()),
  //        end(obj.contacts().end());
  //      it != end;
  //      ++it)
  //   if (!add_contact_to_service(svc, it->c_str()))
  //     throw (engine_error() << "Could not add contact '"
  //            << *it << "' to service '" << obj.service_description()
  //            << "' of host '" << *obj.hosts().begin() << "'");

  // // Add contactgroups.
  // for (set_string::const_iterator
  //        it(obj.contactgroups().begin()),
  //        end(obj.contactgroups().end());
  //      it != end;
  //      ++it)
  //   if (!add_contactgroup_to_service(svc, it->c_str()))
  //     throw (engine_error() << "Could not add contact group '"
  //            << *it << "' to service '" << obj.service_description()
  //            << "' of host '" << *obj.hosts().begin() << "'");

  // // Add custom variables.
  // for (map_customvar::const_iterator
  //        it(obj.customvariables().begin()),
  //        end(obj.customvariables().end());
  //      it != end;
  //      ++it)
  //   if (!add_custom_variable_to_service(
  //          svc,
  //          it->first.c_str(),
  //          it->second.c_str()))
  //     throw (engine_error() << "Could not add custom variable '"
  //            << it->first << "' to service '"
  //            << obj.service_description() << "' of host '"
  //            << *obj.hosts().begin() << "'");

  // // Notify event broker.
  // timeval tv(get_broker_timestamp(NULL));
  // broker_adaptive_service_data(
  //   NEBTYPE_SERVICE_ADD,
  //   NEBFLAG_NONE,
  //   NEBATTR_NONE,
  //   svc,
  //   CMD_NONE,
  //   MODATTR_ALL,
  //   MODATTR_ALL,
  //   &tv);

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
  // XXX
  // std::string const& host_name(*obj.hosts().begin());
  // std::string const& service_description(obj.service_description());

  // // Logging.
  // logger(logging::dbg_config, logging::more)
  //   << "Modifying new service '" << service_description
  //   << "' of host '" << host_name << "'.";

  // // Find the configuration object.
  // set_service::iterator it_cfg(config->services_find(obj.key()));
  // if (it_cfg == config->services().end())
  //   throw (engine_error() << "Cannot modify non-existing "
  //          "service '" << service_description << "' of host '"
  //          << host_name << "'");

  // // Find service object.
  // umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::iterator
  //   it_obj(applier::state::instance().services_find(obj.key()));
  // if (it_obj == applier::state::instance().services().end())
  //   throw (engine_error() << "Could not modify non-existing "
  //          << "service object '" << service_description
  //          << "' of host '" << host_name << "'");
  // service_struct* s(it_obj->second.get());

  // // Update the global configuration set.
  // configuration::service obj_old(*it_cfg);
  // config->services().erase(it_cfg);
  // config->services().insert(obj);

  // // Modify properties.
  // modify_if_different(
  //   s->display_name,
  //   NULL_IF_EMPTY(obj.display_name()));
  // modify_if_different(
  //   s->service_check_command,
  //   NULL_IF_EMPTY(obj.check_command()));
  // modify_if_different(
  //   s->event_handler,
  //   NULL_IF_EMPTY(obj.event_handler()));
  // modify_if_different(
  //   s->event_handler_enabled,
  //   static_cast<int>(obj.event_handler_enabled()));
  // modify_if_different(
  //   s->initial_state,
  //   static_cast<int>(obj.initial_state()));
  // modify_if_different(
  //   s->check_interval,
  //   static_cast<double>(obj.check_interval()));
  // modify_if_different(
  //   s->retry_interval,
  //   static_cast<double>(obj.retry_interval()));
  // modify_if_different(
  //   s->max_attempts,
  //   static_cast<int>(obj.max_check_attempts()));
  // modify_if_different(
  //   s->notification_interval,
  //   static_cast<double>(obj.notification_interval()));
  // modify_if_different(
  //   s->first_notification_delay,
  //   static_cast<double>(obj.first_notification_delay()));
  // modify_if_different(
  //   s->notify_on_unknown,
  //   static_cast<int>(static_cast<bool>(
  //     obj.notification_options() & configuration::service::unknown)));
  // modify_if_different(
  //   s->notify_on_warning,
  //   static_cast<int>(static_cast<bool>(
  //     obj.notification_options() & configuration::service::warning)));
  // modify_if_different(
  //   s->notify_on_critical,
  //   static_cast<int>(static_cast<bool>(
  //     obj.notification_options() & configuration::service::critical)));
  // modify_if_different(
  //   s->notify_on_recovery,
  //   static_cast<int>(static_cast<bool>(
  //     obj.notification_options() & configuration::service::ok)));
  // modify_if_different(
  //   s->notify_on_flapping,
  //   static_cast<int>(static_cast<bool>(
  //     obj.notification_options() & configuration::service::flapping)));
  // modify_if_different(
  //   s->notify_on_downtime,
  //   static_cast<int>(static_cast<bool>(
  //     obj.notification_options() & configuration::service::downtime)));
  // modify_if_different(
  //   s->stalk_on_ok,
  //   static_cast<int>(static_cast<bool>(
  //     obj.stalking_options() & configuration::service::ok)));
  // modify_if_different(
  //   s->stalk_on_warning,
  //   static_cast<int>(static_cast<bool>(
  //     obj.stalking_options() & configuration::service::warning)));
  // modify_if_different(
  //   s->stalk_on_unknown,
  //   static_cast<int>(static_cast<bool>(
  //     obj.stalking_options() & configuration::service::unknown)));
  // modify_if_different(
  //   s->stalk_on_critical,
  //   static_cast<int>(static_cast<bool>(
  //     obj.stalking_options() & configuration::service::critical)));
  // modify_if_different(
  //   s->notification_period,
  //   NULL_IF_EMPTY(obj.notification_period()));
  // modify_if_different(
  //   s->check_period,
  //   NULL_IF_EMPTY(obj.check_period()));
  // modify_if_different(
  //   s->flap_detection_enabled,
  //   static_cast<int>(obj.flap_detection_enabled()));
  // modify_if_different(
  //   s->low_flap_threshold,
  //   static_cast<double>(obj.low_flap_threshold()));
  // modify_if_different(
  //   s->high_flap_threshold,
  //   static_cast<double>(obj.high_flap_threshold()));
  // modify_if_different(
  //   s->flap_detection_on_ok,
  //   static_cast<int>(static_cast<bool>(
  //     obj.flap_detection_options() & configuration::service::ok)));
  // modify_if_different(
  //   s->flap_detection_on_warning,
  //   static_cast<int>(static_cast<bool>(
  //     obj.flap_detection_options() & configuration::service::warning)));
  // modify_if_different(
  //   s->flap_detection_on_unknown,
  //   static_cast<int>(static_cast<bool>(
  //     obj.flap_detection_options() & configuration::service::unknown)));
  // modify_if_different(
  //   s->flap_detection_on_critical,
  //   static_cast<int>(static_cast<bool>(
  //     obj.flap_detection_options() & configuration::service::critical)));
  // modify_if_different(
  //   s->process_performance_data,
  //   static_cast<int>(obj.process_perf_data()));
  // modify_if_different(
  //   s->check_freshness,
  //   static_cast<int>(obj.check_freshness()));
  // modify_if_different(
  //   s->freshness_threshold,
  //   static_cast<int>(obj.freshness_threshold()));
  // modify_if_different(
  //   s->accept_passive_service_checks,
  //   static_cast<int>(obj.checks_passive()));
  // modify_if_different(
  //   s->event_handler,
  //   NULL_IF_EMPTY(obj.event_handler()));
  // modify_if_different(
  //   s->checks_enabled,
  //   static_cast<int>(obj.checks_active()));
  // modify_if_different(
  //   s->retain_status_information,
  //   static_cast<int>(obj.retain_status_information()));
  // modify_if_different(
  //   s->retain_nonstatus_information,
  //   static_cast<int>(obj.retain_nonstatus_information()));
  // modify_if_different(
  //   s->notifications_enabled,
  //   static_cast<int>(obj.notifications_enabled()));
  // modify_if_different(
  //   s->obsess_over_service,
  //   static_cast<int>(obj.obsess_over_service()));
  // modify_if_different(s->notes, NULL_IF_EMPTY(obj.notes()));
  // modify_if_different(s->notes_url, NULL_IF_EMPTY(obj.notes_url()));
  // modify_if_different(s->action_url, NULL_IF_EMPTY(obj.action_url()));
  // modify_if_different(s->icon_image, NULL_IF_EMPTY(obj.icon_image()));
  // modify_if_different(
  //   s->icon_image_alt,
  //   NULL_IF_EMPTY(obj.icon_image_alt()));
  // modify_if_different(
  //   s->is_volatile,
  //   static_cast<int>(obj.is_volatile()));
  // service_other_props[std::make_pair(
  //                            *obj.hosts().begin(),
  //                            obj.service_description())].timezone = obj.timezone();
  // service_other_props[std::make_pair(
  //                            *obj.hosts().begin(),
  //                            obj.service_description())].host_id = obj.host_id();
  // service_other_props[std::make_pair(
  //                            *obj.hosts().begin(),
  //                            obj.service_description())].service_id = obj.service_id();
  // service_other_props[std::make_pair(
  //                            *obj.hosts().begin(),
  //                            obj.service_description())].acknowledgement_timeout
  //   = obj.get_acknowledgement_timeout() * config->interval_length();
  // service_other_props[std::make_pair(
  //                            *obj.hosts().begin(),
  //                            obj.service_description())].recovery_notification_delay
  //   = obj.recovery_notification_delay();

  // // Contacts.
  // if (obj.contacts() != obj_old.contacts()) {
  //   // Delete old contacts.
  //   deleter::listmember(s->contacts, &deleter::contactsmember);

  //   // Add contacts to host.
  //   for (set_string::const_iterator
  //          it(obj.contacts().begin()),
  //          end(obj.contacts().end());
  //        it != end;
  //        ++it)
  //     if (!add_contact_to_service(s, it->c_str()))
  //       throw (engine_error() << "Could not add contact '"
  //              << *it << "' to service '" << service_description
  //              << "' on host '" << host_name << "'");
  // }

  // // Contact groups.
  // if (obj.contactgroups() != obj_old.contactgroups()) {
  //   // Delete old contact groups.
  //   deleter::listmember(
  //     s->contact_groups,
  //     &deleter::contactgroupsmember);

  //   // Add contact groups to host.
  //   for (set_string::const_iterator
  //          it(obj.contactgroups().begin()),
  //          end(obj.contactgroups().end());
  //        it != end;
  //        ++it)
  //     if (!add_contactgroup_to_service(s, it->c_str()))
  //       throw (engine_error() << "Could not add contact group '"
  //              << *it << "' to service '" << service_description
  //              << "' on host '" << host_name << "'");
  // }

  // // Custom variables.
  // if (obj.customvariables() != obj_old.customvariables()) {
  //   // Delete old custom variables.
  //   remove_all_custom_variables_from_service(s);

  //   // Add custom variables.
  //   for (map_customvar::const_iterator
  //          it(obj.customvariables().begin()),
  //          end(obj.customvariables().end());
  //        it != end;
  //        ++it)
  //     if (!add_custom_variable_to_service(
  //            s,
  //            it->first.c_str(),
  //            it->second.c_str()))
  //       throw (engine_error() << "Could not add custom variable '"
  //              << it->first << "' to service '" << service_description
  //              << "' on host '" << host_name << "'");
  // }

  // // Notify event broker.
  // timeval tv(get_broker_timestamp(NULL));
  // broker_adaptive_service_data(
  //   NEBTYPE_SERVICE_UPDATE,
  //   NEBFLAG_NONE,
  //   NEBATTR_NONE,
  //   s,
  //   CMD_NONE,
  //   MODATTR_ALL,
  //   MODATTR_ALL,
  //   &tv);
  // return ;
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
  umap<std::pair<std::string, std::string>, shared_ptr<::service> >::iterator
    it(applier::state::instance().services().find(obj.key()));
  if (it != applier::state::instance().services().end()) {
    ::service* svc(it->second.get());

    // Remove service comments.
    delete_all_service_comments(
      host_name.c_str(),
      service_description.c_str());

    // Remove service downtimes.
    delete_downtime_by_hostname_service_description_start_time_comment(
      host_name.c_str(),
      service_description.c_str(),
      (time_t)0,
      NULL);

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
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving service '" << obj.service_description()
    << "' of host '" << *obj.hosts().begin() << "'.";

  try {
    // Find service.
    ::service& svc(
      *applier::state::instance().services_find(obj.key()));

    // Find host and adjust its counters.
    ::host& hst(
      *applier::state::instance().hosts_find(*obj.hosts().begin()));
    svc.set_host(&hst);
    hst.add_service(&svc);
    // XXX
    // ++hst->set_second->total_services;
    // hst->second->total_service_check_interval
    //   += static_cast<unsigned long>(it->second->check_interval);


    // Resolve check command.
    {
      // Get the command name.
      std::string command_name(obj.check_command().substr(
                                 0,
                                 obj.check_command().find_first_of('!')));

      // Set resolved command and arguments.
      svc.set_check_command(&find_command(command_name));
      svc.set_check_command_args(obj.check_command());
    }

    // Resolve check period.
    if (!obj.check_period().empty())
      svc.set_check_period(
        configuration::applier::state::instance().timeperiods_find(
          obj.check_period()).get());

    // Resolve contacts.

    // Resolve contact groups.

    // Resolve event handler.
    if (!obj.event_handler().empty()) {
      // Get the command name.
      std::string command_name(obj.event_handler().substr(
                                 0,
                                 obj.event_handler().find_first_of('!')));

      // Get command.
      svc.set_event_handler(&find_command(command_name));
      svc.set_event_handler_args(obj.event_handler());
    }

    // Resolve notification period.
    // XXX
  }
  catch (std::exception const& e) {
    throw (engine_error() << "Could not resolve service '"
           << obj.service_description() << "' of host '"
           << *obj.hosts().begin() << "': " << e.what());
  }

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
  if (!obj.host_id()
      || !obj.contacts_defined()
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
    if (!obj.host_id())
      obj.host_id(it->host_id());
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
