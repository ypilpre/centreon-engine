/*
** Copyright 1999-2010           Ethan Galstad
** Copyright 2011-2013,2016-2018 Centreon
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
#include "com/centreon/engine/macros/clear_host.hh"
#include "com/centreon/engine/macros/clear_hostgroup.hh"
#include "com/centreon/engine/macros/defines.hh"
#include "com/centreon/engine/macros/grab.hh"
#include "com/centreon/engine/macros/grab_host.hh"
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
 *  Generate total services macros.
 *
 *  @param[in]  hst    Host object.
 *  @param[out] mac    Macro array.
 */
static void generate_host_total_services(
              host& hst,
              nagios_macros* mac) {
  // Generate host service summary macros
  // (if they haven't already been computed).
  if (!mac->x[MACRO_TOTALHOSTSERVICES]) {
    unsigned long total_host_services(0);
    unsigned long total_host_services_ok(0);
    unsigned long total_host_services_warning(0);
    unsigned long total_host_services_unknown(0);
    unsigned long total_host_services_critical(0);
    for (service_set::const_iterator
           it(hst.get_services().begin()),
           end(hst.get_services().end());
         it != end;
         ++it) {
      service* temp_service(*it);
      if (temp_service) {
        ++total_host_services;
        switch (temp_service->get_current_state()) {
         case STATE_OK:
          ++total_host_services_ok;
          break ;
         case STATE_WARNING:
          ++total_host_services_warning;
          break ;
         case STATE_UNKNOWN:
          ++total_host_services_unknown;
          break ;
         case STATE_CRITICAL:
          ++total_host_services_critical;
          break ;
        }
      }
    }

    // These macros are time-intensive to compute, and will likely be
    // used together, so save them all for future use.
    string::setstr(mac->x[MACRO_TOTALHOSTSERVICES], total_host_services);
    string::setstr(mac->x[MACRO_TOTALHOSTSERVICESOK], total_host_services_ok);
    string::setstr(mac->x[MACRO_TOTALHOSTSERVICESWARNING], total_host_services_warning);
    string::setstr(mac->x[MACRO_TOTALHOSTSERVICESUNKNOWN], total_host_services_unknown);
    string::setstr(mac->x[MACRO_TOTALHOSTSERVICESCRITICAL], total_host_services_critical);
  }
  return;
}

/**
 *  Extract host check type.
 *
 *  @param[in] hst Host object.
 *  @param[in] mac Unused.
 *
 *  @return Newly allocated string containing either "PASSIVE" or
 *          "ACTIVE".
 */
static char const* get_host_check_type(host& hst, nagios_macros* mac) {
  (void)mac;
  return (string::dup(
            (HOST_CHECK_PASSIVE == hst.get_check_type()
            ? "PASSIVE"
            : "ACTIVE")));
}

/**
 *  Extract host group names.
 *
 *  @param[in] hst Host object.
 *  @param[in] mac Unused.
 *
 *  @return Newly allocated string with group names.
 */
static char* get_host_group_names(host& hst, nagios_macros* mac) {
  (void)mac;

  std::string buf;
  // Find all hostgroups this host is associated with.
  for (hostgroup_set::const_iterator
         it(hst.get_groups().begin()),
         end(hst.get_groups().end());
       it != end;
       ++it) {
    hostgroup* temp_hostgroup(*it);
    if (!buf.empty())
      buf.append(",");
    buf.append(temp_hostgroup->group_name);
  }
  return (string::dup(buf));
}

/**
 *  Extract host state.
 *
 *  @param[in] stateid  State to convert.
 *
 *  @return Newly allocated string with host state in plain text.
 */
static char* get_host_state(int stateid) {
  char const* state;
  switch (stateid) {
   case HOST_DOWN:
    state = "DOWN";
    break ;
   case HOST_UNREACHABLE:
    state = "UNREACHABLE";
    break ;
   default:
    state = "UP";
  }
  return (string::dup(state));
}
static char const* get_current_host_state(host& hst, nagios_macros* mac) {
  (void)mac;
  return (get_host_state(hst.get_current_state()));
}
static char const* get_last_host_state(host& hst, nagios_macros* mac) {
  (void)mac;
  return (get_host_state(hst.get_last_state()));
}

