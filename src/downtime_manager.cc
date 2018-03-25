/*
** Copyright 2018 Centreon
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

#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/downtime_manager.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/service.hh"

using namespace com::centreon::engine;

/**
 *  Schedule a downtime.
 *
 *  @param[in,out] target        Target object.
 *  @param[in]     entry_time    Entry time.
 *  @param[in]     author        Author.
 *  @param[in]     comment       Comment.
 *  @param[in]     start_time    Start time.
 *  @param[in]     end_time      End time. Downtime will not be started
 *                               after this time.
 *  @param[in]     fixed         True if downtime is fixed, false if
 *                               flexible.
 *  @param[in]     duration      Downtime duration in seconds.
 *  @param[in]     triggered_by  ID of the downtime triggering the new
 *                               downtime.
 *
 *  @return New downtime ID.
 */
unsigned long downtime_manager::schedule(
                                  notifications::notifier* target,
                                  time_t entry_time,
                                  std::string const& author,
                                  std::string const& comment,
                                  time_t start_time,
                                  time_t end_time,
                                  bool fixed,
                                  unsigned long duration,
                                  unsigned long triggered_by) {
  // Debug.
  logger(logging::dbg_functions, logging::basic) << "downtime scheduling";

  // Create comment text.
  char start_time_string[MAX_DATETIME_LENGTH] = "";
  char end_time_string[MAX_DATETIME_LENGTH] = "";
  get_datetime_string(
    &start_time,
    start_time_string,
    MAX_DATETIME_LENGTH,
    SHORT_DATE_TIME);
  get_datetime_string(
    &end_time,
    end_time_string,
    MAX_DATETIME_LENGTH,
    SHORT_DATE_TIME);
  int hours(0);
  int minutes(0);
  int seconds(0);
  hours = duration / 3600;
  minutes = (duration - hours * 3600) / 60;
  seconds = duration - hours * 3600 - minutes * 60;
  std::string type_string(target->is_host() ? "host" : "service");
  std::ostringstream oss;
  if (fixed)
    oss << "This " << type_string
        << " has been scheduled for fixed downtime from "
        << start_time_string << " to " << end_time_string << "."
        << " Notifications for the " << type_string
        << " will not be sent out during that time period.";
  else
    oss << "This " << type_string
        << " has been scheduled for flexible downtime starting between "
        << start_time_string << " and " << end_time_string
        << " and lasting for a period of " << hours << " hours and "
        << minutes << " minutes. Notifications for the " << type_string
        << " will not be sent out during that time period.";

  // Add a non-persistent comment to the target regarding the scheduled outage.
  com::centreon::engine::comment* new_comment;
  if (target->is_host())
    new_comment = comment::add_new_comment(
      comment::HOST_COMMENT,
      comment::DOWNTIME_COMMENT,
      target,
      time(NULL),
      "(Centreon Engine Process)",
      oss.str(),
      0,
      comment::COMMENTSOURCE_INTERNAL,
      false,
      (time_t)0);
  else
    new_comment = comment::add_new_comment(
      comment::SERVICE_COMMENT,
      comment::DOWNTIME_COMMENT,
      target,
      time(NULL),
      "(Centreon Engine Process)",
      oss.str(),
      0,
      comment::COMMENTSOURCE_INTERNAL,
      false,
      (time_t)0);

  // Create downtime object.
  downtime dt(
    _get_next_downtime_id(),
    target,
    entry_time,
    author,
    comment,
    new_comment->get_id(),
    start_time,
    end_time,
    fixed,
    duration,
    triggered_by);
  _downtimes[dt.get_id()] = dt;

  // Debug.
  logger(logging::dbg_downtime, logging::basic)
    << "Scheduled Downtime Details:";
  if (target->is_host())
    logger(logging::dbg_downtime, logging::basic)
      << " Type:        Host Downtime\n"
         " Host:        " << static_cast<host*>(target)->get_name();
  else
    logger(logging::dbg_downtime, logging::basic)
      << " Type:        Service Downtime\n"
         " Host:        " << static_cast<service*>(target)->get_host_name() << "\n"
         " Service:     " << static_cast<service*>(target)->get_description();
  logger(logging::dbg_downtime, logging::basic)
    << " Fixed/Flex:  " << (fixed ? "Fixed\n" : "Flexible\n")
    << " Start:       " << start_time_string << "\n"
       " End:         " << end_time_string << "\n"
       " Duration:    " << hours << "h " << minutes << "m " << seconds << "s\n"
       " Downtime ID: " << dt.get_id() << "\n"
       " Trigger ID:  " << triggered_by;

  // Schedule non-triggered fixed downtimes.
  unsigned long* id(NULL);
  if (fixed && (triggered_by == 0)) {
    id = new unsigned long;
    *id = dt.get_id();
    schedule_new_event(
      EVENT_SCHEDULED_DOWNTIME,
      true,
      start_time,
      false,
      0,
      NULL,
      true,
      (void*)id,
      NULL,
      0);
    id = NULL;
  }
  id = new unsigned long;
  *id = dt.get_id();
  schedule_new_event(
    EVENT_EXPIRE_DOWNTIME,
    true,
    end_time,
    false,
    0,
    NULL,
    true,
    (void*)id,
    NULL,
    0);
  id = NULL;

  return (dt.get_id());
}

/**
 *  Get the next downtime ID.
 *
 *  @return Next available downtime ID.
 */
unsigned long downtime_manager::_get_next_downtime_id() {
  // XXX : handle existing downtime ID
  return (_next_downtime_id++);
}
