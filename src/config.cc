/*
** Copyright 1999-2008           Ethan Galstad
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

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/not_found.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/objects/commandsmember.hh"
#include "com/centreon/engine/objects/contactsmember.hh"
#include "com/centreon/engine/objects/contactgroupsmember.hh"
#include "com/centreon/engine/objects/hostsmember.hh"
#include "com/centreon/engine/objects/objectlist.hh"
#include "com/centreon/engine/objects/servicesmember.hh"
#include "com/centreon/engine/objects/timeperiodexclusion.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/****************************************************************/
/**************** CONFIG VERIFICATION FUNCTIONS *****************/
/****************************************************************/

/* dfs status values */
#define DFS_UNCHECKED                    0      /* default value */
#define DFS_TEMP_CHECKED                 1      /* check just one time */
#define DFS_OK                           2      /* no problem */
#define DFS_NEAR_LOOP                    3      /* has trouble sons */
#define DFS_LOOPY                        4      /* is a part of a loop */

#define dfs_get_status(h) (h)->get_circular_path_checked()
#define dfs_unset_status(h) (h)->set_circular_path_checked(0)
#define dfs_set_status(h, flag) (h)->set_circular_path_checked(flag)
#define dfs_host_status(h) ((h) ? dfs_get_status(h) : DFS_OK)

/**
 * Modified version of Depth-first Search
 * http://en.wikipedia.org/wiki/Depth-first_search
 */
static int dfs_host_path(host* root) {
  if (!root)
    return (DFS_NEAR_LOOP);

  if (dfs_get_status(root) != DFS_UNCHECKED)
    return (dfs_get_status(root));

  /* Mark the root temporary checked */
  dfs_set_status(root, DFS_TEMP_CHECKED);

  /* We are scanning the children */
  for (host_set::const_iterator
         it(root->get_children().begin()),
         end(root->get_children().end());
       it != end;
       ++it) {
    int child_status = dfs_get_status(*it);

    /* If a child is not checked, check it */
    if (child_status == DFS_UNCHECKED)
      child_status = dfs_host_path(*it);

    /* If a child already temporary checked, its a problem,
     * loop inside, and its a acked status */
    if (child_status == DFS_TEMP_CHECKED) {
      dfs_set_status(*it, DFS_LOOPY);
      dfs_set_status(root, DFS_LOOPY);
    }

    /* If a child already temporary checked, its a problem, loop inside */
    if (child_status == DFS_NEAR_LOOP || child_status == DFS_LOOPY) {
      /* if a node is know to be part of a loop, do not let it be less */
      if (dfs_get_status(root) != DFS_LOOPY)
        dfs_set_status(root, DFS_NEAR_LOOP);

      /* we already saw this child, it's a problem */
      dfs_set_status(*it, DFS_LOOPY);
    }
  }

  /*
   * If root have been modified, do not set it OK
   * A node is OK if and only if all of his children are OK
   * If it does not have child, goes ok
   */
  if (dfs_get_status(root) == DFS_TEMP_CHECKED)
    dfs_set_status(root, DFS_OK);
  return (dfs_get_status(root));
}

