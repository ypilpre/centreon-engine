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
#include <sstream>
#include "com/centreon/engine/downtime.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/notifications/notifier.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::notifications;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 * Constructor.
 */
notifier::notifier()
  : _current_notifications(0),
    _last_notification(0),
    _current_notification_number(0),
    _notified_states(0),
    _notification_interval(60),
    _in_downtime(false),
    _scheduled_downtime_depth(0),
    _current_acknowledgement(ACKNOWLEDGEMENT_NONE) {}

/**
 * Copy constructor.
 *
 * @param[in] other Object to copy.
 */
notifier::notifier(notifier const& other)
  : _current_notifications(other._current_notifications),
    _last_notification(other._last_notification),
    _current_notification_number(other._current_notification_number),
    _notified_states(other._notified_states),
    _notification_interval(other._notification_interval),
    _in_downtime(other._in_downtime),
    _scheduled_downtime_depth(other._scheduled_downtime_depth),
    _current_acknowledgement(other._current_acknowledgement) {}

/**
 * Assignment operator.
 *
 * @param[in] other Object to copy.
 *
 * @return This object
 */
notifier& notifier::operator=(notifier const& other) {
  _current_notifications = other._current_notifications;
  _last_notification = other._last_notification;
  _current_notification_number = other._current_notification_number;
  _notified_states = other._notified_states;
  _notification_interval = other._notification_interval;
  return (*this);
}

/**
 * Destructor.
 */
notifier::~notifier() {
  logger(dbg_functions, basic)
    << "notifier: destructor";
}

void notifier::add_contactgroup(shared_ptr<engine::contactgroup> cg) {
  _contact_groups[cg->get_name()] = cg;
}

void notifier::add_contact(shared_ptr<engine::contact> user) {
  _contacts[user->get_name()] = user;
}

/**
 * This method tells if notifications are enabled globally
 *
 * @return a boolean
 */
bool notifier::get_notifications_enabled() const {
  return _notifications_enabled;
  //return config->enable_notifications();
}

void notifier::set_notifications_enabled(bool enabled) {
  _notifications_enabled = enabled;
}
/**
 *  Tell if the current notification is escalated
 *
 *  @return a boolean
 */
bool notifier::should_be_escalated() const {

  logger(dbg_functions, basic)
    << "notifier: should_be_escalated()";

  // FIXME DBR: not implemented
  return false;
}

/**
 *  Deletes all non-persistent acknowledgement comments for a particular
 *  notifier
 */
void notifier::delete_acknowledgement_comments() {
  comment* temp_comment = NULL;
  comment* next_comment = NULL;

  /* delete comments from memory */
  //FIXME DBR: to rewrite
//  for (temp_comment = comment_list;
//       temp_comment != NULL;
//       temp_comment = next_comment) {
//    next_comment = temp_comment->next;
//    if (temp_comment->comment_type == SERVICE_COMMENT
//        && !strcmp(temp_comment->host_name, get_host_name().c_str())
//        && !strcmp(temp_comment->service_description, get_description().c_str())
//        && temp_comment->entry_type == ACKNOWLEDGEMENT_COMMENT
//        && temp_comment->persistent == false)
//      delete_comment(SERVICE_COMMENT, temp_comment->comment_id);
//  }
}