/**
 *  Extract services statistics.
 *
 *  @param[in] hst Host object.
 *  @param[in] mac Macro array.
 *
 *  @return Newly allocated string with requested value in plain text.
 */
template <unsigned int macro_id>
static char const* get_host_total_services(host& hst, nagios_macros* mac) {
  generate_host_total_services(hst, mac);
  return (mac->x[macro_id]);
}

/**
 *  Get the parents of a host.
 *
 *  @param[in] hst Host object.
 *  @param[in] mac Macro array.
 *
 *  @return Newly allocated string with requested value in plain text.
 */
static char const* get_host_parents(host& hst, nagios_macros* mac) {
  (void)mac;
  std::string retval;
  for (host_set::const_iterator
         it(hst.get_parents().begin()),
         end(hst.get_parents().end());
       it != end;
       ++it) {
    if (!retval.empty())
      retval.append(",");
    retval.append((*it)->get_name());
  }
  return (string::dup(retval.c_str()));
}

/**
 *  Get the children of a host.
 *
 *  @param[in] hst Host object.
 *  @param[in] mac Macro array.
 *
 *  @return Newly allocated string with requested value in plain text.
 */
static char const* get_host_children(host& hst, nagios_macros* mac) {
  (void)mac;
  std::string retval;
  for (host_set::const_iterator
         it(hst.get_children().begin()),
         end(hst.get_children().end());
       it != end;
       ++it) {
    if (!retval.empty())
      retval.append(",");
    retval.append((*it)->get_name());
  }
  return (string::dup(retval.c_str()));
}

/**************************************
*                                     *
*         Redirection Object          *
*                                     *
**************************************/

