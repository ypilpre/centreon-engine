/*
** Copyright 2001-2009      Ethan Galstad
** Copyright 2011-2013,2017 Centreon
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

#include <iomanip>
#include <sstream>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/comment.hh"
#include "com/centreon/engine/statusdata.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/******************************************************************/
/******************** FLAP DETECTION FUNCTIONS ********************/
/******************************************************************/

/* detects service flapping */
void check_for_service_flapping(
       service* svc,
       int update) {
  int update_history = true;
  int is_flapping = false;
  int last_state_history_value = STATE_OK;
  double curved_changes = 0.0;
  double curved_percent_change = 0.0;
  double low_threshold = 0.0;
  double high_threshold = 0.0;
  double low_curve_value = 0.75;
  double high_curve_value = 1.25;

  /* large install tweaks skips all flap detection logic - including state change calculation */

  logger(dbg_functions, basic)
    << "check_for_service_flapping()";

  if (svc == NULL)
    return ;

  logger(dbg_flapping, more)
    << "Checking service '" << svc->get_description()
    << "' on host '" << svc->get_host_name() << "' for flapping...";

  /* if this is a soft service state and not a soft recovery, don't record this in the history */
  /* only hard states and soft recoveries get recorded for flap detection */
  if (svc->get_current_state_type() == SOFT_STATE
      && svc->get_current_state() != STATE_OK)
    return ;

  /* what threshold values should we use (global or service-specific)? */
  low_threshold =
    (svc->get_low_flap_threshold() <= 0.0)
    ? config->low_service_flap_threshold()
    : svc->get_low_flap_threshold();
  high_threshold =
    (svc->get_high_flap_threshold() <= 0.0)
    ? config->high_service_flap_threshold()
    : svc->get_high_flap_threshold();

  update_history = update;

  /* should we update state history for this state? */
  if (update_history == true) {
    if (svc->get_current_state() == STATE_OK
        && !svc->get_flap_detection_on_ok())
      update_history = false;
    if (svc->get_current_state() == STATE_WARNING
        && !svc->get_flap_detection_on_warning())
      update_history = false;
    if (svc->get_current_state() == STATE_UNKNOWN
        && !svc->get_flap_detection_on_unknown())
      update_history = false;
    if (svc->get_current_state() == STATE_CRITICAL
        && !svc->get_flap_detection_on_critical())
      update_history = false;
  }

  /* record current service state */
  if (update_history == true)
    /* record the current state in the state history */
    svc->add_historical_state(svc->get_current_state());

  /* calculate overall and curved percent state changes */
  last_state_history_value = svc->get_historical_state(0);
  for (int i(1); i < checks::checkable::historical_state_entries; ++i) {
    if (last_state_history_value != svc->get_historical_state(i))
      curved_changes
        += (((double)(i - 1)
             * (high_curve_value - low_curve_value))
            / ((double)(checks::checkable::historical_state_entries - 2))) + low_curve_value;
    last_state_history_value = svc->get_historical_state(i);
  }

  /* calculate overall percent change in state */
  curved_percent_change
    = (double)(((double)curved_changes * 100.0)
               / (double)(checks::checkable::historical_state_entries - 1));

  svc->set_percent_state_change(curved_percent_change);

  logger(dbg_flapping, most)
    << com::centreon::logging::setprecision(2)
    << "LFT=" << low_threshold
    << ", HFT=" << high_threshold
    << ", CPC=" << curved_percent_change
    << ", PSC=" << curved_percent_change << "%";

  /* don't do anything if we don't have flap detection enabled on a program-wide basis */
  if (config->enable_flap_detection() == false)
    return ;

  /* don't do anything if we don't have flap detection enabled for this service */
  if (!svc->get_flap_detection_enabled())
    return ;

  /* are we flapping, undecided, or what?... */

  /* we're undecided, so don't change the current flap state */
  if (curved_percent_change > low_threshold
      && curved_percent_change < high_threshold)
    return;
  /* we're below the lower bound, so we're not flapping */
  else if (curved_percent_change <= low_threshold)
    is_flapping = false;
  /* else we're above the upper bound, so we are flapping */
  else if (curved_percent_change >= high_threshold)
    is_flapping = true;

  logger(dbg_flapping, more)
    << com::centreon::logging::setprecision(2)
    << "Service " << (is_flapping == true ? "is" : "is not")
    << " flapping (" << curved_percent_change << "% state change).";

  /* did the service just start flapping? */
  if (is_flapping && !svc->get_flapping())
    set_service_flap(
      svc,
      curved_percent_change,
      high_threshold,
      low_threshold);

  /* did the service just stop flapping? */
  else if (!is_flapping && svc->get_flapping())
    clear_service_flap(
      svc,
      curved_percent_change,
      high_threshold,
      low_threshold);
  return;
}