void notifier::notify(
    notification_type type,
    std::string const& author,
    std::string const& comment,
    int options) {

  /* Normal acknowledgement is lost when state changes. We must verify
   * that the acknowledgement is older than the state change.
   */
  /* Sticky acknowledgement is lost when state changes to OK. We must verify
   * that the acknowledgement is older than the state change.
   */
  if (get_last_acknowledgement() < get_last_check()
      && ((get_acknowledgement_type() == ACKNOWLEDGEMENT_NORMAL
          && get_current_state() != get_last_state())
          || (get_acknowledgement_type() == ACKNOWLEDGEMENT_STICKY
          && get_current_state() == 0))) {
    set_acknowledged(ACKNOWLEDGEMENT_NONE);
    delete_acknowledgement_comments();
  }

  std::list<shared_ptr<engine::contact> > users_to_notify = get_contacts_list();
  if (users_to_notify.empty())
    return ;

  if (get_notifications_enabled()) {
    notifier_filter should_notify = _get_filter(type);
    if ((this->*should_notify)()) {
      nagios_macros mac;
      std::ostringstream oss;
      bool first_time = true;

      // Notify each contact
      for (
        std::list<shared_ptr<engine::contact> >::iterator
                                                it(users_to_notify.begin()),
                                                end(users_to_notify.end());
        it != end;
        ++it) {

        logger(dbg_notifications, most)
          << "** Notifying contact '" << (*it)->get_name() << "'";

        if (first_time)
          oss << (*it)->get_name();
        else
          oss << ',' << (*it)->get_name();
      }

      memset(&mac, 0, sizeof(mac));

      // grab the macro variables
      // FIXME DBR: This code cannot work now. Could we improve macros ?
//      grab_host_macros_r(&mac, get_host());
//      if (!_is_host())
//        grab_service_macros_r(&mac, get_service());

      /* The author is a string taken from an external command. It can be
         the contact name or the contact alias. And if the external command
         is badly configured, maybe no contact is associated to this author */
      configuration::contact const* author_contact;
      if (!author.empty()) {
        configuration::set_contact::const_iterator it = config->contacts_find(author);
        if (it != config->contacts().end())
          author_contact = &(*it);
        else
          author_contact = NULL;
      }

      string::setstr(mac.x[MACRO_NOTIFICATIONAUTHOR], author);
      string::setstr(mac.x[MACRO_NOTIFICATIONCOMMENT], comment);
      // FIXME DBR: temp_contact is not defined, see previous lines...
//      if (temp_contact) {
//        string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORNAME], temp_contact->get_name());
//        string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORALIAS], temp_contact->get_alias());
//      }
//      else {
//        string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORNAME]);
//        string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORALIAS]);
//      }

      /* Old Nagios comment: these macros are deprecated and will likely
         disappear in next major release. if this is an acknowledgement,
         get author and comment macros: FIXME: Is it useful for us ??? */
      if (_current_notifications == ACKNOWLEDGEMENT) {
        string::setstr(mac.x[MACRO_SERVICEACKAUTHOR], author);
        string::setstr(mac.x[MACRO_SERVICEACKCOMMENT], comment);
        // FIXME DBR: Same as previous comment
//        if (temp_contact) {
//          string::setstr(mac.x[MACRO_SERVICEACKAUTHORNAME], temp_contact->get_name());
//          string::setstr(mac.x[MACRO_SERVICEACKAUTHORALIAS], temp_contact->get_alias());
//        }
//        else {
//          string::setstr(mac.x[MACRO_SERVICEACKAUTHORNAME]);
//          string::setstr(mac.x[MACRO_SERVICEACKAUTHORALIAS]);
//        }
      }

      // Set the notification type macro
      string::setstr(
                mac.x[MACRO_NOTIFICATIONTYPE],
                _notification_string[_current_notifications]);

      // Set the notification number
      string::setstr(
                mac.x[MACRO_SERVICENOTIFICATIONNUMBER],
                _current_notification_number);

      /* set the notification id macro */
      // FIXME DBR: _current_notification_id is not managed now, the only
      // possible value is 0.
      string::setstr(
                mac.x[MACRO_SERVICENOTIFICATIONID],
                _current_notification_id);

      string::setstr(
                mac.x[MACRO_NOTIFICATIONISESCALATED],
                should_be_escalated());
      string::setstr(
                mac.x[MACRO_NOTIFICATIONRECIPIENTS],
                oss.str());

      _current_notifications |= (1 << type);
      switch (type) {
        case PROBLEM:
          _current_notifications &= ~(1 << RECOVERY);
          break;
        case RECOVERY:
          _current_notifications &= ~(1 << PROBLEM);
          break;
        case FLAPPINGSTART:
          _current_notifications &= ~((1 << FLAPPINGSTOP) | (1 << FLAPPINGDISABLED));
          break;
        case FLAPPINGSTOP:
        case FLAPPINGDISABLED:
          _current_notifications &= ~(1 << FLAPPINGSTART);
          break;
      }
      time(&_last_notification);
    }
  }
}

static bool _compare_shared_ptr(
    shared_ptr<engine::contact> const& a,
    shared_ptr<engine::contact> const& b) {
  return (*a < *b);
}

/**
 *  Clear contact list.
 */
void notifier::clear_contacts() {
  _contacts.clear();
  return ;
}

/**
 *  get the users to notify in a string form each one separated by a comma.
 *
 *  @return A string.
 */