// Redirection object.
struct grab_host_redirection {
  typedef umap<unsigned int, std::pair<com::centreon::shared_ptr<grabber<host> >, bool> > entry;
  entry routines;
  grab_host_redirection() {
    // Name.
    routines[MACRO_HOSTNAME].first =
      new member_grabber<host, std::string const&>(&host::get_name);
    routines[MACRO_HOSTNAME].second = true;
    // Display name.
    routines[MACRO_HOSTDISPLAYNAME].first =
      new member_grabber<host, std::string const&>(&host::get_display_name);
    routines[MACRO_HOSTDISPLAYNAME].second = true;
    // Alias.
    routines[MACRO_HOSTALIAS].first =
      new member_grabber<host, std::string const&>(&host::get_alias);
    routines[MACRO_HOSTALIAS].second = true;
    // Address.
    routines[MACRO_HOSTADDRESS].first =
      new member_grabber<host, std::string const&>(&host::get_address);
    routines[MACRO_HOSTADDRESS].second = true;
    // State.
    routines[MACRO_HOSTSTATE].first =
      new function_grabber<host>(&get_current_host_state);
    routines[MACRO_HOSTSTATE].second = true;
    // State ID.
    routines[MACRO_HOSTSTATEID].first =
      new member_grabber<host, int>(&host::get_current_state);
    routines[MACRO_HOSTSTATEID].second = true;
    // Last state.
    routines[MACRO_LASTHOSTSTATE].first =
      new function_grabber<host>(&get_last_host_state);
    routines[MACRO_LASTHOSTSTATE].second = true;
    // Last state ID.
    routines[MACRO_LASTHOSTSTATEID].first =
      new member_grabber<host, int>(&host::get_last_state);
    routines[MACRO_LASTHOSTSTATEID].second = true;
    // Check type.
    routines[MACRO_HOSTCHECKTYPE].first =
      new function_grabber<host>(&get_host_check_type);
    routines[MACRO_HOSTCHECKTYPE].second = true;
    // State type.
    routines[MACRO_HOSTSTATETYPE].first =
      new function_grabber<host>(&get_state_type<host>);
    routines[MACRO_HOSTSTATETYPE].second = true;
    // Output.
    routines[MACRO_HOSTOUTPUT].first =
      new member_grabber<host, std::string const&>(&host::get_output);
    routines[MACRO_HOSTOUTPUT].second = true;
    // Long output.
    routines[MACRO_LONGHOSTOUTPUT].first =
      new member_grabber<host, std::string const&>(&host::get_long_output);
    routines[MACRO_LONGHOSTOUTPUT].second = true;
    // Perfdata.
    routines[MACRO_HOSTPERFDATA].first =
      new member_grabber<host, std::string const&>(&host::get_perfdata);
    routines[MACRO_HOSTPERFDATA].second = true;
    // Check command.
    routines[MACRO_HOSTCHECKCOMMAND].first =
      new member_grabber<host, std::string const&>(&host::get_check_command_args);
    routines[MACRO_HOSTCHECKCOMMAND].second = true;
    // Attempt.
    routines[MACRO_HOSTATTEMPT].first =
      new member_grabber<host, int>(&host::get_current_attempt);
    routines[MACRO_HOSTATTEMPT].second = true;
    // Max attempt.
    routines[MACRO_MAXHOSTATTEMPTS].first =
      new member_grabber<host, int>(&host::get_max_attempts);
    routines[MACRO_MAXHOSTATTEMPTS].second = true;
    // Downtime.
    routines[MACRO_HOSTDOWNTIME].first =
      new member_grabber<host, int>(&host::get_scheduled_downtime_depth);
    routines[MACRO_HOSTDOWNTIME].second = true;
    // Percent state change.
    routines[MACRO_HOSTPERCENTCHANGE].first =
      new member_grabber<host, double>(&host::get_percent_state_change);
    routines[MACRO_HOSTPERCENTCHANGE].second = true;
    // Duration.
    routines[MACRO_HOSTDURATION].first =
      new function_grabber<host>(&get_duration<host>);
    routines[MACRO_HOSTDURATION].second = true;
    // Duration in seconds.
    routines[MACRO_HOSTDURATIONSEC].first =
      new function_grabber<host>(&get_duration_sec<host>);
    routines[MACRO_HOSTDURATIONSEC].second = true;
    // Execution time.
    routines[MACRO_HOSTEXECUTIONTIME].first =
      new member_grabber<host, double>(&host::get_execution_time);
    routines[MACRO_HOSTEXECUTIONTIME].second = true;
    // Latency.
    routines[MACRO_HOSTLATENCY].first =
      new member_grabber<host, double>(&host::get_latency);
    routines[MACRO_HOSTLATENCY].second = true;
    // Last check.
    routines[MACRO_LASTHOSTCHECK].first =
      new member_grabber<host, time_t>(&host::get_last_check);
    routines[MACRO_LASTHOSTCHECK].second = true;
    // Last state change.
    routines[MACRO_LASTHOSTSTATECHANGE].first =
      new member_grabber<host, time_t>(&host::get_last_state_change);
    routines[MACRO_LASTHOSTSTATECHANGE].second = true;
    // Last up.
    routines[MACRO_LASTHOSTUP].first =
      new member_grabber<host, time_t>(&host::get_last_time_up);
    routines[MACRO_LASTHOSTUP].second = true;
    // Last down.
    routines[MACRO_LASTHOSTDOWN].first =
      new member_grabber<host, time_t>(&host::get_last_time_down);
    routines[MACRO_LASTHOSTDOWN].second = true;
    // Last unreachable.
    routines[MACRO_LASTHOSTUNREACHABLE].first =
      new member_grabber<host, time_t>(&host::get_last_time_unreachable);
    routines[MACRO_LASTHOSTUNREACHABLE].second = true;
    // Notification number.
    routines[MACRO_HOSTNOTIFICATIONNUMBER].first =
      new member_grabber<host, int>(&host::get_current_notification_number);
    routines[MACRO_HOSTNOTIFICATIONNUMBER].second = true;
    // Notification ID.
    routines[MACRO_HOSTNOTIFICATIONID].first =
      new member_grabber<host, int>(&host::get_current_notification_id);
    routines[MACRO_HOSTNOTIFICATIONID].second = true;
    // Event ID.
    routines[MACRO_HOSTEVENTID].first =
      new member_grabber<host, int>(&host::get_current_event_id);
    routines[MACRO_HOSTEVENTID].second = true;
    // Last event ID.
    routines[MACRO_LASTHOSTEVENTID].first =
      new member_grabber<host, int>(&host::get_last_event_id);
    routines[MACRO_LASTHOSTEVENTID].second = true;
    // Problem ID.
    routines[MACRO_HOSTPROBLEMID].first =
      new member_grabber<host, int>(&host::get_current_problem_id);
    routines[MACRO_HOSTPROBLEMID].second = true;
    // Last problem ID.
    routines[MACRO_LASTHOSTPROBLEMID].first =
      new member_grabber<host, int>(&host::get_last_problem_id);
    routines[MACRO_LASTHOSTPROBLEMID].second = true;
    // Action URL.
    routines[MACRO_HOSTACTIONURL].first =
      new function_grabber<host>(&get_action_url<host>);
    routines[MACRO_HOSTACTIONURL].second = true;
    // Notes URL.
    routines[MACRO_HOSTNOTESURL].first =
      new function_grabber<host>(&get_notes_url<host>);
    routines[MACRO_HOSTNOTESURL].second = true;
    // Notes.
    routines[MACRO_HOSTNOTES].first =
      new function_grabber<host>(&get_notes<host>);
    routines[MACRO_HOSTNOTES].second = true;
    // Group names.
    routines[MACRO_HOSTGROUPNAMES].first =
      new function_grabber<host>(
        (char const* (*)(host&, nagios_macros*))&get_host_group_names);
    routines[MACRO_HOSTGROUPNAMES].second = true;
    // Total services.
    routines[MACRO_TOTALHOSTSERVICES].first =
      new function_grabber<host>(
        &get_host_total_services<MACRO_TOTALHOSTSERVICES>);
    routines[MACRO_TOTALHOSTSERVICES].second = false;
    // Total services ok.
    routines[MACRO_TOTALHOSTSERVICESOK].first =
      new function_grabber<host>(
        &get_host_total_services<MACRO_TOTALHOSTSERVICESOK>);
    routines[MACRO_TOTALHOSTSERVICESOK].second = false;
    // Total services warning.
    routines[MACRO_TOTALHOSTSERVICESWARNING].first =
      new function_grabber<host>(
        &get_host_total_services<MACRO_TOTALHOSTSERVICESWARNING>);
    routines[MACRO_TOTALHOSTSERVICESWARNING].second = false;
    // Total services unknown.
    routines[MACRO_TOTALHOSTSERVICESUNKNOWN].first =
      new function_grabber<host>(
        &get_host_total_services<MACRO_TOTALHOSTSERVICESUNKNOWN>);
    routines[MACRO_TOTALHOSTSERVICESUNKNOWN].second = false;
    // Total services critical.
    routines[MACRO_TOTALHOSTSERVICESCRITICAL].first =
      new function_grabber<host>(
        &get_host_total_services<MACRO_TOTALHOSTSERVICESCRITICAL>);
    routines[MACRO_TOTALHOSTSERVICESCRITICAL].second = false;
    // Acknowledgement author.
    routines[MACRO_HOSTACKAUTHOR].first =
      new function_grabber<host>(&get_macro_copy<host, MACRO_HOSTACKAUTHOR>);
    routines[MACRO_HOSTACKAUTHOR].second = true;
    // Acknowledgement author name.
    routines[MACRO_HOSTACKAUTHORNAME].first =
      new function_grabber<host>(&get_macro_copy<host, MACRO_HOSTACKAUTHORNAME>);
    routines[MACRO_HOSTACKAUTHORNAME].second = true;
    // Acknowledgement author alias.
    routines[MACRO_HOSTACKAUTHORALIAS].first =
      new function_grabber<host>(&get_macro_copy<host, MACRO_HOSTACKAUTHORALIAS>);
    routines[MACRO_HOSTACKAUTHORALIAS].second = true;
    // Acknowledgement comment.
    routines[MACRO_HOSTACKCOMMENT].first =
      new function_grabber<host>(&get_macro_copy<host, MACRO_HOSTACKCOMMENT>);
    routines[MACRO_HOSTACKCOMMENT].second = true;
    // Host parents.
    routines[MACRO_HOSTPARENTS].first =
      new function_grabber<host>(&get_host_parents);
    routines[MACRO_HOSTPARENTS].second = true;
    // Host children.
    routines[MACRO_HOSTCHILDREN].first =
      new function_grabber<host>(&get_host_children);
    routines[MACRO_HOSTCHILDREN].second = true;
    // Host ID.
    routines[MACRO_HOSTID].first =
      new member_grabber<host, unsigned int>(&host::get_id);
    routines[MACRO_HOSTID].second = true;
    // Host timezone.
    routines[MACRO_HOSTTIMEZONE].first =
      new member_grabber<host, std::string const&>(&host::get_timezone);
    routines[MACRO_HOSTTIMEZONE].second = true;
  }
} static const redirector;