/* detects host flapping */
void check_for_host_flapping(
       host* hst,
       int update,
       int actual_check) {
  int update_history = true;
  int is_flapping = false;
  int last_state_history_value = HOST_UP;
  unsigned long wait_threshold = 0L;
  double curved_changes = 0.0;
  double curved_percent_change = 0.0;
  time_t current_time = 0L;
  double low_threshold = 0.0;
  double high_threshold = 0.0;
  double low_curve_value = 0.75;
  double high_curve_value = 1.25;

  logger(dbg_functions, basic)
    << "check_for_host_flapping()";

  if (hst == NULL)
    return;

  logger(dbg_flapping, more)
    << "Checking host '" << hst->get_host_name() << "' for flapping...";

  time(&current_time);

  /* period to wait for updating archived state info if we have no state change */
  if (hst->get_services().empty())
    wait_threshold
      = static_cast<unsigned long>(hst->get_notification_interval()
                                   * config->interval_length());
  else
    wait_threshold
      = static_cast<unsigned long>((hst->get_total_service_check_interval()
                                    * config->interval_length())
                                   / hst->get_services().size());

  /* should we update state history for this state? */
  update_history = update;
  if (update_history == true) {
    if (hst->get_current_state() == HOST_UP
        && !hst->get_flap_detection_on_up())
      update_history = false;
    if (hst->get_current_state() == HOST_DOWN
        && !hst->get_flap_detection_on_down())
      update_history = false;
    if (hst->get_current_state() == HOST_UNREACHABLE
        && !hst->get_flap_detection_on_unreachable())
      update_history = false;
  }

  /* if we didn't have an actual check, only update if we've waited long enough */
  if (update_history
      && !actual_check
      && static_cast<unsigned long>(
           current_time - hst->get_last_historical_state_update()) < wait_threshold) {
    update_history = false;
  }

  /* what thresholds should we use (global or host-specific)? */
  low_threshold =
    (hst->get_low_flap_threshold() <= 0.0)
    ? config->low_host_flap_threshold()
    : hst->get_low_flap_threshold();
  high_threshold =
    (hst->get_high_flap_threshold() <= 0.0)
    ? config->high_host_flap_threshold()
    : hst->get_high_flap_threshold();

  /* record current host state */
  if (update_history == true) {
    /* update the last record time */
    hst->set_last_historical_state_update(current_time);

    /* record the current state in the state history */
    hst->add_historical_state(hst->get_current_state());
  }

  /* calculate overall changes in state */
  last_state_history_value = hst->get_historical_state(0);
  for (int i(1); i < checks::checkable::historical_state_entries; ++i) {
    if (last_state_history_value != hst->get_historical_state(i))
      curved_changes
        += (((double)(i - 1) * (high_curve_value - low_curve_value))
            / ((double)(checks::checkable::historical_state_entries - 2))) + low_curve_value;
    last_state_history_value = hst->get_historical_state(i);
  }

  /* calculate overall percent change in state */
  curved_percent_change
    = (double)(((double)curved_changes * 100.0)
               / (double)(checks::checkable::historical_state_entries - 1));
  hst->set_percent_state_change(curved_percent_change);

  logger(dbg_flapping, most)
    << com::centreon::logging::setprecision(2)
    << "LFT=" << low_threshold
    << ", HFT=" << high_threshold
    << ", CPC=" << curved_percent_change
    << ", PSC=" << curved_percent_change << "%";

  /* don't do anything if we don't have flap detection enabled on a program-wide basis */
  if (config->enable_flap_detection() == false)
    return ;

  /* don't do anything if we don't have flap detection enabled for this host */
  if (!hst->get_flap_detection_enabled())
    return ;

  /* are we flapping, undecided, or what?... */

  /* we're undecided, so don't change the current flap state */
  if (curved_percent_change > low_threshold
      && curved_percent_change < high_threshold)
    return ;

  /* we're below the lower bound, so we're not flapping */
  else if (curved_percent_change <= low_threshold)
    is_flapping = false;

  /* else we're above the upper bound, so we are flapping */
  else if (curved_percent_change >= high_threshold)
    is_flapping = true;

  logger(dbg_flapping, more)
    << "Host " << (is_flapping == true ? "is" : "is not")
    << " flapping (" << curved_percent_change << "% state change).";

  /* did the host just start flapping? */
  if (is_flapping && !hst->get_flapping())
    set_host_flap(
      hst,
      curved_percent_change,
      high_threshold,
      low_threshold);

  /* did the host just stop flapping? */
  else if (!is_flapping && hst->get_flapping())
    clear_host_flap(
      hst,
      curved_percent_change,
      high_threshold,
      low_threshold);
  return;
}

