/*
** Copyright 1999-2010           Ethan Galstad
** Copyright 2011-2013,2016-2017 Centreon
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

#include <sstream>
#include <utility>
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros/clear_service.hh"
#include "com/centreon/engine/macros/clear_servicegroup.hh"
#include "com/centreon/engine/macros/defines.hh"
#include "com/centreon/engine/macros/grab.hh"
#include "com/centreon/engine/macros/grab_service.hh"
#include "com/centreon/engine/macros/misc.hh"
#include "com/centreon/engine/objects/objectlist.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/shared_ptr.hh"
#include "com/centreon/unordered_hash.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::macros;
using namespace com::centreon::engine::logging;

/**************************************
*                                     *
*           Local Functions           *
*                                     *
**************************************/

/**
 *  Extract service check type.
 *
 *  @param[in] svc Service object.
 *  @param[in] mac Unused.
 *
 *  @return Newly allocated string containing either "PASSIVE" or
 *          "ACTIVE".
 */
static char const* get_service_check_type(service& svc, nagios_macros* mac) {
  (void)mac;
  return (string::dup(
            (SERVICE_CHECK_PASSIVE == svc.get_check_type()
            ? "PASSIVE"
            : "ACTIVE")));
}

/**
 *  Extract service group names.
 *
 *  @param[in] svc Target service.
 *  @param[in] mac Unused.
 *
 *  @return List of names of groups associated with this service.
 */
static char* get_service_group_names(service& svc, nagios_macros* mac) {
  (void)mac;

  // Find all servicegroups this service is associated with.
  std::string buf;
  for (servicegroup_set::const_iterator
         it(svc.get_groups().begin()),
         end(svc.get_groups().end());
       it != end;
       ++it) {
    servicegroup* temp_servicegroup(it->second);
    if (!buf.empty())
      buf.append(",");
    buf.append(temp_servicegroup->group_name);
  }
  return (string::dup(buf));
}

/**
 *  Extract service state.
 *
 *  @param[in] stateid  State to convert.
 *
 *  @return Newly allocated string with service state as plain text.
 */
static char* get_service_state(int stateid) {
  char const* state;
  switch (stateid) {
   case STATE_OK:
    state = "OK";
    break ;
   case STATE_WARNING:
    state = "WARNING";
    break ;
   case STATE_CRITICAL:
    state = "CRITICAL";
    break ;
   default:
    state = "UNKNOWN";
  }
  return (string::dup(state));
}
static char const* get_current_service_state(service& svc, nagios_macros* mac) {
  (void)mac;
  return (get_service_state(svc.get_current_state()));
}
static char const* get_last_service_state(service& svc, nagios_macros* mac) {
  (void)mac;
  return (get_service_state(svc.get_last_state()));
}

/**************************************
*                                     *
*         Redirection Object          *
*                                     *
**************************************/

