/*
** Copyright 2011-2013,2015-2017 Centreon
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

#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/retention/applier/service.hh"
#include "com/centreon/engine/retention/applier/utils.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::retention;

/**
 *  Update service list.
 *
 *  @param[in] config                The global configuration.
 *  @param[in] lst                   The service list to update.
 *  @param[in] scheduling_info_is_ok True if the retention is not
 *                                   outdated.
 */
void applier::service::apply(
       configuration::state const& config,
       list_service const& lst,
       bool scheduling_info_is_ok) {
  for (list_service::const_iterator it(lst.begin()), end(lst.end());
       it != end;
       ++it) {
    try {
      ::service& svc(*configuration::applier::state::instance().services_find(
        std::make_pair((*it)->host_name(), (*it)->service_description())));
      _update(config, **it, svc, scheduling_info_is_ok);
    }
    catch (...) {
      // ignore exception for the retention.
    }
  }
}

/**
 *  Update internal service base on service retention.
 *
 *  @param[in]      config                The global configuration.
 *  @param[in]      state                 The service retention state.
 *  @param[in, out] obj                   The service to update.
 *  @param[in]      scheduling_info_is_ok True if the retention is
 *                                        not outdated.
 */
void applier::service::_update(
       configuration::state const& config,
       retention::service const& state,
       ::service& obj,
       bool scheduling_info_is_ok) {
  if (state.modified_attributes().is_set()) {
    obj.set_modified_attributes(*state.modified_attributes());
    // Mask out attributes we don't want to retain.
    obj.set_modified_attributes(
          obj.get_modified_attributes()
          & ~config.retained_host_attribute_mask());
  }

  if (obj.get_retain_state_info()) {
    if (state.has_been_checked().is_set())
      obj.set_has_been_checked(*state.has_been_checked());
    if (state.check_execution_time().is_set())
      obj.set_execution_time(*state.check_execution_time());
    if (state.check_latency().is_set())
      obj.set_latency(*state.check_latency());
    if (state.check_type().is_set())
      obj.set_check_type(*state.check_type());
    if (state.current_state().is_set())
      obj.set_current_state(*state.current_state());
    if (state.last_state().is_set())
      obj.set_last_state(*state.last_state());
    if (state.last_hard_state().is_set())
      obj.set_last_hard_state(*state.last_hard_state());
    if (state.current_attempt().is_set())
      obj.set_current_attempt(*state.current_attempt());
    if (state.current_event_id().is_set())
      obj.set_current_event_id(*state.current_event_id());
    if (state.last_event_id().is_set())
      obj.set_last_event_id(*state.last_event_id());
    if (state.current_problem_id().is_set())
      obj.set_current_problem_id(*state.current_problem_id());
    if (state.last_problem_id().is_set())
      obj.set_last_problem_id(*state.last_problem_id());
    if (state.state_type().is_set())
      obj.set_current_state_type(*state.state_type());
    if (state.last_state_change().is_set())
      obj.set_last_state_change(*state.last_state_change());
    if (state.last_hard_state_change().is_set())
      obj.set_last_hard_state_change(*state.last_hard_state_change());
    if (state.last_time_ok().is_set())
      obj.set_last_time_ok(*state.last_time_ok());
    if (state.last_time_warning().is_set())
      obj.set_last_time_warning(*state.last_time_warning());
    if (state.last_time_unknown().is_set())
      obj.set_last_time_unknown(*state.last_time_unknown());
    if (state.last_time_critical().is_set())
      obj.set_last_time_critical(*state.last_time_critical());
    if (state.plugin_output().is_set())
      obj.set_output(*state.plugin_output());
    if (state.long_plugin_output().is_set())
      obj.set_long_output(*state.long_plugin_output());
    if (state.performance_data().is_set())
      obj.set_perfdata(*state.performance_data());
    // XXX
    // if (state.last_acknowledgement().is_set())
    //   obj.set_last_acknowledgement(*state.last_acknowledgement());
    if (state.last_check().is_set())
      obj.set_last_check(*state.last_check());
    if (state.next_check().is_set()
        && config.use_retained_scheduling_info()
        && scheduling_info_is_ok)
      obj.set_next_check(*state.next_check());
    if (state.check_options().is_set()
        && config.use_retained_scheduling_info()
        && scheduling_info_is_ok)
      obj.set_check_options(*state.check_options());
    if (state.notified_on_unknown().is_set())
      obj.set_notify_on_unknown(*state.notified_on_unknown());
    if (state.notified_on_warning().is_set())
      obj.set_notify_on_warning(*state.notified_on_warning());
    if (state.notified_on_critical().is_set())
      obj.set_notify_on_critical(*state.notified_on_critical());
    if (state.current_notification_number().is_set())
      obj.set_current_notification_number(*state.current_notification_number());
    // XXX
    // if (state.current_notification_id().is_set())
    //   obj.set_current_notification_id(*state.current_notification_id());
    if (state.last_notification().is_set())
      obj.set_last_notification(*state.last_notification());
    if (state.percent_state_change().is_set())
      obj.set_percent_state_change(*state.percent_state_change());
    // if (state.check_flapping_recovery_notification().is_set())
    //   obj.set_check_flapping_recovery_notification(*state.check_flapping_recovery_notification());
    if (state.state_history().is_set())
      utils::set_state_history(*state.state_history(), obj);
  }

  if (obj.get_retain_nonstate_info()) {
    if (state.problem_has_been_acknowledged().is_set())
      obj.set_acknowledged(*state.problem_has_been_acknowledged());
    if (state.acknowledgement_type().is_set())
      obj.set_acknowledgement_type(*state.acknowledgement_type());
    // XXX
    // if (state.notifications_enabled().is_set()
    //     && (obj.get_modified_attributes() & MODATTR_NOTIFICATIONS_ENABLED))
    //   obj.set_notifications_enabled(*state.notifications_enabled());
    if (state.active_checks_enabled().is_set()
        && (obj.get_modified_attributes() & MODATTR_ACTIVE_CHECKS_ENABLED))
      obj.set_active_checks_enabled(*state.active_checks_enabled());
    if (state.passive_checks_enabled().is_set()
        && (obj.get_modified_attributes() & MODATTR_PASSIVE_CHECKS_ENABLED))
      obj.set_passive_checks_enabled(*state.passive_checks_enabled());
    if (state.event_handler_enabled().is_set()
        && (obj.get_modified_attributes() & MODATTR_EVENT_HANDLER_ENABLED))
      obj.set_event_handler_enabled(*state.event_handler_enabled());
    if (state.flap_detection_enabled().is_set()
        && (obj.get_modified_attributes() & MODATTR_FLAP_DETECTION_ENABLED))
      obj.set_flap_detection_enabled(*state.flap_detection_enabled());
    if (state.process_performance_data().is_set()
        && (obj.get_modified_attributes() & MODATTR_PERFORMANCE_DATA_ENABLED))
      obj.set_process_perfdata(*state.process_performance_data());
    if (state.obsess_over_service().is_set()
        && (obj.get_modified_attributes() & MODATTR_OBSESSIVE_HANDLER_ENABLED))
      obj.set_ocp_enabled(*state.obsess_over_service());
    // XXX
    // if (state.check_command().is_set()
    //     && (obj.modified_attributes & MODATTR_CHECK_COMMAND)) {
    //   if (utils::is_command_exist(*state.check_command()))
    //     string::setstr(obj.service_check_command, *state.check_command());
    //   else
    //     obj.modified_attributes -= MODATTR_CHECK_COMMAND;
    // }
    // if (state.check_period().is_set()
    //     && (obj.modified_attributes & MODATTR_CHECK_TIMEPERIOD)) {
    //   if (is_timeperiod_exist(*state.check_period()))
    //     string::setstr(obj.check_period, *state.check_period());
    //   else
    //     obj.modified_attributes -= MODATTR_CHECK_TIMEPERIOD;
    // }
    // if (state.notification_period().is_set()
    //     && (obj.modified_attributes & MODATTR_NOTIFICATION_TIMEPERIOD)) {
    //   if (is_timeperiod_exist(*state.notification_period()))
    //     string::setstr(obj.notification_period, *state.notification_period());
    //   else
    //     obj.modified_attributes -= MODATTR_NOTIFICATION_TIMEPERIOD;
    // }
    // if (state.event_handler().is_set()
    //     && (obj.modified_attributes & MODATTR_EVENT_HANDLER_COMMAND)) {
    //   if (utils::is_command_exist(*state.event_handler()))
    //     string::setstr(obj.event_handler, *state.event_handler());
    //   else
    //     obj.modified_attributes -= MODATTR_EVENT_HANDLER_COMMAND;
    // }

    if (state.normal_check_interval().is_set()
        && (obj.get_modified_attributes() & MODATTR_NORMAL_CHECK_INTERVAL))
      obj.set_normal_check_interval(*state.normal_check_interval());
    if (state.retry_check_interval().is_set()
        && (obj.get_modified_attributes() & MODATTR_RETRY_CHECK_INTERVAL))
      obj.set_retry_check_interval(*state.retry_check_interval());
    if (state.max_attempts().is_set()
        && (obj.get_modified_attributes() & MODATTR_MAX_CHECK_ATTEMPTS)) {
      obj.set_max_attempts(*state.max_attempts());

      // Adjust current attempt number if in a hard state.
      if (obj.get_current_state_type() == HARD_STATE
          && obj.get_current_state() != STATE_OK
          && obj.get_current_attempt() > 1)
        obj.set_current_attempt(obj.get_max_attempts());
    }

    if (!state.customvariables().empty()
        && (obj.get_modified_attributes() & MODATTR_CUSTOM_VARIABLE)) {
      for (map_customvar::const_iterator
             it(state.customvariables().begin()),
             end(state.customvariables().end());
           it != end;
           ++it) {
        customvar var;
        var.set_name(it->first);
        var.set_value(it->second);
        obj.set_customvar(var);
      }
    }
  }
  // Adjust modified attributes if necessary.
  else
    obj.set_modified_attributes(MODATTR_NONE);

  bool allow_flapstart_notification(true);

  // Adjust modified attributes if no custom variable has been changed.
  if (obj.get_modified_attributes() & MODATTR_CUSTOM_VARIABLE) {
    bool at_least_one_modified(false);
    for (customvar_set::const_iterator
           it(obj.get_customvars().begin()),
           end(obj.get_customvars().end());
         it != end;
         ++it)
      if (it->second.get_modified())
        at_least_one_modified = true;
    if (!at_least_one_modified)
      obj.set_modified_attributes(
            obj.get_modified_attributes() - MODATTR_CUSTOM_VARIABLE);
  }

  // Calculate next possible notification time.
  // FIXME DBR: What to do here ?????
//  if (obj.get_current_state() != STATE_OK && obj.get_last_notification())
//    obj.set_next_notification(
//          get_next_service_notification_time(
//            &obj,
//            obj.get_last_notification()));

  // Fix old vars.
  if (!obj.get_has_been_checked()
      && obj.get_current_state_type() == SOFT_STATE)
    obj.set_current_state_type(HARD_STATE);

  // ADDED 01/23/2009 adjust current check attempt if service is
  // in hard problem state (max attempts may have changed in config
  // since restart).
  if (obj.get_current_state() != STATE_OK
      && obj.get_current_state_type() == HARD_STATE)
    obj.set_current_attempt(obj.get_max_attempts());

  // ADDED 02/20/08 assume same flapping state if large
  // install tweaks enabled.
  if (config.use_large_installation_tweaks())
    obj.set_flapping(state.is_flapping());
  // else use normal startup flap detection logic.
  else {
    // service was flapping before program started.
    // 11/10/07 don't allow flapping notifications to go out.
    allow_flapstart_notification = !state.is_flapping();

    // check for flapping.
    check_for_service_flapping(
      &obj,
      false,
      allow_flapstart_notification);

    // service was flapping before and isn't now, so clear
    // recovery check variable if service isn't flapping now.
    // XXX
    // if (state.is_flapping() && !obj.get_flapping())
    //   obj.set_check_flapping_recovery_notification(false);
  }

  // handle new vars added in 2.x.
  if (obj.get_last_hard_state_change())
    obj.set_last_hard_state_change(obj.get_last_state_change());

  // Handle recovery been sent
  // XXX
  // if (state.recovery_been_sent().is_set())
  //   obj.set_recovery_been_sent(*state.recovery_been_sent());

  // update service status.
  update_service_status(&obj, false);
}
