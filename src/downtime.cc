/*
** Copyright 2017 Centreon
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
#include "com/centreon/engine/downtime.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/service.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::notifications;

/**
 *  Constructor.
 *
 *  @param[in] cfg  Base configuration.
 */
downtime::downtime(
            downtime_type type,
            notifications::notifier* parent,
            time_t entry_time,
            std::string const& author,
            std::string const& comment_data,
            time_t start_time,
            time_t end_time,
            int fixed,
            unsigned long triggered_by,
            unsigned long duration)
  : _type(type),
    _parent(parent),
    _entry_time(entry_time),
    _author(author),
    _comment_data(comment_data),
    _start_time(start_time),
    _end_time(end_time),
    _fixed(fixed),
    _triggered_by(triggered_by),
    _duration(duration) {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
downtime::downtime(downtime const& other)
  : _parent(other._parent) {
  _internal_copy(other);
}

/**
 *  Destructor.
 */
downtime::~downtime() {
  std::string svc_descr;

  /* remove the downtime from the list in memory */
  /* first remove the comment associated with this downtime */
  if (get_type() == HOST_DOWNTIME) {
    delete_host_comment(get_comment_id());
    svc_descr = "";
  }
  else {
    service* svc(static_cast<service*>(get_parent()));
    delete_service_comment(get_comment_id());
    svc_descr = svc->get_description();
  }

  /* send data to event broker */
  broker_downtime_data(
    NEBTYPE_DOWNTIME_DELETE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    get_type(),
    get_host_name(),
    svc_descr,
    get_entry_time(),
    get_author(),
    get_comment(),
    get_start_time(),
    get_end_time(),
    get_fixed(),
    get_triggered_by(),
    get_duration(),
    get_downtime_id(),
    NULL);
}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
downtime& downtime::operator=(downtime const& other) {
  if (this != &other)
    _internal_copy(other);
  return (*this);
}

/**
 *  Copy internal data members.
 *
 *  @param[in] other  Object to copy.
 */
void downtime::_internal_copy(downtime const& other) {
  _entry_time = other._entry_time;
  _author = other._author;
  _comment_data = other._comment_data;
  _start_time = other._start_time;
  _end_time = other._end_time;
  _fixed = other._fixed;
  _triggered_by = other._triggered_by;
  _duration = other._duration;
  _downtime_id = other._downtime_id;
}

std::string const& downtime::get_host_name() const {
  return static_cast<monitorable*>(_parent)->get_host_name();
}

downtime::downtime_type downtime::get_type() const {
  return _type;
}

unsigned long downtime::get_id() const {
  return _downtime_id;
}

void downtime::set_id(unsigned long id) {
  _downtime_id = id;
}

time_t downtime::get_entry_time() const {
  return _entry_time;
}

time_t downtime::get_start_time() const {
  return _start_time;
}

time_t downtime::get_end_time() const {
  return _end_time;
}

int downtime::get_fixed() const {
  return _fixed;
}

unsigned long downtime::get_triggered_by() const {
  return _triggered_by;
}

unsigned long downtime::get_duration() const {
  return _duration;
}

std::string const& downtime::get_author() const {
  return _author;
}

std::string const& downtime::get_comment() const {
  return _comment_data;
}

unsigned long downtime::get_downtime_id() const {
  return _downtime_id;
}

int downtime::registration() {
  char start_time_string[MAX_DATETIME_LENGTH] = "";
  char end_time_string[MAX_DATETIME_LENGTH] = "";
  host* hst;
  service* svc;
  int hours(0);
  int minutes(0);
  int seconds(0);

  logger(dbg_functions, basic)
    << "downtime registration";

  /* find the host or service associated with this downtime */
  if (_type == HOST_DOWNTIME)
    hst = static_cast<host*>(_parent);
  else
    svc = static_cast<service*>(_parent);

  /* create the comment */
  get_datetime_string(
    &_start_time,
    start_time_string,
    MAX_DATETIME_LENGTH,
    SHORT_DATE_TIME);
  get_datetime_string(
    &_end_time,
    end_time_string,
    MAX_DATETIME_LENGTH,
    SHORT_DATE_TIME);
  hours = seconds / 3600;
  minutes = (_duration - hours * 3600) / 60;
  seconds = _duration - hours * 3600 - minutes * 60;

  std::string type_string(_type == HOST_DOWNTIME ? "host" : "service");
  std::ostringstream oss;
  if (_fixed)
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

  logger(dbg_downtime, basic) << "Scheduled Downtime Details:";
  if (_type == HOST_DOWNTIME) {
    logger(dbg_downtime, basic)
      << " Type:        Host Downtime\n"
         " Host:        " << hst->get_host_name();
  }
  else {
    logger(dbg_downtime, basic)
      << " Type:        Service Downtime\n"
         " Host:        " << svc->get_host_name() << "\n"
         " Service:     " << svc->get_description();
  }
  logger(dbg_downtime, basic)
    << " Fixed/Flex:  " << (_fixed == true ? "Fixed\n" : "Flexible\n")
    << " Start:       " << start_time_string << "\n"
       " End:         " << end_time_string << "\n"
       " Duration:    " << hours << "h " << minutes << "m " << seconds << "s\n"
       " Downtime ID: " << _downtime_id << "\n"
       " Trigger ID:  " << _triggered_by;

  /* add a non-persistent comment to the host or service regarding the scheduled outage */
  if (_type == SERVICE_DOWNTIME)
    add_new_comment(
      SERVICE_COMMENT,
      DOWNTIME_COMMENT,
      svc->get_host_name(),
      svc->get_description(),
      time(NULL),
      "(Centreon Engine Process)",
      oss.str(),
      0,
      COMMENTSOURCE_INTERNAL,
      false,
      (time_t)0,
      &_comment_id);   // FIXME DBR: should be computed automatically
  else
    add_new_comment(
      HOST_COMMENT,
      DOWNTIME_COMMENT,
      hst->get_host_name(),
      "",
      time(NULL),
      "(Centreon Engine Process)",
      oss.str(),
      0,
      COMMENTSOURCE_INTERNAL,
      false,
      (time_t)0,
      &_comment_id);     // FIXME DBR: should be computed automatically

  /*** SCHEDULE DOWNTIME - FLEXIBLE (NON-FIXED) DOWNTIME IS HANDLED AT A LATER POINT ***/

  /* only non-triggered downtime is scheduled... */
  if (_triggered_by == 0) {
//    new_downtime_id = new unsigned long;
//    *new_downtime_id = downtime_id;
    unsigned long* id = new unsigned long;
    *id = get_id();
    schedule_new_event(
      EVENT_SCHEDULED_DOWNTIME,
      true,
      _start_time,
      false,
      0,
      NULL,
      false,
      (void*)id,
      NULL,
      0);
  }

#ifdef PROBABLY_NOT_NEEDED
  /*** FLEXIBLE DOWNTIME SANITY CHECK - ADDED 02/17/2008 ****/

  /* if host/service is in a non-OK/UP state right now, see if we should start flexible time immediately */
  /* this is new logic added in 3.0rc3 */
  if (!_fixed)
      _parent->check_pending_flex_downtime();
#endif
  return (OK);
}

unsigned long downtime::get_comment_id() const {
  return _comment_id;
}


/* unschedules a host or service downtime */
int downtime::unschedule() {
  unsigned long downtime_id(get_id());
  host* hst(NULL);
  service* svc(NULL);
  timed_event* temp_event(NULL);
  int attr(0);
  std::string descr;

  logger(dbg_functions, basic)
    << "unschedule_downtime()";

  /* find the host or service associated with this downtime */
  if (get_type() == HOST_DOWNTIME) {
    hst = static_cast<host*>(_parent);
    descr = "";
  }
  else {
    svc = static_cast<service*>(_parent);
    descr = svc->get_description();
  }

  /* decrement pending flex downtime if necessary ... */
  if (!get_fixed()
      && get_incremented_pending_downtime())
    _parent->dec_pending_flex_downtime();

  /* decrement the downtime depth variable and update status data if necessary */
  if (get_in_effect()) {

    /* send data to event broker */
    attr = NEBATTR_DOWNTIME_STOP_CANCELLED;
    broker_downtime_data(
      NEBTYPE_DOWNTIME_STOP,
      NEBFLAG_NONE,
      attr,
      get_type(),
      get_host_name(),
      descr,
      get_entry_time(),
      get_author(),
      get_comment(),
      get_start_time(),
      get_end_time(),
      get_fixed(),
      get_triggered_by(),
      get_duration(),
      get_downtime_id(),
      NULL);

    if (get_type() == HOST_DOWNTIME) {
      hst->dec_scheduled_downtime_depth();
      //FIXME DBR
      //update_host_status(hst, false);

      /* log a notice - this is parsed by the history CGI */
      if (hst->get_scheduled_downtime_depth() == 0) {
        logger(log_info_message, basic)
          << "HOST DOWNTIME ALERT: " << hst->get_host_name()
          << ";CANCELLED; Scheduled downtime for host has been "
          "cancelled.";

        /* send a notification */
        hst->notify(
               notifier::DOWNTIMECANCELLED,
               "",
               "",
               NOTIFICATION_OPTION_NONE);
      }
    }
    else {
      svc->dec_scheduled_downtime_depth();
      // FIXME DBR
      //update_service_status(svc, false);

      /* log a notice - this is parsed by the history CGI */
      if (svc->get_scheduled_downtime_depth()) {

        logger(log_info_message, basic)
          << "SERVICE DOWNTIME ALERT: " << svc->get_host_name() << ";"
          << svc->get_description() << ";CANCELLED; Scheduled downtime "
          "for service has been cancelled.";

        /* send a notification */
        svc->notify(
               notifier::DOWNTIMECANCELLED,
               "",
               "",
               NOTIFICATION_OPTION_NONE);
      }
    }
  }

  /* remove scheduled entry from event queue */
  for (temp_event = event_list_high;
       temp_event != NULL;
       temp_event = temp_event->next) {
    if (temp_event->event_type != EVENT_SCHEDULED_DOWNTIME)
      continue;
    if (((unsigned long)temp_event->event_data) == downtime_id)
      break;
  }
  if (temp_event != NULL)
    remove_event(temp_event, &event_list_high, &event_list_high_tail);

  scheduled_downtime_list.erase(downtime_id);

  /* unschedule all downtime entries that were triggered by this one */
  downtime* dt;
  do {
    std::map<unsigned long, downtime* >::iterator next_it;

    dt = NULL;
    for (std::map<unsigned long, downtime* >::iterator
           it(scheduled_downtime_list.begin()),
           end(scheduled_downtime_list.end());
         it != end;
         it = next_it) {

      next_it = it;
      ++next_it;
      dt = it->second;

      if (dt->get_triggered_by() == downtime_id) {
        dt->unschedule();
        break;
      }
    }
  }
  while (dt);

  return (OK);
}

int downtime::get_incremented_pending_downtime() const {
  return _incremented_pending_downtime;
}

void downtime::set_incremented_pending_downtime() {
  _incremented_pending_downtime = true;
}

void downtime::unset_incremented_pending_downtime() {
  _incremented_pending_downtime = false;
}

bool downtime::get_in_effect() const {
  return _in_effect;
}

void downtime::set_in_effect(bool in_effect) {
  _in_effect = in_effect;
}

notifications::notifier* const downtime::get_parent() const {
  return _parent;
}

void downtime::handle() {
  int attr(0);
  host* hst(NULL);
  service* svc(NULL);
  downtime* this_downtime;
  time_t event_time(0UL);
  std::string descr;

  logger(dbg_functions, basic)
    << "handle downtime " << get_id();

  /* find the host or service associated with this downtime */
  if (get_type() == HOST_DOWNTIME) {
    hst = static_cast<host*>(get_parent());
    descr = "";
  }
  else if (get_type() == SERVICE_DOWNTIME) {
    svc = static_cast<service*>(get_parent());
    descr = svc->get_description();
  }
  else
    throw engine_error()
             << "Error while handling downtime "
             << static_cast<unsigned int>(get_id()) << ": "
             << "notifier is not a host nor a service";

  /* if downtime is flexible and notifier is in an ok state,
     don't do anything right now (wait for event handler to kick it off) */
  /* start_flex_downtime variable is set to true by event handler functions */
  if (!get_fixed()) {

    /* we're not supposed to force a start of flex downtime... */
    if (!get_start_flex_downtime()) {

      /* host is up or service is ok, so we don't really do anything right now */
      if ((get_type() == HOST_DOWNTIME
           && _parent->get_current_state() == HOST_UP)
          || (get_type() == SERVICE_DOWNTIME
              && _parent->get_current_state() == STATE_OK)) {

        _parent->inc_pending_flex_downtime();
        set_incremented_pending_downtime();

        /*** SINCE THE FLEX DOWNTIME MAY NEVER START,
          WE HAVE TO PROVIDE A WAY OF EXPIRING UNUSED DOWNTIME... ***/

        schedule_new_event(
          EVENT_EXPIRE_DOWNTIME,
          true,
          get_end_time() + 1,
          false,
          0,
          NULL,
          false,
          NULL,
          NULL,
          0);
        return ;
      }
    }
  }

  /* have we come to the end of the scheduled downtime? */
  if (get_in_effect()) {

    /* send data to event broker */
    attr = NEBATTR_DOWNTIME_STOP_NORMAL;
    broker_downtime_data(
      NEBTYPE_DOWNTIME_STOP,
      NEBFLAG_NONE,
      attr,
      get_type(),
      get_host_name(),
      descr,
      get_entry_time(),
      get_author(),
      get_comment(),
      get_start_time(),
      get_end_time(),
      get_fixed(),
      get_triggered_by(),
      get_duration(),
      get_id(),
      NULL);

    /* decrement the downtime depth variable */
    _parent->dec_scheduled_downtime_depth();

    if (get_type() == HOST_DOWNTIME
        && _parent->get_scheduled_downtime_depth() == 0) {

      logger(dbg_downtime, basic)
        << "Host '" << hst->get_host_name() << "' has exited from a period "
        "of scheduled downtime (id=" << get_id() << ").";

      /* log a notice - this one is parsed by the history CGI */
      logger(log_info_message, basic)
        << "HOST DOWNTIME ALERT: " << hst->get_host_name()
        << ";STOPPED; Host has exited from a period of scheduled "
        "downtime";

      /* send a notification */
      _parent->notify(
                 notifier::DOWNTIMESTOP,
                 get_author(),
                 get_comment(),
                 NOTIFICATION_OPTION_NONE);
//      host_notification(
//        hst,
//        NOTIFICATION_DOWNTIMEEND,
//        temp_downtime->author,
//        temp_downtime->comment,
//        NOTIFICATION_OPTION_NONE);
    }
    else if (get_type() == SERVICE_DOWNTIME
             && _parent->get_scheduled_downtime_depth() == 0) {

      logger(dbg_downtime, basic)
        << "Service '" << svc->get_description() << "' on host '"
        << svc->get_host_name() << "' has exited from a period of "
        "scheduled downtime (id=" << get_id() << ").";

      /* log a notice - this one is parsed by the history CGI */
      logger(log_info_message, basic)
        << "SERVICE DOWNTIME ALERT: " << svc->get_host_name()
        << ";" << svc->get_description()
        << ";STOPPED; Service has exited from a period of scheduled "
        "downtime";

      /* send a notification */
      svc->notify(
             notifier::DOWNTIMESTOP,
             get_author(),
             get_comment(),
             NOTIFICATION_OPTION_NONE);
//      service_notification(
//        svc,
//        NOTIFICATION_DOWNTIMEEND,
//        temp_downtime->author,
//        temp_downtime->comment,
//        NOTIFICATION_OPTION_NONE);
    }

    /* update the status data */
    _parent->update_status(false);
//    if (temp_downtime->type == HOST_DOWNTIME)
//      update_host_status(hst, false);
//    else
//      update_service_status(svc, false);

    /* decrement pending flex downtime if necessary */
    if (!get_fixed()
        && get_incremented_pending_downtime()) {
      if (_parent->get_pending_flex_downtime() > 0)
        _parent->dec_pending_flex_downtime();
    }

    /* handle (stop) downtime that is triggered by this one */
    std::map<unsigned long, downtime*>::iterator it, end;
    do {
      /* list contents might change by recursive calls, so we use this inefficient method to prevent segfaults */
      for (it = scheduled_downtime_list.begin(),
           end = scheduled_downtime_list.end();
           it != end;
           ++it) {
        this_downtime = it->second;
        if (this_downtime->get_triggered_by() == get_id()) {
          this_downtime->handle();
          break;
        }
      }
    }
    while (it != end);

    /* delete downtime entry */
    scheduled_downtime_list.erase(get_id());
  }
  /* else we are just starting the scheduled downtime */
  else {

    /* send data to event broker */
    broker_downtime_data(
      NEBTYPE_DOWNTIME_START,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      get_type(),
      get_host_name(),
      descr,
      get_entry_time(),
      get_author(),
      get_comment(),
      get_start_time(),
      get_end_time(),
      get_fixed(),
      get_triggered_by(),
      get_duration(),
      get_id(),
      NULL);

    logger(dbg_downtime, basic)
      << _parent->get_info() << " has entered a period of "
      << "scheduled downtime (id=" << get_id() << ").";

    if (get_type() == HOST_DOWNTIME)
      /* log a notice - this one is parsed by the history CGI */
      logger(log_info_message, basic)
        << "HOST DOWNTIME ALERT: " << hst->get_host_name()
        << ";STARTED; Host has entered a period of scheduled downtime";
    else
      /* log a notice - this one is parsed by the history CGI */
      logger(log_info_message, basic)
        << "SERVICE DOWNTIME ALERT: " << svc->get_host_name()
        << ";" << svc->get_description()
        << ";STARTED; Service has entered a period of scheduled "
        "downtime";

    /* send a notification */
    _parent->notify(
      notifier::DOWNTIMESTART,
      get_author(),
      get_comment(),
      NOTIFICATION_OPTION_NONE);

    /* increment the downtime depth variable */
    _parent->inc_scheduled_downtime_depth();

    /* set the in effect flag */
    set_in_effect(true);

    _parent->update_status(false);

    /* schedule an event */
    if (!get_fixed())
      event_time
        = (time_t)((unsigned long)time(NULL) + get_duration());
    else
      event_time = get_end_time();

    /* FIXME DBR: not beautiful :-( */
    unsigned long* new_downtime_id = new unsigned long(get_id());
    schedule_new_event(
      EVENT_SCHEDULED_DOWNTIME,
      true,
      event_time,
      false,
      0,
      NULL,
      false,
      (void*)new_downtime_id,
      NULL,
      0);

    /* handle (start) downtime that is triggered by this one */
    for (std::map<unsigned long, downtime*>::iterator
           it(scheduled_downtime_list.begin()),
           end(scheduled_downtime_list.end());
         it != end;
         ++it) {
      this_downtime = it->second;
      if (get_triggered_by() == get_id())
        this_downtime->handle();
    }
  }
}

bool downtime::get_start_flex_downtime() const {
  return _start_flex_downtime;
}