/* check for circular paths and dependencies */
int pre_flight_circular_check(int* w, int* e) {
  servicedependency* temp_sd(NULL);
  servicedependency* temp_sd2(NULL);
  hostdependency* temp_hd(NULL);
  hostdependency* temp_hd2(NULL);
  int found(false);
  int warnings(0);
  int errors(0);

  /* bail out if we aren't supposed to verify circular paths */
  if (verify_circular_paths == false)
    return (OK);

  /********************************************/
  /* check for circular paths between hosts   */
  /********************************************/

  /* check routes between all hosts */
  found = false;

  /* We clean the dsf status from previous check */
  for (umap<std::string, com::centreon::shared_ptr<::host> >::iterator
         it(configuration::applier::state::instance().hosts().begin()),
         end(configuration::applier::state::instance().hosts().end());
       it != end;
       ++it)
    dfs_set_status(it->second.get(), DFS_UNCHECKED);

  for (umap<std::string, com::centreon::shared_ptr<::host> >::iterator
         it(configuration::applier::state::instance().hosts().begin()),
         end(configuration::applier::state::instance().hosts().end());
       it != end;
       ++it)
    if (dfs_host_path(it->second.get()) == DFS_LOOPY)
      errors = 1;

  for (umap<std::string, com::centreon::shared_ptr<::host> >::iterator
         it(configuration::applier::state::instance().hosts().begin()),
         end(configuration::applier::state::instance().hosts().end());
       it != end;
       ++it) {
    if (dfs_get_status(it->second.get()) == DFS_LOOPY)
      logger(log_verification_error, basic)
        << "Error: The host '" << it->second->get_host_name()
        << "' is part of a circular parent/child chain!";
    /* clean DFS status */
    dfs_set_status(it->second.get(), DFS_UNCHECKED);
  }

  /********************************************/
  /* check for circular dependencies         */
  /********************************************/

  /* check execution dependencies between all services */
  for (temp_sd = servicedependency_list;
       temp_sd != NULL;
       temp_sd = temp_sd->next) {

    /* clear checked flag for all dependencies */
    for (temp_sd2 = servicedependency_list;
	 temp_sd2 != NULL;
         temp_sd2 = temp_sd2->next)
      temp_sd2->circular_path_checked = false;

    found = check_for_circular_servicedependency_path(
              temp_sd,
              temp_sd,
              EXECUTION_DEPENDENCY);
    if (found == true) {
      logger(log_verification_error, basic)
        << "Error: A circular execution dependency (which could result "
        "in a deadlock) exists for service '"
        << temp_sd->service_description << "' on host '"
        << temp_sd->host_name << "'!";
      ++errors;
    }
  }

  /* check notification dependencies between all services */
  for (temp_sd = servicedependency_list;
       temp_sd != NULL;
       temp_sd = temp_sd->next) {

    /* clear checked flag for all dependencies */
    for (temp_sd2 = servicedependency_list;
	 temp_sd2 != NULL;
         temp_sd2 = temp_sd2->next)
      temp_sd2->circular_path_checked = false;

    found = check_for_circular_servicedependency_path(
              temp_sd,
              temp_sd,
              NOTIFICATION_DEPENDENCY);
    if (found == true) {
      logger(log_verification_error, basic)
        << "Error: A circular notification dependency (which could "
        "result in a deadlock) exists for service '"
        << temp_sd->service_description << "' on host '"
        << temp_sd->host_name << "'!";
      ++errors;
    }
  }

  /* clear checked flag for all dependencies */
  for (temp_sd = servicedependency_list;
       temp_sd != NULL;
       temp_sd = temp_sd->next)
    temp_sd->circular_path_checked = false;

  /* check execution dependencies between all hosts */
  for (temp_hd = hostdependency_list;
       temp_hd != NULL;
       temp_hd = temp_hd->next) {

    /* clear checked flag for all dependencies */
    for (temp_hd2 = hostdependency_list;
	 temp_hd2 != NULL;
         temp_hd2 = temp_hd2->next)
      temp_hd2->circular_path_checked = false;

    found = check_for_circular_hostdependency_path(
              temp_hd,
              temp_hd,
              EXECUTION_DEPENDENCY);
    if (found == true) {
      logger(log_verification_error, basic)
        << "Error: A circular execution dependency (which could "
        "result in a deadlock) exists for host '"
        << temp_hd->host_name << "'!";
      ++errors;
    }
  }

  /* check notification dependencies between all hosts */
  for (temp_hd = hostdependency_list;
       temp_hd != NULL;
       temp_hd = temp_hd->next) {

    /* clear checked flag for all dependencies */
    for (temp_hd2 = hostdependency_list;
	 temp_hd2 != NULL;
         temp_hd2 = temp_hd2->next)
      temp_hd2->circular_path_checked = false;

    found = check_for_circular_hostdependency_path(
              temp_hd,
              temp_hd,
              NOTIFICATION_DEPENDENCY);
    if (found == true) {
      logger(log_verification_error, basic)
        << "Error: A circular notification dependency (which could "
        "result in a deadlock) exists for host '"
        << temp_hd->host_name << "'!";
      ++errors;
    }
  }

  /* clear checked flag for all dependencies */
  for (temp_hd = hostdependency_list;
       temp_hd != NULL;
       temp_hd = temp_hd->next)
    temp_hd->circular_path_checked = false;

  /* update warning and error count */
  if (w != NULL)
    *w += warnings;
  if (e != NULL)
    *e += errors;

  return ((errors > 0) ? ERROR : OK);
}

