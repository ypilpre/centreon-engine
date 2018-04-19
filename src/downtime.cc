/*
** Copyright 2017-2018 Centreon
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

#include <memory>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/downtime.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/engine/statusdata.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::notifications;

/**
 *  Constructor.
 */
downtime::downtime(
            unsigned long id,
            notifications::notifier* parent,
            time_t entry_time,
            std::string const& author,
            std::string const& comment_data,
            unsigned long comment_id,
            time_t start_time,
            time_t end_time,
            bool fixed,
            unsigned long duration,
            unsigned long triggered_by)
  : _id(id),
    _parent(parent),
    _entry_time(entry_time),
    _author(author),
    _comment_data(comment_data),
    _start_time(start_time),
    _end_time(end_time),
    _fixed(fixed),
    _triggered_by(triggered_by),
    _comment_id(comment_id),
    _duration(duration),
    _in_effect(false) {}

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
  std::string hst_name;
  std::string svc_descr;

  /* remove the downtime from the list in memory */
  /* first remove the comment associated with this downtime */
  if (get_type() == HOST_DOWNTIME) {
    hst_name = static_cast<host*>(get_parent())->get_name();
    svc_descr = "";
  }
  else {
    service* svc(static_cast<service*>(get_parent()));
    hst_name = svc->get_host_name();
    svc_descr = svc->get_description();
  }
  comment::delete_comment(get_comment_id());

  /* send data to event broker */
  broker_downtime_data(
    NEBTYPE_DOWNTIME_DELETE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    get_type(),
    hst_name,
    svc_descr,
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
  _comment_id = other._comment_id;
  _start_time = other._start_time;
  _end_time = other._end_time;
  _fixed = other._fixed;
  _triggered_by = other._triggered_by;
  _duration = other._duration;
  _id = other._id;
  _in_effect = other._in_effect;
}

downtime::downtime_type downtime::get_type() const {
  return (_parent->is_host() ? HOST_DOWNTIME : SERVICE_DOWNTIME);
}

unsigned long downtime::get_id() const {
  return _id;
}