// Redirection object.
struct grab_service_redirection {
  typedef umap<unsigned int, std::pair<com::centreon::shared_ptr<grabber<service> >, bool> > entry;
  entry routines;
  grab_service_redirection() {
    // Description.
    routines[MACRO_SERVICEDESC].first =
      new member_grabber<service, std::string const&>(&service::get_description);
    routines[MACRO_SERVICEDESC].second = true;
    // Display name.
    routines[MACRO_SERVICEDISPLAYNAME].first =
      new member_grabber<service, std::string const&>(&service::get_display_name);
    routines[MACRO_SERVICEDISPLAYNAME].second = true;
    // Output.
    routines[MACRO_SERVICEOUTPUT].first =
      new member_grabber<service, std::string const&>(&service::get_output);
    routines[MACRO_SERVICEOUTPUT].second = true;
    // Long output.
    routines[MACRO_LONGSERVICEOUTPUT].first =
      new member_grabber<service, std::string const&>(&service::get_long_output);
    routines[MACRO_LONGSERVICEOUTPUT].second = true;
    // Perfdata.
    routines[MACRO_SERVICEPERFDATA].first =
      new member_grabber<service, std::string const&>(&service::get_perfdata);
    routines[MACRO_SERVICEPERFDATA].second = true;
    // XXX
    // // Check command.
    // routines[MACRO_SERVICECHECKCOMMAND].first = &get_member_as_string<service, char*, &service::service_check_command>;
    // routines[MACRO_SERVICECHECKCOMMAND].second = true;
    // Check type.
    routines[MACRO_SERVICECHECKTYPE].first =
      new function_grabber<service>(&get_service_check_type);
    routines[MACRO_SERVICECHECKTYPE].second = true;
    // State type.
    routines[MACRO_SERVICESTATETYPE].first =
      new function_grabber<service>(&get_state_type<service>);
    routines[MACRO_SERVICESTATETYPE].second = true;
    // State.
    routines[MACRO_SERVICESTATE].first =
      new function_grabber<service>(&get_current_service_state);
    routines[MACRO_SERVICESTATE].second = true;
    // State ID.
    routines[MACRO_SERVICESTATEID].first =
      new member_grabber<service, int>(&service::get_current_state);
    routines[MACRO_SERVICESTATEID].second = true;
    // Last state.
    routines[MACRO_LASTSERVICESTATE].first =
      new function_grabber<service>(&get_last_service_state);
    routines[MACRO_LASTSERVICESTATE].second = true;
    // Last state ID.
    routines[MACRO_LASTSERVICESTATEID].first =
      new member_grabber<service, int>(&service::get_last_state);
    routines[MACRO_LASTSERVICESTATEID].second = true;
    // Is volatile.
    routines[MACRO_SERVICEISVOLATILE].first =
      new member_grabber<service, bool>(&service::get_volatile);
    routines[MACRO_SERVICEISVOLATILE].second = true;
    // Attempt.
    routines[MACRO_SERVICEATTEMPT].first =
      new member_grabber<service, int>(&service::get_current_attempt);
    routines[MACRO_SERVICEATTEMPT].second = true;
    // Max attempts.
    routines[MACRO_MAXSERVICEATTEMPTS].first =
      new member_grabber<service, int>(&service::get_max_attempts);
    routines[MACRO_MAXSERVICEATTEMPTS].second = true;
    // Execution time.
    routines[MACRO_SERVICEEXECUTIONTIME].first =
      new member_grabber<service, double>(&service::get_execution_time);
    routines[MACRO_SERVICEEXECUTIONTIME].second = true;
    // Latency.
    routines[MACRO_SERVICELATENCY].first =
      new member_grabber<service, double>(&service::get_latency);
    routines[MACRO_SERVICELATENCY].second = true;
    // Last check.
    routines[MACRO_LASTSERVICECHECK].first =
      new member_grabber<service, time_t>(&service::get_last_check);
    routines[MACRO_LASTSERVICECHECK].second = true;
    // Last state change.
    routines[MACRO_LASTSERVICESTATECHANGE].first =
      new member_grabber<service, time_t>(&service::get_last_state_change);
    routines[MACRO_LASTSERVICESTATECHANGE].second = true;
    // Last time ok.
    routines[MACRO_LASTSERVICEOK].first =
      new member_grabber<service, time_t>(&service::get_last_time_ok);
    routines[MACRO_LASTSERVICEOK].second = true;
    // Last time warning.
    routines[MACRO_LASTSERVICEWARNING].first =
      new member_grabber<service, time_t>(&service::get_last_time_warning);
    routines[MACRO_LASTSERVICEWARNING].second = true;
    // Last time unknown.
    routines[MACRO_LASTSERVICEUNKNOWN].first =
      new member_grabber<service, time_t>(&service::get_last_time_unknown);
    routines[MACRO_LASTSERVICEUNKNOWN].second = true;
    // Last time critical.
    routines[MACRO_LASTSERVICECRITICAL].first =
      new member_grabber<service, time_t>(&service::get_last_time_critical);
    routines[MACRO_LASTSERVICECRITICAL].second = true;
    // Downtime.
    routines[MACRO_SERVICEDOWNTIME].first =
      new member_grabber<service, int>(&service::get_scheduled_downtime_depth);
    routines[MACRO_SERVICEDOWNTIME].second = true;
    // Percent state change.
    routines[MACRO_SERVICEPERCENTCHANGE].first =
      new member_grabber<service, double>(&service::get_percent_state_change);
    routines[MACRO_SERVICEPERCENTCHANGE].second = true;
    // XXX
    // // Duration.
    // routines[MACRO_SERVICEDURATION].first = &get_duration<service>;
    // routines[MACRO_SERVICEDURATION].second = true;
    // // Duration in seconds.
    // routines[MACRO_SERVICEDURATIONSEC].first = &get_duration_sec<service>;
    // routines[MACRO_SERVICEDURATIONSEC].second = true;
    // Notification number.
    routines[MACRO_SERVICENOTIFICATIONNUMBER].first =
      new member_grabber<service, int>(&service::get_current_notification_number);
    routines[MACRO_SERVICENOTIFICATIONNUMBER].second = true;
    // Notification ID.
    routines[MACRO_SERVICENOTIFICATIONID].first =
      new member_grabber<service, int>(&service::get_current_notification_id);
    routines[MACRO_SERVICENOTIFICATIONID].second = true;
    // Event ID.
    routines[MACRO_SERVICEEVENTID].first =
      new member_grabber<service, int>(&service::get_current_event_id);
    routines[MACRO_SERVICEEVENTID].second = true;
    // Last event ID.
    routines[MACRO_LASTSERVICEEVENTID].first =
      new member_grabber<service, int>(&service::get_last_event_id);
    routines[MACRO_LASTSERVICEEVENTID].second = true;
    // Problem ID.
    routines[MACRO_SERVICEPROBLEMID].first =
      new member_grabber<service, int>(&service::get_current_problem_id);
    routines[MACRO_SERVICEPROBLEMID].second = true;
    // Last problem ID.
    routines[MACRO_LASTSERVICEPROBLEMID].first =
      new member_grabber<service, int>(&service::get_last_problem_id);
    routines[MACRO_LASTSERVICEPROBLEMID].second = true;
    // Action URL.
    routines[MACRO_SERVICEACTIONURL].first =
      new function_grabber<service>(&get_action_url<service>);
    routines[MACRO_SERVICEACTIONURL].second = true;
    // Notes URL.
    routines[MACRO_SERVICENOTESURL].first =
      new function_grabber<service>(&get_notes_url<service>);
    routines[MACRO_SERVICENOTESURL].second = true;
    // Notes.
    routines[MACRO_SERVICENOTES].first =
      new function_grabber<service>(&get_notes<service>);
    routines[MACRO_SERVICENOTES].second = true;
    // Group names.
    routines[MACRO_SERVICEGROUPNAMES].first =
      new function_grabber<service>(
        (char const* (*)(service&, nagios_macros*))&get_service_group_names);
    routines[MACRO_SERVICEGROUPNAMES].second = true;
    // XXX
    // // Acknowledgement author.
    // routines[MACRO_SERVICEACKAUTHOR].first = &get_macro_copy<service, MACRO_SERVICEACKAUTHOR>;
    // routines[MACRO_SERVICEACKAUTHOR].second = true;
    // // Acknowledgement author name.
    // routines[MACRO_SERVICEACKAUTHORNAME].first = &get_macro_copy<service, MACRO_SERVICEACKAUTHORNAME>;
    // routines[MACRO_SERVICEACKAUTHORNAME].second = true;
    // // Acknowledgement author alias.
    // routines[MACRO_SERVICEACKAUTHORALIAS].first = &get_macro_copy<service, MACRO_SERVICEACKAUTHORALIAS>;
    // routines[MACRO_SERVICEACKAUTHORALIAS].second = true;
    // // Acknowledgement comment.
    // routines[MACRO_SERVICEACKCOMMENT].first = &get_macro_copy<service, MACRO_SERVICEACKCOMMENT>;
    // routines[MACRO_SERVICEACKCOMMENT].second = true;
    // // Acknowledgement comment.
    // routines[MACRO_SERVICEACKCOMMENT].first = &get_macro_copy<service, MACRO_SERVICEACKCOMMENT>;
    // routines[MACRO_SERVICEACKCOMMENT].second = true;
    // // Acknowledgement comment.
    // routines[MACRO_SERVICETIMEZONE].first = &get_service_macro_timezone;
    // routines[MACRO_SERVICETIMEZONE].second = true;
    // Service id.
    routines[MACRO_SERVICEID].first =
      new member_grabber<service, unsigned int>(&service::get_id);
    routines[MACRO_SERVICEID].second = true;
  }
} static const redirector;