int check_contact(contact* cntct, int* w, int* e) {
  int warnings(0);
  int errors(0);

  /* check service notification commands */
  if (cntct->service_notification_commands == NULL) {
    logger(log_verification_error, basic)
      << "Error: Contact '" << cntct->name << "' has no service "
      "notification commands defined!";
    ++errors;
  }
  else
    for (commandsmember* temp_commandsmember = cntct->service_notification_commands;
	 temp_commandsmember != NULL;
	 temp_commandsmember = temp_commandsmember->next) {

      /* check the host notification command */
      char* buf = string::dup(temp_commandsmember->cmd);

      /* get the command name, leave any arguments behind */
      char* temp_command_name = my_strtok(buf, "!");

      command* temp_command(NULL);
      try {
        temp_command = &find_command(temp_command_name);
      }
      catch (not_found const& e) {
        (void)e;
        logger(log_verification_error, basic)
          << "Error: Service notification command '"
          << temp_command_name << "' specified for contact '"
          << cntct->name << "' is not defined anywhere!";
	++errors;
      }

      /* save pointer to the command for later */
      temp_commandsmember->command_ptr = temp_command;

      delete[] buf;
    }

  /* check host notification commands */
  if (cntct->host_notification_commands == NULL) {
    logger(log_verification_error, basic)
      << "Error: Contact '" << cntct->name << "' has no host "
      "notification commands defined!";
    ++errors;
  }
  else
    for (commandsmember* temp_commandsmember = cntct->host_notification_commands;
	 temp_commandsmember != NULL;
	 temp_commandsmember = temp_commandsmember->next) {

      /* check the host notification command */
      char* buf = string::dup(temp_commandsmember->cmd);

      /* get the command name, leave any arguments behind */
      char* temp_command_name = my_strtok(buf, "!");

      command* temp_command(NULL);
      try {
        temp_command = &find_command(temp_command_name);
      }
      catch (not_found const& e) {
        (void)e;
        logger(log_verification_error, basic)
          << "Error: Host notification command '" << temp_command_name
          << "' specified for contact '" << cntct->name
          << "' is not defined anywhere!";
	++errors;
      }

      /* save pointer to the command for later */
      temp_commandsmember->command_ptr = temp_command;

      delete[] buf;
    }

  /* check service notification timeperiod */
  if (cntct->service_notification_period == NULL) {
    logger(log_verification_error, basic)
      << "Warning: Contact '" << cntct->name << "' has no service "
      "notification time period defined!";
    warnings++;
  }

  else {
    timeperiod* temp_timeperiod(NULL);
    try {
      temp_timeperiod = configuration::applier::state::instance().timeperiods_find(
                          cntct->service_notification_period).get();
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Service notification period '"
        << cntct->service_notification_period
        << "' specified for contact '" << cntct->name
        << "' is not defined anywhere!";
      ++errors;
    }

    /* save the pointer to the service notification timeperiod for later */
    cntct->service_notification_period_ptr = temp_timeperiod;
  }

  /* check host notification timeperiod */
  if (cntct->host_notification_period == NULL) {
    logger(log_verification_error, basic)
      << "Warning: Contact '" << cntct->name << "' has no host "
      "notification time period defined!";
    warnings++;
  }

  else {
    timeperiod* temp_timeperiod(NULL);
    try {
      temp_timeperiod = configuration::applier::state::instance().timeperiods_find(
                          cntct->host_notification_period).get();
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Host notification period '"
        << cntct->host_notification_period
        << "' specified for contact '" << cntct->name
        << "' is not defined anywhere!";
      ++errors;
    }

    /* save the pointer to the host notification timeperiod for later */
    cntct->host_notification_period_ptr = temp_timeperiod;
  }

  /* check for sane host recovery options */
  if (cntct->notify_on_host_recovery == true
      && cntct->notify_on_host_down == false
      && cntct->notify_on_host_unreachable == false) {
    logger(log_verification_error, basic)
      << "Warning: Host recovery notification option for contact '"
      << cntct->name << "' doesn't make any sense - specify down "
      "and/or unreachable options as well";
    warnings++;
  }

  /* check for sane service recovery options */
  if (cntct->notify_on_service_recovery == true
      && cntct->notify_on_service_critical == false
      && cntct->notify_on_service_warning == false) {
    logger(log_verification_error, basic)
      << "Warning: Service recovery notification option for contact '"
      << cntct->name << "' doesn't make any sense - specify critical "
      "and/or warning options as well";
    warnings++;
  }

  /* check for illegal characters in contact name */
  if (contains_illegal_object_chars(cntct->name) == true) {
    logger(log_verification_error, basic)
      << "Error: The name of contact '" << cntct->name
      << "' contains one or more illegal characters.";
    ++errors;
  }

  if (w != NULL)
    *w += warnings;
  if (e != NULL)
    *e += errors;
  return (errors == 0);
}