/******************************************************************/
/********************* FLAP HANDLING FUNCTIONS ********************/
/******************************************************************/

/* handles a service that is flapping */
void set_service_flap(
       service* svc,
       double percent_change,
       double high_threshold,
       double low_threshold) {
  logger(dbg_functions, basic)
    << "set_service_flap()";

  if (svc == NULL)
    return;

  logger(dbg_flapping, more)
    << "Service '" << svc->get_description() << "' on host '"
    << svc->get_host_name() << "' started flapping!";

  /* log a notice - this one is parsed by the history CGI */
  logger(log_runtime_warning, basic)
    << com::centreon::logging::setprecision(1)
    << "SERVICE FLAPPING ALERT: " << svc->get_host_name()
    << ";" << svc->get_description()
    << ";STARTED; Service appears to have started flapping ("
    << percent_change << "% change >= " << high_threshold
    << "% threshold)";

  /* add a non-persistent comment to the service */
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(1)
      << "Notifications for this service are being suppressed because "
    "it was detected as " << "having been flapping between different "
    "states (" << percent_change << "% change >= " << high_threshold
      << "% threshold).  When the service state stabilizes and the "
    "flapping " << "stops, notifications will be re-enabled.";

  // XXX
  // add_new_service_comment(
  //   FLAPPING_COMMENT,
  //   svc->get_host_name().c_str(),
  //   svc->get_description().c_str(),
  //   time(NULL),
  //   "(Centreon Engine Process)",
  //   oss.str().c_str(),
  //   0,
  //   COMMENTSOURCE_INTERNAL,
  //   false,
  //   (time_t)0,
  //   &(svc->flapping_comment_id));

  /* set the flapping indicator */
  svc->set_flapping(true);

  /* send data to event broker */
  broker_flapping_data(
    NEBTYPE_FLAPPING_START,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    SERVICE_FLAPPING,
    svc,
    percent_change,
    high_threshold,
    low_threshold,
    NULL);

  /* see if we should check to send a recovery notification out when flapping stops */
  // XXX
//  if (svc->current_state != STATE_OK
//      && svc->current_notification_number > 0)
//    svc->check_flapping_recovery_notification = true;
//  else
//    svc->check_flapping_recovery_notification = false;

  /* send a notification */
  if (allow_flapstart_notification) {
          ////////////////
          // FIXME DBR  //
          ////////////////
//    svc->notify(notifier::FLAPPINGSTART);
//    service_notification(
//      svc,
//      NOTIFICATION_FLAPPINGSTART,
//      NULL,
//      NULL,
//      NOTIFICATION_OPTION_NONE);
  }
}