std::list<shared_ptr<engine::contact> > notifier::get_contacts_list() {

  /* See if this notification should be escalated */
  _escalate_notification = should_be_escalated();

  std::list<shared_ptr<engine::contact> > retval;

  for (
    umap<std::string, shared_ptr<engine::contact> >::const_iterator
      it(_contacts.begin()),
      end(_contacts.end());
    it != end;
    ++it) {
    retval.push_back(it->second);
  }

  retval.sort(_compare_shared_ptr);
  retval.unique();
  return retval;
}

/**
 * This method tells if the notifier should notify on current state.
 *
 * @return a boolean
 */
bool notifier::is_state_notification_enabled(int state) const {
  return (_notified_states & (1 << state));
}

/**
 * This method tells if this checkable is in downtime.
 *
 * @return a boolean
 */
bool notifier::is_in_downtime() const {
  return _in_downtime;
}

/**
 * Get the filter method associated to the given type.
 *
 * @param type The notification's type.
 *
 * @return A filter method
 */
notifier::notifier_filter notifier::_get_filter(notification_type type) const {
  return _filter[type];
}

umap<std::string, shared_ptr<engine::contact> > const& notifier::get_contacts() const {
  return _contacts;
}

umap<std::string, shared_ptr<engine::contact> >& notifier::get_contacts() {
  return _contacts;
}

/**
 *  Clear contact group list.
 */
void notifier::clear_contactgroups() {
  _contact_groups.clear();
  return ;
}

umap<std::string, shared_ptr<engine::contactgroup> > const& notifier::get_contactgroups() const {
  return _contact_groups;
}

umap<std::string, shared_ptr<engine::contactgroup> >& notifier::get_contactgroups() {
  return _contact_groups;
}

/**************************************
*                                     *
*           Static Objects            *
*                                     *
**************************************/

notifier::notifier_filter notifier::_filter[] = {
  &notifier::_problem_filter,
  &notifier::_recovery_filter,
  &notifier::_acknowledgement_filter,
  &notifier::_flappingstart_filter,
  &notifier::_flappingstopdisabled_filter,
  &notifier::_flappingstopdisabled_filter,
  &notifier::_downtimestart_filter,
  &notifier::_downtimestopcancelled_filter,
  &notifier::_downtimestopcancelled_filter,
  &notifier::_custom_filter
};

std::string notifier::_notification_string[] = {
  "PROBLEM",
  "RECOVERY",
  "ACKNOWLEDGEMENT",
  "FLAPPINGSTART",
  "FLAPPINGSTOP",
  "FLAPPINGDISABLED",
  "DOWNTIMESTART",
  "DOWNTIMESTOP",
  "DOWNTIMECANCELLED",
  "CUSTOM"
};

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 * Filter method on problem notifications
 *
 * @return a boolean
 */
bool notifier::_problem_filter() {
  time_t now;
  time(&now);
  /* No notification sent if:
   *  * notifier in downtime
   *  * notifier is flapping
   *  * notifier is not configured to send notification on the current state
   *  * state is not hard.
   */
  if (is_in_downtime()
      || get_flapping()
      || !is_state_notification_enabled(get_current_state())
      || get_last_hard_state_change() < get_last_state_change()
      || is_acknowledged())
    return false;

  int notif_number = get_current_notification_number();

  /* A PROBLEM notification has already been sent */
  if (notif_number >= 1
      && (get_current_notifications_flag() & (1 << PROBLEM))
      && get_last_state() == get_current_state()) {
    /* No notification if the delay between previous notification and now
       is less than notification_interval */
    if (now - get_last_notification() < get_notification_interval())
      return false;

  }
  return true;
}

unsigned int notifier::get_current_notifications_flag() const {
  return _current_notifications;
}

/**
 * Filter method on recovery notifications
 *
 * @return a boolean
 */
bool notifier::_recovery_filter() {

  if (is_in_downtime()
      || (get_current_notifications_flag() & (1 << PROBLEM)) == 0
      || get_current_state() != 0)
    return false;

  return true;
}

/**
 * Filter method on acknowledgement notifications
 *
 * @return a boolean
 */
bool notifier::_acknowledgement_filter() {

  if (is_in_downtime()
      || (get_current_notifications_flag() & (1 << PROBLEM)) == 0
      || get_current_state() == 0)
    return false;

  return true;
}