/**************************************
*                                     *
*           Global Functions          *
*                                     *
**************************************/

extern "C" {
/**
 *  Grab a standard host macro.
 *
 *  @param[out] mac        Macro array.
 *  @param[in]  macro_type Macro to dump.
 *  @param[in]  hst        Target host.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer should be free
 *                         by caller.
 *
 *  @return OK on success.
 */
int grab_standard_host_macro_r(
      nagios_macros* mac,
      int macro_type,
      host* hst,
      char** output,
      int* free_macro) {
  // Check that function was called with valid arguments.
  int retval;
  if (hst && output && free_macro) {
    grab_host_redirection::entry::const_iterator it(
      redirector.routines.find(macro_type));
    // Found matching routine.
    if (it != redirector.routines.end()) {
      // Call routine.
      *output = const_cast<char*>((*it->second.first)(*hst, mac));

      // Set the free macro flag.
      *free_macro = it->second.second;

      // Successful execution.
      retval = OK;
    }
    // Non-existent macro.
    else {
      logger(dbg_macros, basic)
        << "UNHANDLED HOST MACRO #" << macro_type << "! THIS IS A BUG!";
      retval = ERROR;
    }
  }
  else
    retval = ERROR;

  return (retval);
}

/**
 *  Grab a standard host macro for global macros.
 *
 *  @param[in]  macro_type Macro to dump.
 *  @param[in]  hst        Target host.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer should be free
 *                         by caller.
 *
 *  @return OK on success.
 *
 *  @see grab_standard_host_macro_r
 */
int grab_standard_host_macro(
      int macro_type,
      host* hst,
      char** output,
      int* free_macro) {
  return (grab_standard_host_macro_r(
            get_global_macros(),
            macro_type,
            hst,
            output,
            free_macro));
}

/**
 *  Grab macros that are specific to a host.
 *
 *  @param[in] mac Macros object.
 *  @param[in] hst Host pointer.
 *
 *  @return OK on success.
 */
int grab_host_macros_r(nagios_macros* mac, host* hst) {
  // Clear host-related macros.
  clear_host_macros_r(mac);
  clear_hostgroup_macros_r(mac);

  // Save pointer to host.
  mac->host_ptr = hst;
  mac->hostgroup_ptr = NULL;

  if (hst == NULL)
    return (ERROR);

  // Save pointer to host's first/primary hostgroup.
  if (!hst->get_groups().empty())
    mac->hostgroup_ptr = *hst->get_groups().begin();

  return (OK);
}

/**
 *  Grab macros that are specific to a host.
 *
 *  @param[in] hst Host pointer.
 *
 *  @return OK on success.
 *
 *  @see grab_host_macros_r
 */
int grab_host_macros(host* hst) {
  return (grab_host_macros_r(get_global_macros(), hst));
}

}