/* handles a service that has stopped flapping */
void clear_service_flap(
       service* svc,
       double percent_change,
       double high_threshold,
       double low_threshold) {

  logger(dbg_functions, basic)
    << "clear_service_flap()";

  if (svc == NULL)
    return;

  logger(dbg_flapping, more)
    << "Service '" << svc->get_description() << "' on host '"
    << svc->get_host_name() << "' stopped flapping.";

  /* log a notice - this one is parsed by the history CGI */
  logger(log_info_message, basic)
    << com::centreon::logging::setprecision(1)
    << "SERVICE FLAPPING ALERT: " << svc->get_host_name()
    << ";" << svc->get_description()
    << ";STOPPED; Service appears to have stopped flapping ("
    << percent_change << "% change < " << low_threshold
    << "% threshold)";

  /* delete the comment we added earlier */
  // XXX
  // if (svc->flapping_comment_id != 0)
  //   delete_service_comment(svc->flapping_comment_id);
  // svc->flapping_comment_id = 0;

  /* clear the flapping indicator */
  svc->set_flapping(false);

  /* send data to event broker */
  broker_flapping_data(
    NEBTYPE_FLAPPING_STOP,
    NEBFLAG_NONE,
    NEBATTR_FLAPPING_STOP_NORMAL,
    SERVICE_FLAPPING,
    svc,
    percent_change,
    high_threshold,
    low_threshold,
    NULL);

  /* send a notification */
          ////////////////
          // FIXME DBR  //
          ////////////////
//  svc->notify(notifier::FLAPPINGSTOP);
//  service_notification(
//    svc,
//    NOTIFICATION_FLAPPINGSTOP,
//    NULL,
//    NULL,
//    NOTIFICATION_OPTION_NONE);

  /* should we send a recovery notification? */
  // XXX
//  if (svc->check_flapping_recovery_notification == true
//      && svc->current_state == STATE_OK)
          ////////////////
          // FIXME DBR  //
          ////////////////
//    svc->notify(notifier::PROBLEM);
//    service_notification(
//      svc,
//      NOTIFICATION_NORMAL,
//      NULL,
//      NULL,
//      NOTIFICATION_OPTION_NONE);

  /* clear the recovery notification flag */
  // XXX
//  svc->check_flapping_recovery_notification = false;
  return;
}

/* handles a host that is flapping */
void set_host_flap(
       host* hst,
       double percent_change,
       double high_threshold,
       double low_threshold) {
  logger(dbg_functions, basic)
    << "set_host_flap()";

  if (hst == NULL)
    return;

  logger(dbg_flapping, more)
    << "Host '" << hst->get_host_name() << "' started flapping!";

  /* log a notice - this one is parsed by the history CGI */
  logger(log_runtime_warning, basic)
    << com::centreon::logging::setprecision(1)
    << "HOST FLAPPING ALERT: " << hst->get_host_name()
    << ";STARTED; Host appears to have started flapping ("
    << percent_change << "% change > "
    << high_threshold << "% threshold)";

  /* add a non-persistent comment to the host */
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(1)
      << "Notifications for this host are being suppressed because it "
      "was detected as " << "having been flapping between different "
      "states (" << percent_change << "% change > " << high_threshold
      << "% threshold).  When the host state stabilizes and the "
      << "flapping stops, notifications will be re-enabled.";

  // XXX
  // add_new_host_comment(
  //   FLAPPING_COMMENT,
  //   hst->name,
  //   time(NULL),
  //   "(Centreon Engine Process)",
  //   oss.str().c_str(),
  //   0,
  //   COMMENTSOURCE_INTERNAL,
  //   false,
  //   (time_t)0,
  //   &(hst->flapping_comment_id));

  /* set the flapping indicator */
  hst->set_flapping(true);

  /* send data to event broker */
  broker_flapping_data(
    NEBTYPE_FLAPPING_START,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    HOST_FLAPPING,
    hst,
    percent_change,
    high_threshold,
    low_threshold,
    NULL);

  /* see if we should check to send a recovery notification out when flapping stops */
  // XXX
  // if (hst->current_state != HOST_UP
  //     && hst->current_notification_number > 0)
  //   hst->check_flapping_recovery_notification = true;
  // else
  //   hst->check_flapping_recovery_notification = false;

  /* send a notification */
  // if (allow_flapstart_notification) {
          ////////////////
          // FIXME DBR  //
          ////////////////
//    hst->notify(notifier::FLAPPINGSTART);
//    host_notification(
//      hst,
//      NOTIFICATION_FLAPPINGSTART,
//      NULL,
//      NULL,
//      NOTIFICATION_OPTION_NONE);
  // }
}

