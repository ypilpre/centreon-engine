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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/downtime_manager.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/service.hh"

using namespace com::centreon::engine;

// Instance.
downtime_manager* downtime_manager::_instance(NULL);

/**
 *  Load downtime manager.
 */
void downtime_manager::load() {
  _instance = new downtime_manager();
  return ;
}

/**
 *  Get instance of downtime manager.
 *
 *  @return Reference to the downtime manager.
 */
downtime_manager& downtime_manager::instance() {
  return (*_instance);
}

/**
 *  Unload downtime manager.
 */
void downtime_manager::unload() {
  delete _instance;
  _instance = NULL;
  return ;
}

/**
 *  Get currently scheduled downtimes.
 *
 *  @return Container with currently scheduled downtimes.
 */
umap<unsigned long, downtime> const& downtime_manager::get_downtimes() const {
  return (_downtimes);
}

/**
 *  Get the next downtime ID.
 *
 *  @return Next downtime ID.
 */
unsigned long downtime_manager::get_next_downtime_id() const {
  return (_next_downtime_id);
}

/**
 *  Set the next downtime ID.
 *
 *  @param[in] id  New next downtime ID.
 */
void downtime_manager::set_next_downtime_id(unsigned long id) {
  _next_downtime_id = id;
  return ;
}

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
 *  @param[in]     propagate     Downtime propagation parameters (only
 *                               for hosts).
 *  @param[in]     id            Downtime ID. 0 if it should be
 *                               generated automatically.
 *
 *  @return New downtime ID.
 */
unsigned long downtime_manager::schedule(
                                  notifications::notifiable* target,
                                  time_t entry_time,
                                  std::string const& author,
                                  std::string const& comment,
                                  time_t start_time,
                                  time_t end_time,
                                  bool fixed,
                                  unsigned long duration,
                                  unsigned long triggered_by,
                                  downtime_manager::propagation propagate,
                                  unsigned long id) {
  // Debug.
  logger(logging::dbg_functions, logging::basic)
    << "downtime_manager::schedule()";

  // Don't add old or invalid downtimes.
  if (start_time >= end_time || end_time <= time(NULL))
    return (0);

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
    id ? id : _use_next_downtime_id(),
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

  // Increment pending flex downtime.
  if (!fixed) {
    target->inc_pending_flex_downtime();
    _downtimes[dt.get_id()].set_incremented_pending_downtime();
  }

  // Schedule non-triggered fixed downtimes.
  unsigned long* id_ptr(NULL);
  if (fixed && (triggered_by == 0)) {
    id_ptr = new unsigned long;
    *id_ptr = dt.get_id();
    schedule_new_event(
      EVENT_SCHEDULED_DOWNTIME,
      true,
      start_time,
      false,
      0,
      NULL,
      true,
      id_ptr,
      NULL,
      0);
    id_ptr = NULL;
  }
  id_ptr = new unsigned long;
  *id_ptr = dt.get_id();
  schedule_new_event(
    EVENT_EXPIRE_DOWNTIME,
    true,
    end_time,
    false,
    0,
    NULL,
    true,
    id_ptr,
    NULL,
    0);
  id_ptr = NULL;

  // Propagate downtime.
  if (target->is_host()) {
    host* temp_host(static_cast<host*>(target));
    switch (propagate) {
     case DOWNTIME_PROPAGATE_NONE:
      triggered_by = 0;
      propagate = DOWNTIME_PROPAGATE_NONE;
      break ;
     case DOWNTIME_PROPAGATE_SIMPLE:
      propagate = DOWNTIME_PROPAGATE_NONE;
      break ;
     default:
      break ;
    }
    // Check all child hosts...
    for (host_set::const_iterator
           it(temp_host->get_children().begin()),
           end(temp_host->get_children().end());
         it != end;
         ++it)
      schedule(
        *it,
        entry_time,
        author,
        comment,
        start_time,
        end_time,
        fixed,
        duration,
        triggered_by,
        propagate);
  }

  return (dt.get_id());
}

/**
 *  Start a downtime.
 *
 *  @param[in] id  Downtime ID.
 */
void downtime_manager::start(unsigned long id) {
  umap<unsigned long, downtime>::iterator
    dt(_downtimes.find(id));
  if (dt != _downtimes.end()) {
    // Start downtime.
    dt->second.start();

    // Start downtimes that are triggered by this one.
    for (umap<unsigned long, downtime>::iterator
           it(_downtimes.begin()),
           end(_downtimes.end());
         it != end;
         ++it)
      if (it->second.get_triggered_by() == id)
        it->second.start();
  }
  return ;
}

/**
 *  Stop a downtime.
 *
 *  @param[in] id  Downtime ID.
 */
void downtime_manager::stop(unsigned long id) {
  umap<unsigned long, downtime>::iterator
    dt(_downtimes.find(id));
  if (dt != _downtimes.end()) {
    // Stop downtime.
    dt->second.stop();
    _downtimes.erase(dt);

    // Stop downtimes that are triggered by this one.
    for (umap<unsigned long, downtime>::iterator
           it(_downtimes.begin()),
           end(_downtimes.end());
         it != end;
         ++it)
      if (it->second.get_triggered_by() == id)
        it->second.stop();
  }

  return ;
}

/**
 *  Unschedules a downtime.
 *
 *  @param[in] id  Downtime ID.
 */