/**
 *  Check and resolve service groups.
 *
 *  @param[in,out] sg Service group object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_servicegroup(servicegroup* sg, int* w, int* e) {
  (void)w;
  int errors(0);

  // Check all group members.
  for (servicesmember* temp_servicesmember(sg->members);
       temp_servicesmember;
       temp_servicesmember = temp_servicesmember->next) {
    service* temp_service(NULL);
    try {
      temp_service = configuration::applier::state::instance().services_find(
        std::make_pair(
               temp_servicesmember->host_name,
               temp_servicesmember->service_description)).get();
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Service '"
        << temp_servicesmember->service_description
        << "' on host '" << temp_servicesmember->host_name
        << "' specified in service group '" << sg->group_name
        << "' is not defined anywhere!";
      ++errors;
    }

    // Add service to group and group to service links.
    temp_service->add_servicegroup(sg);
    temp_servicesmember->service_ptr = temp_service;
  }

  // Check for illegal characters in servicegroup name.
  if (contains_illegal_object_chars(sg->group_name) == true) {
    logger(log_verification_error, basic)
      << "Error: The name of servicegroup '" << sg->group_name
      << "' contains one or more illegal characters.";
    ++errors;
  }

  // Add errors.
  if (e)
    *e += errors;

  return (errors == 0);
}

/**
 *  Check and resolve host groups.
 *
 *  @param[in,out] hg Host group object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_hostgroup(hostgroup* hg, int* w, int* e) {
  (void)w;
  int errors(0);

  // Check all group members.
  for (hostsmember* temp_hostsmember(hg->members);
       temp_hostsmember;
       temp_hostsmember = temp_hostsmember->next) {
    host* temp_host(NULL);
    try {
      temp_host = configuration::applier::state::instance().hosts_find(
                    temp_hostsmember->host_name).get();
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Host '" << temp_hostsmember->host_name
        << "' specified in host group '" << hg->group_name
        << "' is not defined anywhere!";
      ++errors;
    }

    // Add host to group and group to service links.
    temp_host->add_hostgroup(hg);
    temp_hostsmember->host_ptr = temp_host;
  }

  // Check for illegal characters in hostgroup name.
  if (contains_illegal_object_chars(hg->group_name) == true) {
    logger(log_verification_error, basic)
      << "Error: The name of hostgroup '" << hg->group_name
      << "' contains one or more illegal characters.";
    ++errors;
  }

  // Add errors.
  if (e)
    *e += errors;

  return (errors == 0);
}

/**
 *  Check and resolve a contact group.
 *
 *  @param[in,out] cg Contact group object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_contactgroup(contactgroup* cg, int* w, int* e) {
  (void)w;
  int errors(0);

  // Check all the group members.
  for (contactsmember* temp_contactsmember(cg->members);
       temp_contactsmember;
       temp_contactsmember = temp_contactsmember->next) {
    contact* temp_contact(NULL);
    try {
      temp_contact = &find_contact(temp_contactsmember->contact_name);
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Contact '" << temp_contactsmember->contact_name
        << "' specified in contact group '" << cg->group_name
        << "' is not defined anywhere!";
      ++errors;
    }

    // Save a pointer to this contact group for faster contact/group
    // membership lookups later.
    add_object_to_objectlist(&temp_contact->contactgroups_ptr, cg);

    // Save the contact pointer for later.
    temp_contactsmember->contact_ptr = temp_contact;
  }

  // Check for illegal characters in contact group name.
  if (contains_illegal_object_chars(cg->group_name) == true) {
    logger(log_verification_error, basic)
      << "Error: The name of contact group '" << cg->group_name
      << "' contains one or more illegal characters.";
    ++errors;
  }

  // Add errors.
  if (e)
    *e += errors;

  return (errors == 0);
}

/**
 *  Check and resolve a service dependency.
 *
 *  @param[in,out] sd Service dependency object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_servicedependency(servicedependency* sd, int* w, int* e) {
  (void)w;
  int errors(0);

  // Find the dependent service.
  service* temp_service(NULL);
  try {
    temp_service = configuration::applier::state::instance().services_find(
      std::make_pair(
             sd->dependent_host_name,
             sd->dependent_service_description)).get();
  }
  catch (not_found const& e) {
    (void)e;
    logger(log_verification_error, basic)
      << "Error: Dependent service '"
      << sd->dependent_service_description << "' on host '"
      << sd->dependent_host_name
      << "' specified in service dependency for service '"
      << sd->service_description << "' on host '"
      << sd->host_name << "' is not defined anywhere!";
    ++errors;
  }

  // Save pointer for later.
  sd->dependent_service_ptr = temp_service;

  // Find the service we're depending on.
  try {
    temp_service = configuration::applier::state::instance().services_find(
      std::make_pair(
             sd->host_name,
             sd->service_description)).get();
  }
  catch (not_found const& e) {
    (void)e;
    logger(log_verification_error, basic)
      << "Error: Service '" << sd->service_description << "' on host '"
      << sd->host_name
      << "' specified in service dependency for service '"
      << sd->dependent_service_description << "' on host '"
      << sd->dependent_host_name << "' is not defined anywhere!";
    ++errors;
  }

  // Save pointer for later.
  sd->master_service_ptr = temp_service;

  // Make sure they're not the same service.
  if (sd->dependent_service_ptr == sd->master_service_ptr) {
    logger(log_verification_error, basic)
      << "Error: Service dependency definition for service '"
      << sd->dependent_service_description << "' on host '"
      << sd->dependent_host_name
      << "' is circular (it depends on itself)!";
    ++errors;
  }

  // Find the timeperiod.
  if (sd->dependency_period) {
    timeperiod* temp_timeperiod(NULL);
    try {
      temp_timeperiod = configuration::applier::state::instance().timeperiods_find(
                          sd->dependency_period).get();
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Dependency period '" << sd->dependency_period
        << "' specified in service dependency for service '"
        << sd->dependent_service_description << "' on host '"
        << sd->dependent_host_name << "' is not defined anywhere!";
      ++errors;
    }

    // Save the timeperiod pointer for later.
    sd->dependency_period_ptr = temp_timeperiod;
  }

  // Add errors.
  if (e)
    *e += errors;

  return (errors == 0);
}

/**
 *  Check and resolve a host dependency.
 *
 *  @param[in,out] hd Host dependency object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_hostdependency(hostdependency* hd, int* w, int* e) {
  (void)w;
  int errors(0);

  // Find the dependent host.
  host* temp_host(NULL);
  try {
    temp_host = configuration::applier::state::instance().hosts_find(
                  hd->dependent_host_name).get();
  }
  catch (not_found const& e) {
    (void)e;
    logger(log_verification_error, basic)
      << "Error: Dependent host specified in host dependency for "
         "host '" << hd->dependent_host_name
      << "' is not defined anywhere!";
    ++errors;
  }

  // Save pointer for later.
  hd->dependent_host_ptr = temp_host;

  // Find the host we're depending on.
  try {
    temp_host = configuration::applier::state::instance().hosts_find(
                  hd->host_name).get();
  }
  catch (not_found const& e) {
    (void)e;
    logger(log_verification_error, basic)
      << "Error: Host specified in host dependency for host '"
      << hd->dependent_host_name << "' is not defined anywhere!";
    ++errors;
  }

  // Save pointer for later.
  hd->master_host_ptr = temp_host;

  // Make sure they're not the same host.
  if (hd->dependent_host_ptr == hd->master_host_ptr) {
    logger(log_verification_error, basic)
      << "Error: Host dependency definition for host '"
      << hd->dependent_host_name
      << "' is circular (it depends on itself)!";
    ++errors;
  }

  // Find the timeperiod.
  if (hd->dependency_period) {
    timeperiod* temp_timeperiod(NULL);
    try {
      temp_timeperiod = configuration::applier::state::instance().timeperiods_find(
                          hd->dependency_period).get();
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Dependency period '" << hd->dependency_period
        << "' specified in host dependency for host '"
        << hd->dependent_host_name
        << "' is not defined anywhere!";
      ++errors;
    }

    // Save the timeperiod pointer for later.
    hd->dependency_period_ptr = temp_timeperiod;
  }

  // Add errors.
  if (e)
    *e += errors;

  return (errors == 0);
}

/**
 *  Check and resolve a service escalation.
 *
 *  @param[in,out] se Service escalation object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_serviceescalation(serviceescalation* se, int* w, int* e) {
  (void)w;
  int errors(0);

  // Find the service.
  service* temp_service(NULL);
  try {
    temp_service = configuration::applier::state::instance().services_find(
                     std::make_pair(
                            se->host_name,
                            se->description)).get();
  }
  catch (not_found const& e) {
    (void)e;
    logger(log_verification_error, basic) << "Error: Service '"
        << se->description << "' on host '" << se->host_name
        << "' specified in service escalation is not defined anywhere!";
    ++errors;
  }

  // Save the service pointer for later.
  se->service_ptr = temp_service;

  // Find the timeperiod.
  if (se->escalation_period) {
    timeperiod* temp_timeperiod(NULL);
    try {
      temp_timeperiod = configuration::applier::state::instance().timeperiods_find(
                          se->escalation_period).get();
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Escalation period '" << se->escalation_period
        << "' specified in service escalation for service '"
        << se->description << "' on host '"
        << se->host_name << "' is not defined anywhere!";
      ++errors;
    }

    // Save the timeperiod pointer for later.
    se->escalation_period_ptr = temp_timeperiod;
  }

  // Check all contacts.
  for (contactsmember* temp_contactsmember(se->contacts);
       temp_contactsmember;
       temp_contactsmember = temp_contactsmember->next) {
    // Find the contact.
    contact* temp_contact(NULL);
    try {
      temp_contact = &find_contact(
                        temp_contactsmember->contact_name);
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Contact '" << temp_contactsmember->contact_name
        << "' specified in service escalation for service '"
        << se->description << "' on host '"
        << se->host_name << "' is not defined anywhere!";
      ++errors;
    }

    // Save the contact pointer for later.
    temp_contactsmember->contact_ptr = temp_contact;
  }

  // Check all contact groups.
  for (contactgroupsmember*
         temp_contactgroupsmember(se->contact_groups);
       temp_contactgroupsmember;
       temp_contactgroupsmember = temp_contactgroupsmember->next) {
    // Find the contact group.
    contactgroup* temp_contactgroup(NULL);
    try {
      temp_contactgroup =  &find_contactgroup(
        temp_contactgroupsmember->group_name);
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Contact group '"
        << temp_contactgroupsmember->group_name
        << "' specified in service escalation for service '"
        << se->description << "' on host '" << se->host_name
        << "' is not defined anywhere!";
      ++errors;
    }

    // Save the contact group pointer for later.
    temp_contactgroupsmember->group_ptr = temp_contactgroup;
  }

  // Add errors.
  if (e)
    *e += errors;

  return (errors == 0);
}

/**
 *  Check and resolve a host escalation.
 *
 *  @param[in,out] he Host escalation object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_hostescalation(hostescalation* he, int* w, int* e) {
  (void)w;
  int errors(0);

  // Find the host.
  host* temp_host(NULL);
  try {
    temp_host = configuration::applier::state::instance().hosts_find(
                  he->host_name).get();
  }
  catch (not_found const& e) {
    (void)e;
    logger(log_verification_error, basic)
      << "Error: Host '" << he->host_name
      << "' specified in host escalation is not defined anywhere!";
    ++errors;
  }

  // Save the host pointer for later.
  he->host_ptr = temp_host;

  // Find the timeperiod.
  if (he->escalation_period) {
    timeperiod* temp_timeperiod(NULL);
    try {
      temp_timeperiod = configuration::applier::state::instance().timeperiods_find(
                          he->escalation_period).get();
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Escalation period '" << he->escalation_period
        << "' specified in host escalation for host '"
        << he->host_name << "' is not defined anywhere!";
      ++errors;
    }

    // Save the timeperiod pointer for later.
    he->escalation_period_ptr = temp_timeperiod;
  }

  // Check all contacts.
  for (contactsmember* temp_contactsmember(he->contacts);
       temp_contactsmember;
       temp_contactsmember = temp_contactsmember->next) {
    // Find the contact.
    contact* temp_contact(NULL);
    try {
      temp_contact = &find_contact(
                        temp_contactsmember->contact_name);
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Contact '" << temp_contactsmember->contact_name
        << "' specified in host escalation for host '"
        << he->host_name << "' is not defined anywhere!";
      ++errors;
    }

    // Save the contact pointer for later.
    temp_contactsmember->contact_ptr = temp_contact;
  }

  // Check all contact groups.
  for (contactgroupsmember*
         temp_contactgroupsmember(he->contact_groups);
       temp_contactgroupsmember;
       temp_contactgroupsmember = temp_contactgroupsmember->next) {
    // Find the contact group.
    contactgroup* temp_contactgroup(NULL);
    try {
      temp_contactgroup = &find_contactgroup(
        temp_contactgroupsmember->group_name);
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Contact group '"
        << temp_contactgroupsmember->group_name
        << "' specified in host escalation for host '"
        << he->host_name << "' is not defined anywhere!";
      ++errors;
    }

    // Save the contact group pointer for later.
    temp_contactgroupsmember->group_ptr = temp_contactgroup;
  }

  // Add errors.
  if (e)
    *e += errors;
  return (errors == 0);
}

/**
 *  Check and resolve a time period.
 *
 *  @param[in,out] tp Time period object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_timeperiod(timeperiod* tp, int* w, int* e) {
  (void)w;
  int errors(0);

  // Check for illegal characters in timeperiod name.
  if (contains_illegal_object_chars(tp->name) == true) {
    logger(log_verification_error, basic)
      << "Error: The name of time period '" << tp->name
      << "' contains one or more illegal characters.";
    ++errors;
  }

  // Check for valid timeperiod names in exclusion list.
  for (timeperiodexclusion*
         temp_timeperiodexclusion(tp->exclusions);
       temp_timeperiodexclusion;
       temp_timeperiodexclusion = temp_timeperiodexclusion->next) {
    timeperiod* temp_timeperiod2(NULL);
    try {
      temp_timeperiod2 = configuration::applier::state::instance().timeperiods_find(
                           temp_timeperiodexclusion->timeperiod_name).get();
    }
    catch (not_found const& e) {
      (void)e;
      logger(log_verification_error, basic)
        << "Error: Excluded time period '"
        << temp_timeperiodexclusion->timeperiod_name
        << "' specified in timeperiod '" << tp->name
        << "' is not defined anywhere!";
      ++errors;
    }

    // Save the timeperiod pointer for later.
    temp_timeperiodexclusion->timeperiod_ptr = temp_timeperiod2;
  }

  // Add errors.
  if (e)
    *e += errors;

  return (errors == 0);
}