/* handles a host that has stopped flapping */
void clear_host_flap(
       host* hst,
       double percent_change,
       double high_threshold,
       double low_threshold) {

  logger(dbg_functions, basic)
    << "clear_host_flap()";

  if (hst == NULL)
    return;

  logger(dbg_flapping, basic)
    << "Host '" << hst->get_host_name() << "' stopped flapping.";

  /* log a notice - this one is parsed by the history CGI */
  logger(log_info_message, basic)
    << com::centreon::logging::setprecision(1)
    << "HOST FLAPPING ALERT: " << hst->get_host_name()
    << ";STOPPED; Host appears to have stopped flapping ("
    << percent_change << "% change < "
    << low_threshold << "% threshold)";

  /* delete the comment we added earlier */
  // XXX
  // if (hst->flapping_comment_id != 0)
  //   delete_host_comment(hst->flapping_comment_id);
  // hst->flapping_comment_id = 0;

  /* clear the flapping indicator */
  hst->set_flapping(false);

  /* send data to event broker */
  broker_flapping_data(
    NEBTYPE_FLAPPING_STOP,
    NEBFLAG_NONE,
    NEBATTR_FLAPPING_STOP_NORMAL,
    HOST_FLAPPING,
    hst,
    percent_change,
    high_threshold,
    low_threshold,
    NULL);

  /* send a notification */
          ////////////////
          // FIXME DBR  //
          ////////////////
//  hst->notify(notifier::FLAPPINGSTOP);
//  host_notification(
//    hst,
//    NOTIFICATION_FLAPPINGSTOP,
//    NULL,
//    NULL,
//    NOTIFICATION_OPTION_NONE);

  /* should we send a recovery notification? */
  // XXX
  // if (hst->check_flapping_recovery_notification
  //     && hst->current_state == HOST_UP) {
          ////////////////
          // FIXME DBR  //
          ////////////////
//    hst->notify(notifier::PROBLEM);
//    host_notification(
//      hst,
//      NOTIFICATION_NORMAL,
//      NULL,
//      NULL,
//      NOTIFICATION_OPTION_NONE);
  // }

  /* clear the recovery notification flag */
  // hst->check_flapping_recovery_notification = false;
  return;
}

/******************************************************************/
/***************** FLAP DETECTION STATUS FUNCTIONS ****************/
/******************************************************************/

/* enables flap detection on a program wide basis */
void enable_flap_detection_routines() {
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic)
    << "enable_flap_detection_routines()";

  /* bail out if we're already set */
  if (config->enable_flap_detection() == true)
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;
  modified_service_process_attributes |= attr;

  /* set flap detection flag */
  config->enable_flap_detection(true);

  /* send data to event broker */
  broker_adaptive_program_data(
    NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
    NEBFLAG_NONE, NEBATTR_NONE,
    CMD_NONE,
    attr,
    modified_host_process_attributes,
    attr,
    modified_service_process_attributes,
    NULL);

  /* update program status */
  update_program_status(false);

  /* check for flapping */
  for (umap<std::string, com::centreon::shared_ptr< ::host> >::iterator
         it(configuration::applier::state::instance().hosts().begin()),
         end(configuration::applier::state::instance().hosts().end());
       it != end;
       ++it)
    check_for_host_flapping(it->second.get(), false, false);
  for (umap<std::pair<std::string, std::string>, com::centreon::shared_ptr< ::service> >::iterator
         it(configuration::applier::state::instance().services().begin()),
         end(configuration::applier::state::instance().services().end());
       it != end;
       ++it)
    check_for_service_flapping(it->second.get(), false);
  return ;
}