void downtime_manager::unschedule(unsigned long id) {
  // Debug.
  logger(logging::dbg_functions, logging::basic)
    << "downtime_manager::unschedule()";

  // Find target downtime.
  umap<unsigned long, downtime>::iterator
    dt_it(_downtimes.find(id));
  if (dt_it != _downtimes.end()) {
    downtime& dt(dt_it->second);
    notifications::notifiable* parent(dt.get_parent());
    union {
      host* hst;
      service* svc;
    } target;
    if (parent->is_host())
      target.hst = static_cast<host*>(parent);
    else
      target.svc = static_cast<service*>(parent);

    // Decrement pending flex downtime if necessary...
    if (!dt.get_fixed() && dt.get_incremented_pending_downtime())
      parent->dec_pending_flex_downtime();

    // Decrement the downtime depth variable
    // and update status data if necessary.
    if (dt.get_in_effect()) {
      // Send data to event broker.
      if (parent->is_host())
        broker_downtime_data(
          NEBTYPE_DOWNTIME_STOP,
          NEBFLAG_NONE,
          NEBATTR_DOWNTIME_STOP_CANCELLED,
          downtime::HOST_DOWNTIME,
          target.hst->get_name(),
          "",
          dt.get_entry_time(),
          dt.get_author(),
          dt.get_comment(),
          dt.get_start_time(),
          dt.get_end_time(),
          dt.get_fixed(),
          dt.get_triggered_by(),
          dt.get_duration(),
          dt.get_id(),
          NULL);
      else
        broker_downtime_data(
          NEBTYPE_DOWNTIME_STOP,
          NEBFLAG_NONE,
          NEBATTR_DOWNTIME_STOP_CANCELLED,
          downtime::SERVICE_DOWNTIME,
          target.svc->get_host_name(),
          target.svc->get_description(),
          dt.get_entry_time(),
          dt.get_author(),
          dt.get_comment(),
          dt.get_start_time(),
          dt.get_end_time(),
          dt.get_fixed(),
          dt.get_triggered_by(),
          dt.get_duration(),
          dt.get_id(),
          NULL);

      // Decrement downtime depth and update status.
      parent->dec_scheduled_downtime_depth();
      if (parent->is_host()) {
        host* hst(static_cast<host*>(parent));
        broker_host_status(hst);
        if (!parent->get_scheduled_downtime_depth()) {
          logger(logging::log_info_message, logging::basic)
            << "HOST DOWNTIME ALERT: " << hst->get_name()
            << ";CANCELLED; Scheduled downtime for host has been "
               "cancelled.";

          // Send a notification.
          parent->notify(
            notifications::notifiable::DOWNTIMECANCELLED,
            "",
            "",
            NOTIFICATION_OPTION_NONE);
        }
      }
      else {
        service* svc(static_cast<service*>(parent));
        broker_service_status(svc);
        if (!parent->get_scheduled_downtime_depth()) {
          logger(logging::log_info_message, logging::basic)
            << "SERVICE DOWNTIME ALERT: " << svc->get_host_name() << ";"
            << svc->get_description() << ";CANCELLED; Scheduled downtime "
               "for service has been cancelled.";

          // Send a notification.
          parent->notify(
            notifications::notifiable::DOWNTIMECANCELLED,
            "",
            "",
            NOTIFICATION_OPTION_NONE);
        }
      }
    }

    // Remove scheduled entry from event queue.
    timed_event* temp_event(NULL);
    for (temp_event = event_list_high;
         temp_event != NULL;
         temp_event = temp_event->next) {
      if (temp_event->event_type != EVENT_SCHEDULED_DOWNTIME)
        continue ;
      if (((unsigned long)temp_event->event_data) == id)
      break ;
    }
    if (temp_event != NULL)
      remove_event(temp_event, &event_list_high, &event_list_high_tail);

    // Notify event broker.
    if (parent->is_host())
      broker_downtime_data(
        NEBTYPE_DOWNTIME_DELETE,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        downtime::HOST_DOWNTIME,
        target.hst->get_name(),
        "",
        dt.get_entry_time(),
        dt.get_author(),
        dt.get_comment(),
        dt.get_start_time(),
        dt.get_end_time(),
        dt.get_fixed(),
        dt.get_triggered_by(),
        dt.get_duration(),
        dt.get_id(),
        NULL);
    else
      broker_downtime_data(
        NEBTYPE_DOWNTIME_DELETE,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        downtime::SERVICE_DOWNTIME,
        target.svc->get_host_name(),
        target.svc->get_description(),
        dt.get_entry_time(),
        dt.get_author(),
        dt.get_comment(),
        dt.get_start_time(),
        dt.get_end_time(),
        dt.get_fixed(),
        dt.get_triggered_by(),
        dt.get_duration(),
        dt.get_id(),
        NULL);

    // Remove downtime and its comment.
    comment::delete_comment(dt.get_comment_id());
    _downtimes.erase(dt_it);

    // Unschedule all downtime entries that were triggered by this one.
    bool had_downtime_triggered(true);
    while (had_downtime_triggered) {
      had_downtime_triggered = false;
      for (umap<unsigned long, downtime>::const_iterator
             it(_downtimes.begin()),
             end(_downtimes.end());
           it != end;
           ++it)
        if (it->second.get_triggered_by() == id) {
          had_downtime_triggered = true;
          unschedule(it->second.get_id());
          break ;
        }
    }
  }
  return ;
}

/**
 *  Constructor.
 */
downtime_manager::downtime_manager() : _next_downtime_id(1) {}

/**
 *  Destructor.
 */
downtime_manager::~downtime_manager() {}

/**
 *  Use the next downtime ID.
 *
 *  @return Next available downtime ID.
 */
unsigned long downtime_manager::_use_next_downtime_id() {
  while ((_downtimes.find(_next_downtime_id) != _downtimes.end())
         || !_next_downtime_id)
    ++_next_downtime_id;
  return (_next_downtime_id++);
}