/**************************************
*                                     *
*          Global Functions           *
*                                     *
**************************************/

extern "C" {
/**
 *  Grab a standard service macro.
 *
 *  @param[out] mac        Macro array.
 *  @param[in]  macro_type Macro to dump.
 *  @param[in]  svc        Target service.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer should be free
 *                         by caller.
 *
 *  @return OK on success.
 */
int grab_standard_service_macro_r(
      nagios_macros* mac,
      int macro_type,
      service* svc,
      char** output,
      int* free_macro) {
  // Check that function was called with valid arguments.
  int retval;
  if (svc && output && free_macro) {
    grab_service_redirection::entry::const_iterator it(
      redirector.routines.find(macro_type));
    // Found matching routine.
    if (it != redirector.routines.end()) {
      // Call routine.
      *output = const_cast<char*>((*it->second.first)(*svc, mac));

      // Set the free macro flag.
      *free_macro = it->second.second;

      // Successful execution.
      retval = OK;
    }
    // Non-existent macro.
    else {
      logger(dbg_macros, basic)
        << "UNHANDLED SERVICE MACRO #" << macro_type
        << "! THIS IS A BUG!";
      retval = ERROR;
    }
  }
  else
    retval = ERROR;

  return (retval);
}

/**
 *  Grab a standard service macro for global macros.
 *
 *  @param[in]  macro_type Macro to dump.
 *  @param[in]  svc        Target service.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer should be free
 *                         by caller.
 *
 *  @return OK on success.
 *
 *  @see grab_standard_service_macro_r
 */
int grab_standard_service_macro(
      int macro_type,
      service* svc,
      char** output,
      int* free_macro) {
  return (grab_standard_service_macro_r(
            get_global_macros(),
            macro_type,
            svc,
            output,
            free_macro));
}

/**
 *  Grab macros that are specific to a service.
 *
 *  @param[in] mac Macros object.
 *  @param[in] svc Service pointer.
 *
 *  @return OK on success.
 */
int grab_service_macros_r(nagios_macros* mac, service* svc) {
  // Clear service-related macros.
  clear_service_macros_r(mac);
  clear_servicegroup_macros_r(mac);

  // Save pointer for later.
  mac->service_ptr = svc;
  mac->servicegroup_ptr = NULL;

  if (svc == NULL)
    return (ERROR);

  // Save first/primary servicegroup pointer for later.
  // XXX
  // if (svc->servicegroups_ptr)
  //   mac->servicegroup_ptr
  //     = static_cast<servicegroup*>(svc->servicegroups_ptr->object_ptr);

  return (OK);
}

/**
 *  Grab macros that are specific to a service.
 *
 *  @param[in] svc Service pointer.
 *
 *  @return OK on success.
 *
 *  @see grab_service_macros_r
 */
int grab_service_macros(service* svc) {
  return (grab_service_macros_r(get_global_macros(), svc));
}

}