/* disables flap detection on a program wide basis */
void disable_flap_detection_routines() {
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic)
    << "disable_flap_detection_routines()";

  /* bail out if we're already set */
  if (config->enable_flap_detection() == false)
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;
  modified_service_process_attributes |= attr;

  /* set flap detection flag */
  config->enable_flap_detection(false);

  /* send data to event broker */
  broker_adaptive_program_data(
    NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
    NEBFLAG_NONE, NEBATTR_NONE,
    CMD_NONE,
    attr,
    modified_host_process_attributes,
    attr,
    modified_service_process_attributes,
    NULL);

  /* update program status */
  update_program_status(false);

  /* handle the details... */
  for (umap<std::string, com::centreon::shared_ptr< ::host> >::iterator
         it(configuration::applier::state::instance().hosts().begin()),
         end(configuration::applier::state::instance().hosts().end());
       it != end;
       ++it)
    handle_host_flap_detection_disabled(it->second.get());
  for (umap<std::pair<std::string, std::string>, com::centreon::shared_ptr< ::service> >::iterator
         it(configuration::applier::state::instance().services().begin()),
         end(configuration::applier::state::instance().services().end());
       it != end;
       ++it)
    handle_service_flap_detection_disabled(it->second.get());
  return;
}

/* enables flap detection for a specific host */
void enable_host_flap_detection(host* hst) {
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic)
    << "enable_host_flap_detection()";

  if (hst == NULL)
    return;

  logger(dbg_flapping, more) << "Enabling flap detection for host '"
    << hst->get_host_name() << "'.";

  /* nothing to do... */
  if (hst->get_flap_detection_enabled())
    return ;

  /* set the attribute modified flag */
  hst->set_modified_attributes(hst->get_modified_attributes() | attr);

  /* set the flap detection enabled flag */
  hst->set_flap_detection_enabled(true);

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->get_modified_attributes(),
    NULL);

  /* check for flapping */
  check_for_host_flapping(hst, false, false);

  /* update host status */
  update_host_status(hst, false);
  return;
}

/* disables flap detection for a specific host */
void disable_host_flap_detection(host* hst) {
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic)
    << "disable_host_flap_detection()";

  if (hst == NULL)
    return;

  logger(dbg_functions, more) << "Disabling flap detection for host '"
    << hst->get_host_name() << "'.";

  /* nothing to do... */
  if (!hst->get_flap_detection_enabled())
    return ;

  /* set the attribute modified flag */
  hst->set_modified_attributes(hst->get_modified_attributes() | attr);

  /* set the flap detection enabled flag */
  hst->set_flap_detection_enabled(false);

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->get_modified_attributes(),
    NULL);

  /* handle the details... */
  handle_host_flap_detection_disabled(hst);
  return ;
}

/* handles the details for a host when flap detection is disabled (globally or per-host) */
void handle_host_flap_detection_disabled(host* hst) {
  logger(dbg_functions, basic)
    << "handle_host_flap_detection_disabled()";

  if (hst == NULL)
    return;

  /* if the host was flapping, remove the flapping indicator */
  if (hst->get_flapping()) {
    hst->set_flapping(false);

    /* delete the original comment we added earlier */
    // XXX
    // if (hst->flapping_comment_id != 0)
    //   delete_host_comment(hst->flapping_comment_id);
    // hst->flapping_comment_id = 0;

    /* log a notice - this one is parsed by the history CGI */
    logger(log_info_message, basic)
      << "HOST FLAPPING ALERT: " << hst->get_host_name()
      << ";DISABLED; Flap detection has been disabled";

    /* send data to event broker */
    broker_flapping_data(
      NEBTYPE_FLAPPING_STOP,
      NEBFLAG_NONE,
      NEBATTR_FLAPPING_STOP_DISABLED,
      HOST_FLAPPING,
      hst,
      hst->get_percent_state_change(),
      0.0,
      0.0,
      NULL);

    /* send a notification */
          ////////////////
          // FIXME DBR  //
          ////////////////
//    hst->notify(notifier::FLAPPINGDISABLED);
//    host_notification(
//      hst,
//      NOTIFICATION_FLAPPINGDISABLED,
//      NULL,
//      NULL,
//      NOTIFICATION_OPTION_NONE);

    /* should we send a recovery notification? */
    // XXX
    // if (hst->check_flapping_recovery_notification == true
    //     && hst->current_state == HOST_UP) {
          ////////////////
          // FIXME DBR  //
          ////////////////
//      hst->notify(notifier::PROBLEM);
//      host_notification(
//        hst,
//        NOTIFICATION_NORMAL,
//        NULL,
//        NULL,
//        NOTIFICATION_OPTION_NONE);
    // }

    /* clear the recovery notification flag */
    // hst->check_flapping_recovery_notification = false;
  }

  /* update host status */
  update_host_status(hst, false);
  return;
}