/**
 * Filter method on flappingstart notifications
 *
 * @return a boolean
 */
bool notifier::_flappingstart_filter() {

  if (is_in_downtime()
      || (get_current_notifications_flag() & (1 << FLAPPINGSTART)))
    return false;

  return true;
}

/**
 * Filter method on flapping stop/disabled notifications
 *
 * @return a boolean
 */
bool notifier::_flappingstopdisabled_filter() {

  if (is_in_downtime()
      || (get_current_notifications_flag() & (1 << FLAPPINGSTART)) == 0)
    return false;

  return true;
}

/**
 * Filter method on downtime start notifications
 *
 * @return a boolean
 */
bool notifier::_downtimestart_filter() {

  if (is_in_downtime())
    return false;

  return true;
}

/**
 * Filter method on downtime stop/cancelled notifications
 *
 * @return a boolean
 */
bool notifier::_downtimestopcancelled_filter() {

  if (!is_in_downtime())
    return false;

  return true;
}

/**
 * Filter method on custom notifications
 *
 * @return a boolean
 */
bool notifier::_custom_filter() {

  if (is_in_downtime())
    return false;

  return true;
}

int notifier::get_current_notification_id() const {
  return (_current_notification_id);
}

void notifier::set_current_notification_id(int id) {
  _current_notification_id = id;
  return ;
}

/**
 * Getter to the notification number.
 *
 * @return an integer.
 */
int notifier::get_current_notification_number() const {
  return _current_notification_number;
}

long notifier::get_notification_interval() const {
  return _notification_interval;
}

void notifier::set_notification_interval(long interval) {
  _notification_interval = interval;
  return ;
}

time_t notifier::get_last_notification() const {
  return _last_notification;
}

void notifier::enable_state_notification(int state) {
  _notified_states |= (1 << state);
}

void notifier::set_last_notification(time_t last_notification) {
  _last_notification = last_notification;
}

void notifier::set_next_notification(time_t next_notification) {
  _next_notification = next_notification;
}

int notifier::get_first_notification_delay() const {
  return (_first_notification_delay);
}

void notifier::set_first_notification_delay(int delay) {
  _first_notification_delay = delay;
  return ;
}

int notifier::get_recovery_notification_delay() const {
  return (_recovery_notification_delay);
}

void notifier::set_recovery_notification_delay(int delay) {
  _recovery_notification_delay = delay;
  return ;
}

bool notifier::contains_contact(contact* user) const {
  std::string const& name(user->get_name());
  return contains_contact(name);
}

bool notifier::contains_contact(std::string const& username) const {
  umap<std::string, shared_ptr<contact> >::const_iterator it(
    get_contacts().find(username));
  if (it != get_contacts().end())
    return true;

  for (umap<std::string, shared_ptr<engine::contactgroup> >::const_iterator
         cgit(get_contactgroups().begin()),
         end(get_contactgroups().end());
       cgit != end;
       ++cgit) {
    if (cgit->second->contains_member(username))
      return true;
  }
  return false;
}

bool notifier::is_acknowledged() const {
  return (_current_acknowledgement != ACKNOWLEDGEMENT_NONE);
}

notifier::acknowledgement_type notifier::get_acknowledgement_type() const {
  return _current_acknowledgement;
}

void notifier::set_acknowledged(acknowledgement_type type) {
  _current_acknowledgement = type;
}

void notifier::set_initial_notif_time(time_t initial) {
  // FIXME DBR: to implement...
}

void notifier::set_recovery_been_sent(bool sent) {
  // FIXME DBR: to implement...
}

/**
 *  Check if object should notify on down states.
 *
 *  @return True if object should notify.
 */
bool notifier::get_notify_on_down() const {
  return (_notified_states & ON_DOWN);
}

/**
 *  Set whether or not object should notify on down states.
 *
 *  @param[in] notify  True to notify.
 */
void notifier::set_notify_on_down(bool notify) {
  if (notify)
    _notified_states |= ON_DOWN;
  else
    _notified_states &= ~ON_DOWN;
  return ;
}

/**
 *  Check if object should notify when in downtime.
 *
 *  @return True if object should notify.
 */
bool notifier::get_notify_on_downtime() const {
  return (_notified_states & ON_DOWNTIME);
}

/**
 *  Set whether or not not object should notify when flapping.
 *
 *  @param[in] notify  True to notify.
 */