void downtime::set_id(unsigned long id) {
  _id = id;
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

bool downtime::get_fixed() const {
  return (_fixed);
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

unsigned long downtime::get_comment_id() const {
  return _comment_id;
}

void downtime::set_comment_id(unsigned long comment_id) {
  _comment_id = comment_id;
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

/**
 *  Start this downtime.
 */
void downtime::start() {
  // Debug.
  logger(dbg_functions, basic) << "start downtime #" << get_id();

  // Cannot start an already started downtime.
  if (get_in_effect())
    return ;
  set_in_effect(true);

  // Get real target.
  union {
    host* hst;
    service* svc;
  } target;
  if (_parent->is_host())
    target.hst = static_cast<host*>(_parent);
  else
    target.svc = static_cast<service*>(_parent);

  // Send data to event broker.
  if (_parent->is_host())
    broker_downtime_data(
      NEBTYPE_DOWNTIME_START,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      HOST_DOWNTIME,
      target.hst->get_name(),
      "",
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
  else
    broker_downtime_data(
      NEBTYPE_DOWNTIME_START,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      SERVICE_DOWNTIME,
      target.svc->get_host_name(),
      target.svc->get_description(),
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

  // Log info.
  logger(dbg_downtime, basic)
    << _parent->get_info() << " has entered a period of "
    << "scheduled downtime (id=" << get_id() << ").";
  if (_parent->is_host())
    logger(log_info_message, basic)
      << "HOST DOWNTIME ALERT: " << target.hst->get_name()
      << ";STARTED; Host has entered a period of scheduled downtime";
  else
    logger(log_info_message, basic)
      << "SERVICE DOWNTIME ALERT: " << target.svc->get_host_name()
      << ";" << target.svc->get_description()
      << ";STARTED; Service has entered a period of scheduled "
         "downtime";

  // Send a notification.
  _parent->notify(
    notifier::DOWNTIMESTART,
    get_author(),
    get_comment(),
    NOTIFICATION_OPTION_NONE);

  // Increment the downtime depth variable.
  _parent->inc_scheduled_downtime_depth();

  // Send broker info.
  if (_parent->is_host())
    broker_host_status(target.hst);
  else
    broker_service_status(target.svc);

  // Remove old expiration event.
  for (timed_event* ev(event_list_high); ev; ev = ev->next)
    if ((ev->event_type == EVENT_EXPIRE_DOWNTIME)
        && (*static_cast<unsigned long*>(ev->event_data) == get_id())) {
      remove_event(ev, &event_list_high, &event_list_high_tail);
      break ;
    }

  // Schedule new expiration event.
  time_t event_time;
  if (!get_fixed())
    event_time = (time_t)(time(NULL) + get_duration());
  else
    event_time = get_end_time();
  std::auto_ptr<unsigned long>
    new_downtime_id(new unsigned long(get_id()));
  schedule_new_event(
    EVENT_EXPIRE_DOWNTIME,
    true,
    event_time,
    false,
    0,
    NULL,
    true,
    new_downtime_id.get(),
    NULL,
    0);
  new_downtime_id.release();

  return ;
}

/**
 *  Stop this downtime.
 */
void downtime::stop() {
  // Debug.
  logger(dbg_functions, basic) << "stop downtime #" << get_id();

  // Get real target.
  union {
    host* hst;
    service* svc;
  } target;
  if (_parent->is_host())
    target.hst = static_cast<host*>(_parent);
  else
    target.svc = static_cast<service*>(_parent);

  // Have we come to the end of the scheduled downtime?
  if (get_in_effect()) {
    // Decrement the downtime depth variable.
    _parent->dec_scheduled_downtime_depth();

    // Send data to event broker if target is not in downtime anymore.
    if (!_parent->get_scheduled_downtime_depth()) {
      if (_parent->is_host()) {
        broker_downtime_data(
          NEBTYPE_DOWNTIME_STOP,
          NEBFLAG_NONE,
          NEBATTR_DOWNTIME_STOP_NORMAL,
          HOST_DOWNTIME,
          target.hst->get_name(),
          "",
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
          << "Host '" << target.hst->get_name()
          << "' has exited from a period of scheduled downtime (id="
          << get_id() << ").";
        logger(log_info_message, basic)
          << "HOST DOWNTIME ALERT: " << target.hst->get_name()
          << ";STOPPED; Host has exited from a period of scheduled "
             "downtime";
      }
      else {
        broker_downtime_data(
          NEBTYPE_DOWNTIME_STOP,
          NEBFLAG_NONE,
          NEBATTR_DOWNTIME_STOP_NORMAL,
          SERVICE_DOWNTIME,
          target.svc->get_host_name(),
          target.svc->get_description(),
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
          << "Service '" << target.svc->get_description()
          << "' on host '" << target.svc->get_host_name()
          << "' has exited from a period of "
             "scheduled downtime (id=" << get_id() << ").";
        logger(log_info_message, basic)
          << "SERVICE DOWNTIME ALERT: " << target.svc->get_host_name()
          << ";" << target.svc->get_description()
          << ";STOPPED; Service has exited from a period of scheduled "
             "downtime";
      }
      _parent->notify(
        notifier::DOWNTIMESTOP,
        get_author(),
        get_comment(),
        NOTIFICATION_OPTION_NONE);
    }

    // Update the status data.
    if (_parent->is_host())
      broker_host_status(target.hst);
    else
      broker_service_status(target.svc);
  }

  // Decrement pending flex downtime if necessary.
  if (!get_fixed() && get_incremented_pending_downtime()) {
    if (_parent->get_pending_flex_downtime() > 0)
      _parent->dec_pending_flex_downtime();
  }

  return ;
}