/* enables flap detection for a specific service */
void enable_service_flap_detection(service* svc) {
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic)
    << "enable_service_flap_detection()";

  if (svc == NULL)
    return;

  logger(dbg_flapping, more)
    << "Enabling flap detection for service '" << svc->get_description()
    << "' on host '" << svc->get_host_name() << "'.";

  /* nothing to do... */
  if (svc->get_flap_detection_enabled())
    return ;

  /* set the attribute modified flag */
  svc->set_modified_attributes(svc->get_modified_attributes() | attr);

  /* set the flap detection enabled flag */
  svc->set_flap_detection_enabled(true);

  /* send data to event broker */
  broker_adaptive_service_data(
    NEBTYPE_ADAPTIVESERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc,
    CMD_NONE,
    attr,
    svc->get_modified_attributes(),
    NULL);

  /* check for flapping */
  check_for_service_flapping(svc, false);

  /* update service status */
  update_service_status(svc, false);
  return;
}

/* disables flap detection for a specific service */
void disable_service_flap_detection(service* svc) {
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic)
    << "disable_service_flap_detection()";

  if (svc == NULL)
    return;

  logger(dbg_flapping, more) << "Disabling flap detection for service '"
    << svc->get_description() << "' on host '"
    << svc->get_host_name() << "'.";

  /* nothing to do... */
  if (!svc->get_flap_detection_enabled())
    return ;

  /* set the attribute modified flag */
  svc->set_modified_attributes(svc->get_modified_attributes() | attr);

  /* set the flap detection enabled flag */
  svc->set_flap_detection_enabled(false);

  /* send data to event broker */
  broker_adaptive_service_data(
    NEBTYPE_ADAPTIVESERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc,
    CMD_NONE,
    attr,
    svc->get_modified_attributes(),
    NULL);

  /* handle the details... */
  handle_service_flap_detection_disabled(svc);
  return;
}

/* handles the details for a service when flap detection is disabled (globally or per-service) */
void handle_service_flap_detection_disabled(service* svc) {
  logger(dbg_functions, basic)
    << "handle_service_flap_detection_disabled()";

  if (svc == NULL)
    return;

  /* if the service was flapping, remove the flapping indicator */
  if (svc->get_flapping()) {
    svc->set_flapping(false);

    // XXX
    // /* delete the original comment we added earlier */
    // if (svc->flapping_comment_id != 0)
    //   delete_service_comment(svc->flapping_comment_id);
    // svc->flapping_comment_id = 0;

    /* log a notice - this one is parsed by the history CGI */
    logger(log_info_message, basic)
      << "SERVICE FLAPPING ALERT: " << svc->get_host_name()
      << ";" << svc->get_description()
      << ";DISABLED; Flap detection has been disabled";

    /* send data to event broker */
    broker_flapping_data(
      NEBTYPE_FLAPPING_STOP,
      NEBFLAG_NONE,
      NEBATTR_FLAPPING_STOP_DISABLED,
      SERVICE_FLAPPING,
      svc,
      svc->get_percent_state_change(),
      0.0,
      0.0,
      NULL);

    /* send a notification */
          ////////////////
          // FIXME DBR  //
          ////////////////
//    svc->notify(notifier::FLAPPINGDISABLED);
//    service_notification(
//      svc,
//      NOTIFICATION_FLAPPINGDISABLED,
//      NULL,
//      NULL,
//      NOTIFICATION_OPTION_NONE);

    /* should we send a recovery notification? */
    // XXX
    // if (svc->check_flapping_recovery_notification
    //     && svc->current_state == STATE_OK) {
          ////////////////
          // FIXME DBR  //
          ////////////////
//      svc->notify(notifier::PROBLEM);
//      service_notification(
//        svc,
//        NOTIFICATION_NORMAL,
//        NULL,
//        NULL,
//        NOTIFICATION_OPTION_NONE);
    // }

    /* clear the recovery notification flag */
    // svc->check_flapping_recovery_notification = false;
  }

  /* update service status */
  update_service_status(svc, false);
  return;
}