void notifier::set_notify_on_downtime(bool notify) {
  if (notify)
    _notified_states |= ON_DOWNTIME;
  else
    _notified_states &= ~ON_DOWNTIME;
  return ;
}

/**
 *  Check if object should notify when flapping.
 *
 *  @return True if object should notify.
 */
bool notifier::get_notify_on_flapping() const {
  return (_notified_states & ON_FLAPPING);
}

/**
 *  Set whether or not not object should notify when flapping.
 *
 *  @param[in] notify  True to notify.
 */
void notifier::set_notify_on_flapping(bool notify) {
  if (notify)
    _notified_states |= ON_FLAPPING;
  else
    _notified_states &= ~ON_FLAPPING;
  return ;
}

/**
 *  Check if object should notify when recovering.
 *
 *  @return True if object should notify.
 */
bool notifier::get_notify_on_recovery() const {
  return (_notified_states & ON_RECOVERY);
}

/**
 *  Set whether or not not object should notify when recovering.
 *
 *  @param[in] notify  True to notify.
 */
void notifier::set_notify_on_recovery(bool notify) {
  if (notify)
    _notified_states |= ON_RECOVERY;
  else
    _notified_states &= ~ON_RECOVERY;
  return ;
}

/**
 *  Check if object should notify on unreachable states.
 *
 *  @return True if object should notify.
 */
bool notifier::get_notify_on_unreachable() const {
  return (_notified_states & ON_UNREACHABLE);
}

/**
 *  Set whether or not object should notify on unreachable states.
 *
 *  @param[in] notify  True to notify.
 */
void notifier::set_notify_on_unreachable(bool notify) {
  if (notify)
    _notified_states |= ON_UNREACHABLE;
  else
    _notified_states &= ~ON_UNREACHABLE;
  return ;
}

timeperiod* notifier::get_notification_period() const {
  // FIXME DBR: to implement...
}

void notifier::set_notification_period(timeperiod* tperiod) {
  // FIXME DBR: to implement...
}

bool notifier::get_recovery_been_sent() const {
  // FIXME DBR: to implement...
  return false;
}

void notifier::set_current_notification_number(int number) {
  // FIXME DBR: to implement...
}

int notifier::get_pending_flex_downtime() const {
  return _pending_flex_downtime;
}

void notifier::inc_pending_flex_downtime() {
  ++_pending_flex_downtime;
}

void notifier::dec_pending_flex_downtime() {
  --_pending_flex_downtime;
}

/////////////////////////////////////////////////////////
///* checks for flexible (non-fixed) host downtime that should start now */
//int check_pending_flex_host_downtime(host* hst) {
//  scheduled_downtime* temp_downtime(NULL);
//  time_t current_time(0L);
//
//  logger(dbg_functions, basic)
//    << "check_pending_flex_host_downtime()";
//
//  if (hst == NULL)
//    return (ERROR);
//
//  time(&current_time);
//
//  /* if host is currently up, nothing to do */
//  if (hst->current_state == HOST_UP)
//    return (OK);
//
//  /* check all downtime entries */
//  for (temp_downtime = scheduled_downtime_list;
//       temp_downtime != NULL;
//       temp_downtime = temp_downtime->next) {
//    if (temp_downtime->type != HOST_DOWNTIME
//        || temp_downtime->fixed == true
//        || temp_downtime->is_in_effect == true
//        || temp_downtime->triggered_by != 0)
//      continue;
//
//    /* this entry matches our host! */
//    if (find_host(temp_downtime->host_name) == hst) {
//      /* if the time boundaries are okay, start this scheduled downtime */
//      if (temp_downtime->start_time <= current_time
//          && current_time <= temp_downtime->end_time) {
//
//        logger(dbg_downtime, basic)
//          << "Flexible downtime (id=" << temp_downtime->downtime_id
//          << ") for host '" << hst->name << "' starting now...";
//
//        temp_downtime->start_flex_downtime = true;
//        handle_scheduled_downtime(temp_downtime);
//      }
//    }
//  }
//  return (OK);
//}
//
/////////////////////////////////////////////////////////

/**
 *  Checks for flexible (non-fixed) notifier downtime that should start now.
 */
void notifier::check_pending_flex_downtime() {
  time_t current_time(0L);

  logger(dbg_functions, basic)
    << "check_pending_flex_downtime()";

  time(&current_time);

  /* if notifier is currently ok, nothing to do */
  if (get_current_state() == STATE_OK)
    return ;

  /* Check all downtime entries */
  for (std::map<unsigned long, downtime*>::const_iterator
         it(scheduled_downtime_list.begin()),
         end(scheduled_downtime_list.begin());
       it != end;
       ++it) {
    downtime* temp_downtime = it->second;

    if (temp_downtime->get_type() != downtime::SERVICE_DOWNTIME
        || temp_downtime->get_fixed()
        || temp_downtime->get_in_effect()
        || temp_downtime->get_triggered_by() != 0)
      continue;

    /* This entry matches our notifier */
    /* and the time boundaries are ok, start this scheduled downtime */
    if (temp_downtime->get_parent() == this
        && temp_downtime->get_start_time() <= current_time
        && current_time <= temp_downtime->get_end_time()) {
      logger(dbg_downtime, basic)
        << "Flexible downtime (id=" << temp_downtime->get_id()
        << ") for notifier '" << get_info() << "' starting now...";

      temp_downtime->handle();
    }
  }
}
time_t notifier::get_next_notification() const {
  // FIXME DBR: to implement...
  return 0;
}

bool notifier::get_no_more_notifications() const {
  // FIXME DBR: to implement...
  return true;
}

int notifier::get_scheduled_downtime_depth() const {
  return 0;
}

void notifier::set_last_acknowledgement(time_t last_acknowledgement) {
  _last_acknowledgement = last_acknowledgement;
}

time_t notifier::get_last_acknowledgement() const {
  return _last_acknowledgement;
}

void notifier::inc_scheduled_downtime_depth() {
  ++_scheduled_downtime_depth;
}

void notifier::dec_scheduled_downtime_depth() {
  --_scheduled_downtime_depth;
}

int notifier::schedule_downtime(
      downtime::downtime_type type,
      time_t entry_time,
      std::string const& author,
      std::string const& comment_data,
      time_t start_time,
      time_t end_time,
      bool fixed,
      unsigned long triggered_by,
      unsigned long duration,
      downtime_propagation propagate) {

  logger(dbg_functions, basic)
    << "schedule_downtime()";

  /* don't add old or invalid downtimes */
  if (start_time >= end_time || end_time <= time(NULL))
    return (ERROR);

  /* add a new downtime entry */
  downtime* dt(new downtime(
                       type,
                       this,
                       entry_time,
                       author,
                       comment_data,
                       start_time,
                       end_time,
                       fixed,
                       triggered_by,
                       duration));
  if (scheduled_downtime_list.empty())
    dt->set_id(1);
  else
    dt->set_id(scheduled_downtime_list.rbegin()->first + 1);
  scheduled_downtime_list.insert(std::make_pair(dt->get_id(), dt));

  /* register the scheduled downtime */
  dt->registration();

  if (type == downtime::HOST_DOWNTIME) {
    host* temp_host = static_cast<host*>(this);

    int trig;
    downtime_propagation propagate;

    switch (propagate) {
      case DOWNTIME_PROPAGATE_NONE:
        trig = 0UL;
        propagate = DOWNTIME_PROPAGATE_NONE;
        break;
      case DOWNTIME_PROPAGATE_SIMPLE:
        trig = dt->get_triggered_by();
        propagate = DOWNTIME_PROPAGATE_NONE;
        break;
      case DOWNTIME_PROPAGATE_TRIGGERED:
        trig = dt->get_triggered_by();
        propagate = DOWNTIME_PROPAGATE_TRIGGERED;
        break;
    }
    /* check all child hosts... */
    for (host_set::const_iterator
           it(temp_host->get_children().begin()),
           end(temp_host->get_children().end());
         it != end;
         ++it) {
      shared_ptr<host> child_host(*it);

      /* recurse... */
      child_host->schedule_downtime(
        downtime::HOST_DOWNTIME,
        entry_time,
        author,
        comment_data,
        start_time,
        end_time,
        fixed,
        trig,
        duration,
        propagate);
    }
  }
  return (OK);
}

std::string notifier::get_info() {
  std::ostringstream oss;
  service* svc = dynamic_cast<service*>(this);

  if (svc)
    oss << "Notifier: service '" << svc->get_description() << "' ; host '"
        << svc->get_host_name() << "'";
  else
    oss << "Notifier: host '"
        << svc->get_host_name() << "'";
  return oss.str();
}
