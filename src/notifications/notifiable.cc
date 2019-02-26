/*
** Copyright 2017-2019 Centreon
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
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/contactgroup.hh"
#include "com/centreon/engine/downtime_manager.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/macros/grab_host.hh"
#include "com/centreon/engine/macros/grab_service.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/notifications/notifiable.hh"
#include "com/centreon/engine/timeperiod.hh"
#include "com/centreon/engine/timezone_locker.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::notifications;

static char const* notification_types_strings[] = {
  "NORMAL",
  "NORMAL",
  "ACKNOWLEDGEMENT",
  "FLAPPINGSTART",
  "FLAPPINGSTOP",
  "FLAPPINGDISABLED",
  "DOWNTIMESTART",
  "DOWNTIMEEND",
  "DOWNTIMECANCELLED",
  "CUSTOM"
};

static char const* host_states_strings[] = {
  "UP",
  "DOWN",
  "UNREACHABLE",
  "UNKNOWN"
};

static char const* service_states_strings[] = {
  "OK",
  "WARNING",
  "CRITICAL",
  "UNKNOWN"
};

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
notifiable::notifiable()
  : _acknowledgement_timeout(0),
    _current_acknowledgement(ACKNOWLEDGEMENT_NONE),
    _current_notification_id(0),
    _current_notification_number(0),
    _escalate_notification(false),
    _first_notification(0),
    _first_notification_delay(0),
    _last_acknowledgement(0),
    _last_notification(0),
    _next_notification(0),
    _no_more_notifications(false),
    _notification_interval(5 * 60),
    _notification_period(NULL),
    _notifications_enabled(true),
    _notified_states(0),
    _pending_flex_downtime(0),
    _recovery_notification_delay(0),
    _scheduled_downtime_depth(0) {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
notifiable::notifiable(notifiable const& other) : checks::checkable(other) {
  _internal_copy(other);
}

/**
 * Destructor.
 */
notifiable::~notifiable() {
  logger(dbg_functions, basic) << "notifiable: destructor";
}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object
 */
notifiable& notifiable::operator=(notifiable const& other) {
  if (this != &other) {
    checks::checkable::operator=(other);
    _internal_copy(other);
  }
  return (*this);
}

/**************************************
*                                     *
*            Configuration            *
*                                     *
**************************************/

/**
 *  Get first notification delay.
 *
 *  @return First notification delay in seconds.
 */
int notifiable::get_first_notification_delay() const {
  return (_first_notification_delay);
}

/**
 *  Set first notification delay.
 *
 *  @param[in] delay  Delay in seconds.
 */
void notifiable::set_first_notification_delay(int delay) {
  _first_notification_delay = delay;
  return ;
}

/**
 *  Get notification period.
 *
 *  @return Pointer to the notification period object.
 */
timeperiod* notifiable::get_notification_period() const {
  return (_notification_period);
}

/**
 *  Set notification period of this notifiable.
 *
 *  @param[in] tperiod  Pointer to timeperiod.
 */
void notifiable::set_notification_period(timeperiod* tperiod) {
  _notification_period = tperiod;
  return ;
}

/**
 *  Get notification interval.
 *
 *  @return Notification interval in seconds.
 */
long notifiable::get_notification_interval() const {
  return (_notification_interval);
}

/**
 *  Set notification interval.
 *
 *  @param[in] interval  New notification interval in seconds.
 */
void notifiable::set_notification_interval(long interval) {
  _notification_interval = interval;
  return ;
}

/**
 *  Check if notifications are enabled for this notifiable.
 *
 *  @return True if this notifiable has notifications enabled.
 */
bool notifiable::get_notifications_enabled() const {
  return (_notifications_enabled);
}

/**
 *  Enable or disable notifications for this notifiable.
 *
 *  @param[in] enable  True to enable.
 */
void notifiable::set_notifications_enabled(bool enable) {
  _notifications_enabled = enable;
  return ;
}

/**
 *  Check if notification is enabled on some state.
 *
 *  @param[in] state  Target state.
 *
 *  @return True if notification is enabled for the specified state.
 */
bool notifiable::get_notify_on(action_on state) const {
  return (_notified_states & state);
}

/**
 *  Enable or disable notification on a state.
 *
 *  @param[in] state   State.
 *  @param[in] enable  True to enable.
 */
void notifiable::set_notify_on(action_on state, bool enable) {
  if (enable)
    _notified_states |= state;
  else
    _notified_states &= ~state;
  return ;
}

/**
 *  This method tells if the notifiable should notify for a specific
 *  state.
 *
 *  @param[in] state  Target state.
 *
 *  @return True if notifications are enabled for state.
 */
bool notifiable::is_state_notification_enabled(int state) const {
  return (_notified_states & (1 << state));
}

/**
 *  Get recovery notification delay.
 *
 *  @return Recovery notification delay in seconds.
 */
int notifiable::get_recovery_notification_delay() const {
  return (_recovery_notification_delay);
}

/**
 *  Set recovery notification delay.
 *
 *  @param[in] delay  New recovery notification delay in seconds.
 */
void notifiable::set_recovery_notification_delay(int delay) {
  _recovery_notification_delay = delay;
  return ;
}

/**************************************
*                                     *
*               Runtime               *
*                                     *
**************************************/

/**
 *  Get current notification ID.
 *
 *  @return An integer identifying uniquely the current notification.
 */
int notifiable::get_current_notification_id() const {
  return (_current_notification_id);
}

/**
 *  Set current notification ID.
 *
 *  @param[in] id  New notification ID.
 */
void notifiable::set_current_notification_id(int id) {
  _current_notification_id = id;
  return ;
}

/**
 *  Get the current notification number.
 *
 *  @return The number of the current notification.
 */
int notifiable::get_current_notification_number() const {
  return _current_notification_number;
}

/**
 *  Set the current notification number.
 *
 *  @param[in] number  New notification number.
 */
void notifiable::set_current_notification_number(int number) {
  _current_notification_number = number;
  return ;
}

/**
 *  Get the first time a notification serie was sent.
 *
 *  @return Timestamp.
 */
time_t notifiable::get_first_notification() const {
  return (_first_notification);
}

/**
 *  Set the first time a notification was supposed to be sent in the
 *  current notification serie.
 *
 *  @param[in] first_notification  Timestamp.
 */
void notifiable::set_first_notification(time_t first_notification) {
  _first_notification = first_notification;
  return ;
}

/**
 *  Get the last time a notification was sent.
 *
 *  @return Timestamp.
 */
time_t notifiable::get_last_notification() const {
  return _last_notification;
}

/**
 *  Set the last time a notification was sent.
 *
 *  @param[in] last_notification  Timestamp.
 */
void notifiable::set_last_notification(time_t last_notification) {
  _last_notification = last_notification;
  return ;
}

/**
 *  Get the time at which the next notification is expected to be sent.
 *
 *  @return Timestamp.
 */
time_t notifiable::get_next_notification() const {
  return (_next_notification);
}

/**
 *  Set when the next notification is expected to be sent.
 *
 *  @param[in] next_notification  Timestamp.
 */
void notifiable::set_next_notification(time_t next_notification) {
  _next_notification = next_notification;
  return ;
}

/**
 *  Check if notifications should be sent again.
 *
 *  @return True if no more notifications should be sent.
 */
bool notifiable::get_no_more_notifications() const {
  return (_no_more_notifications);
}

/**
 *  Set the *no more notifications* flag.
 *
 *  @param[in] no_more  True to disable further notifications.
 */
void notifiable::set_no_more_notifications(bool no_more) {
  _no_more_notifications = no_more;
  return ;
}

/**
 *  Attempt to send a notification.
 *
 *  @param[in] type     Type of notification.
 *  @param[in] author   Author name.
 *  @param[in] comment  Comment.
 *  @param[in] options  Options.
 */
void notifiable::notify(
                 notification_type type,
                 std::string const& author,
                 std::string const& comment,
                 int options) {
  // Debug.
  time_t now(time(NULL));
  {
    time_t last_notification(get_last_notification());
    logger(logging::dbg_notifications, logging::basic)
      << "** Notification Attempt ** " << get_info()
      << "', Type: " << type << ", Options: " << options
      << ", Current State: " << get_current_state()
      << ", Last Notification: " << my_ctime(&last_notification);
  }

  // Viability checks.
  if (!_is_notification_viable(type, options)) {
    logger(logging::dbg_notifications, logging::basic)
      << "Notification viability test failed. No notification will "
         "be sent out.";
  }
  else {
    logger(logging::dbg_notifications, logging::basic)
      << "Notification viability test passed.";

    // Should the notification number be increased ?
    bool incremented_notification_number(false);
    if ((type == PROBLEM)
        || (type == RECOVERY)
        || (options & NOTIFICATION_OPTION_INCREMENT)) {
      ++_current_notification_number;
      incremented_notification_number = true;
    }
    logger(logging::dbg_notifications, logging::more)
      << "Current notification number: "
      << get_current_notification_number()
      << (incremented_notification_number ? " (incremented)" : " (unchanged)");

    // Save and increase the current notification id.
    _current_notification_id = next_notification_id++;

    // Create users list to notify.
    logger(logging::dbg_notifications, logging::most)
      << "Creating list of contacts to be notified.";
    umap<std::string, engine::contact*> users_to_notify(_contacts);
    for (umap<std::string, engine::contactgroup*>::const_iterator
           grp_it(_contact_groups.begin()),
           grp_end(_contact_groups.end());
         grp_it != grp_end;
         ++grp_it) {
      for (umap<std::string, engine::contact*>::const_iterator
             cntct_it(grp_it->second->get_members().begin()),
             cntct_end(grp_it->second->get_members().end());
           cntct_it != cntct_end;
           ++cntct_it)
        users_to_notify[cntct_it->second->get_name()] = cntct_it->second;
    }
    // Debug.
    std::string recipients;
    {
      std::ostringstream oss;
      bool first_time(true);
      for (umap<std::string, engine::contact*>::const_iterator
             it(users_to_notify.begin()),
             end(users_to_notify.end());
           it != end;
           ++it) {
        logger(dbg_notifications, most)
          << "** Notifying contact '" << it->second->get_name() << "'";
        if (first_time) {
          oss << it->second->get_name();
          first_time = false;
        }
        else
          oss << ',' << it->second->get_name();
      }
      recipients = oss.str();
    }

    // Send data to event broker.
    int neb_result;
    {
      struct timeval start_time_tv;
      memset(&start_time_tv, 0, sizeof(start_time_tv));
      start_time_tv.tv_sec = now;
      struct timeval end_time_tv;
      memset(&end_time_tv, 0, sizeof(end_time_tv));
      neb_result = broker_notification_data(
        NEBTYPE_NOTIFICATION_START,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        is_host() ? HOST_NOTIFICATION : SERVICE_NOTIFICATION,
        type,
        start_time_tv,
        end_time_tv,
        this,
        author.c_str(),
        comment.c_str(),
        false, // XXX escalated,
        0,
        NULL);
    }
    if ((neb_result == NEBERROR_CALLBACKCANCEL)
        || (neb_result == NEBERROR_CALLBACKOVERRIDE))
      return ;

    // Set initial notification time if not already set.
    if ((type == PROBLEM) && !_first_notification)
      _first_notification = time(NULL);

    // Cancel sending of recovery notification if recovery
    // notification delay not elapsed. Keep other processing.
    if ((type != RECOVERY)
        || (_first_notification + _recovery_notification_delay <= time(NULL))) {
      nagios_macros mac;
      memset(&mac, 0, sizeof(mac));

      // Host/service base macros.
      if (is_host()) {
        host* hst = static_cast<host*>(this);
        grab_host_macros_r(&mac, hst);
      }
      else {
        service* svc = static_cast<service*>(this);
        grab_host_macros_r(&mac, svc->get_host());
        grab_service_macros_r(&mac, svc);
      }

      // Notification author macros.
      //
      // The author is a string taken from an external command. It can
      // be the contact name or the contact alias. And if the external
      // command is badly configured, maybe no contact is associated to
      // this author.
      engine::contact* author_contact(NULL);
      if (!author.empty()) {
        contact_map::iterator it(
          configuration::applier::state::instance().contacts().find(author));
        if (it != configuration::applier::state::instance().contacts().end())
          author_contact = it->second.get();
        else
          author_contact = NULL;
      }
      string::setstr(mac.x[MACRO_NOTIFICATIONAUTHOR], author);
      string::setstr(mac.x[MACRO_NOTIFICATIONCOMMENT], comment);
      if (author_contact) {
        string::setstr(
                  mac.x[MACRO_NOTIFICATIONAUTHORNAME],
                  author_contact->get_name());
        string::setstr(
                  mac.x[MACRO_NOTIFICATIONAUTHORALIAS],
                  author_contact->get_alias());
      }
      else {
        string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORNAME]);
        string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORALIAS]);
      }

      // Acknowledgement macros.
      if (type == ACKNOWLEDGEMENT) {
        string::setstr(mac.x[MACRO_SERVICEACKAUTHOR], author);
        string::setstr(mac.x[MACRO_SERVICEACKCOMMENT], comment);
        if (author_contact) {
          string::setstr(
                    mac.x[MACRO_SERVICEACKAUTHORNAME],
                    author_contact->get_name());
          string::setstr(
                    mac.x[MACRO_SERVICEACKAUTHORALIAS],
                    author_contact->get_alias());
        }
        else {
          string::setstr(mac.x[MACRO_SERVICEACKAUTHORNAME]);
          string::setstr(mac.x[MACRO_SERVICEACKAUTHORALIAS]);
        }
      }

      // Notification macros.
      string::setstr(
                mac.x[MACRO_NOTIFICATIONTYPE],
                  _notification_string[type]);
      string::setstr(
                mac.x[MACRO_SERVICENOTIFICATIONNUMBER],
                _current_notification_number);
      string::setstr(
                mac.x[MACRO_SERVICENOTIFICATIONID],
                _current_notification_id);
      string::setstr(
                mac.x[MACRO_NOTIFICATIONISESCALATED],
                should_be_escalated());
      string::setstr(
                mac.x[MACRO_NOTIFICATIONRECIPIENTS],
                recipients);

      // Notify contacts.
      int contacts_notified(0);
      for (umap<std::string, engine::contact*>::iterator
             it_cntct(users_to_notify.begin()),
             end_cntct(users_to_notify.end());
           it_cntct != end_cntct;
           ++it_cntct) {
        // Grab the macro variables for this contact.
        grab_contact_macros_r(&mac, it_cntct->second);

        // Clear summary macros (they are customized for each contact).
        clear_summary_macros_r(&mac);

        // Notify this contact with all its commands.
        std::list<std::pair<commands::command*, std::string> > const&
          cmds(
               is_host()
               ? it_cntct->second->get_host_notification_commands()
               : it_cntct->second->get_service_notification_commands());
        for (std::list<std::pair<commands::command*, std::string> >::const_iterator
               it_cmd(cmds.begin()),
               end_cmd(cmds.end());
             it_cmd != end_cmd;
             ++it_cmd) {
          // Log attempt.
          if (config->log_notifications()) {
            // Get string of the current state.
            char const* state_string;
            if (is_host())
              state_string = host_states_strings[get_current_state()];
            else
              state_string = service_states_strings[get_current_state()];

            // Get string of the notification type.
            char const* type_string;
            type_string = notification_types_strings[type];

            // Get additional info based on notification type.
            std::string info;
            switch (type) {
             case ACKNOWLEDGEMENT:
             case CUSTOM:
              info.append(";").append(author).append(";").append(comment);
              break;
             default:
               ;
            }

            // Generate notification string.
            std::string notification_string;
            if ((type == PROBLEM) || (type == RECOVERY))
              notification_string = state_string;
            else
              notification_string
                .append(type_string)
                .append(" (")
                .append(state_string)
                .append(")");

            if (is_host()) {
              host* hst(static_cast<host*>(this));
              logger(log_service_notification, basic)
                << "HOST NOTIFICATION: " << it_cntct->second->get_name()
                << ';' << hst->get_name() << ';' << notification_string
                << ';' << it_cmd->first->get_name() << ';'
                << get_output() << info;
            }
            else {
              service* svc(static_cast<service*>(this));
              logger(log_service_notification, basic)
                << "SERVICE NOTIFICATION: "
                << it_cntct->second->get_name() << ';'
                << svc->get_host_name() << ';' << svc->get_description()
                << ';' << notification_string << ';'
                << it_cmd->first->get_name() << ';'
                << get_output() << info;
            }
          }

          // Run notification command.
          commands::result res;
          try {
            std::string processed_cmd(it_cmd->first->process_cmd(&mac));
            it_cmd->first->run(
                             processed_cmd,
                             mac,
                             0,
                             res);
          }
          catch (std::exception const& e) {
            logger(logging::log_runtime_warning, logging::basic)
              << "Error: notification failed: " << e.what();
          }
        }
        ++contacts_notified;
      }
      clear_summary_macros_r(&mac);

      if ((type == PROBLEM) || (type == RECOVERY)) {
        // Adjust last/next notification time and notification flags if
        // we notified someone.
        if (contacts_notified > 0) {
          // If notification interval is 0, no more notification should occur.
          if (!_notification_interval) {
            set_no_more_notifications(true);
            logger(logging::dbg_notifications, logging::basic)
              << (is_host() ? "Host" : "Service") << " has no "
                 "notification interval. Disabling further "
                 "notifications until recovery.";
          }
          // Calculate the next acceptable re-notification time.
          else {
            set_no_more_notifications(false);
            time_t next_notification(now + _notification_interval);
            get_next_valid_time(
              next_notification,
              &next_notification,
              get_notification_period());
            set_next_notification(next_notification);
            {
              logger(logging::dbg_notifications, logging::basic)
                << contacts_notified << " contacts were notified.  "
                   "Next possible notification time: "
                << my_ctime(&next_notification);
            }

            // Update the last notification time for this notifiable
            // (this is needed for rescheduling later notifications).
            set_last_notification(now);
          }
        }
        // We didn't end up notifying anyone.
        else if (incremented_notification_number) {
          /* adjust current notification number */
          --_current_notification_number;
          {
            time_t next_notification(get_next_notification());
            logger(logging::dbg_notifications, logging::basic)
              << "No contacts were notified.  Next possible "
                 "notification time: " << my_ctime(&next_notification);
          }
        }
        logger(logging::dbg_notifications, logging::basic)
          << contacts_notified << " contacts were notified.";
      }
      // There were no contacts, so no notification really occurred...
      else if (contacts_notified <= 0) {
        // Readjust current notification number, since one didn't go out.
        if (incremented_notification_number)
          --_current_notification_number;
        logger(logging::dbg_notifications, logging::basic)
          << "No contacts were found for notification purposes.  "
             "No notification was sent out.";
      }

      // Send data to event broker.
      {
        struct timeval start_time_tv;
        memset(&start_time_tv, 0, sizeof(start_time_tv));
        start_time_tv.tv_sec = now;
        struct timeval end_time_tv;
        gettimeofday(&end_time_tv, NULL);
        broker_notification_data(
          NEBTYPE_NOTIFICATION_END,
          NEBFLAG_NONE,
          NEBATTR_NONE,
          is_host() ? HOST_NOTIFICATION : SERVICE_NOTIFICATION,
          type,
          start_time_tv,
          end_time_tv,
          this,
          author.c_str(),
          comment.c_str(),
          false, // XXX escalated,
          contacts_notified,
          NULL);
      }

      // Update the status log with the service information.
      if (is_host())
        broker_host_status(static_cast<host*>(this));
      else
        broker_service_status(static_cast<service*>(this));

      // Clear volatile macros.
      clear_volatile_macros_r(&mac);
    }
    time(&_last_notification);
  }
}

/**************************************
*                                     *
*      Contacts / contact groups      *
*                                     *
**************************************/

/**
 *  Add a contact to this notifiable.
 *
 *  @param[in,out] user  Pointer to a contact that will receive
 *                       notifications from this object.
 */
void notifiable::add_contact(engine::contact* user) {
  if (user)
    _contacts[user->get_name()] = user;
  return ;
}

/**
 *  Add a contact group to this notifiable.
 *
 *  @param[in,out] cg  Pointer to a contact group that will receive
 *                     notifications from this object.
 */
void notifiable::add_contactgroup(engine::contactgroup* cg) {
  if (cg)
    _contact_groups[cg->get_name()] = cg;
  return ;
}

/**
 *  Clear contact list.
 */
void notifiable::clear_contacts() {
  _contacts.clear();
  return ;
}

/**
 *  Clear contact group list.
 */
void notifiable::clear_contactgroups() {
  _contact_groups.clear();
  return ;
}

/**
 *  Check if some contact is notified by this object.
 *
 *  @param[in] username  User name.
 *
 *  @return True if user is notified by this object.
 */
bool notifiable::contains_contact(std::string const& username) const {
  // Check in contact list.
  umap<std::string, contact*>::const_iterator it(
    get_contacts().find(username));
  if (it != get_contacts().end())
    return (true);

  // Check each contact group.
  for (umap<std::string, engine::contactgroup*>::const_iterator
         cgit(get_contactgroups().begin()),
         end(get_contactgroups().end());
       cgit != end;
       ++cgit) {
    if (cgit->second->has_member(username))
      return (true);
  }
  return (false);
}

/**
 *  Get contact container.
 *
 *  @return Reference to the contact container.
 */
umap<std::string, engine::contact*> const& notifiable::get_contacts() const {
  return _contacts;
}

/**
 *  Get contact group container.
 *
 *  @return Reference to the contact group container.
 */
umap<std::string, engine::contactgroup*> const& notifiable::get_contactgroups() const {
  return _contact_groups;
}

/**************************************
*                                     *
*           Acknowledgement           *
*                                     *
**************************************/

/**
 *  Get acknowledgement timeout.
 *
 *  @return Acknowledgement timeout in seconds.
 */
int notifiable::get_acknowledgement_timeout() const {
  return (_acknowledgement_timeout);
}

/**
 *  Set acknowledgement timeout.
 *
 *  @param[in] timeout  Acknowledgement timeout in seconds.
 */
void notifiable::set_acknowledgement_timeout(int timeout) {
  _acknowledgement_timeout = timeout;
  return ;
}

/**
 *  Get current acknowledgement type.
 *
 *  @return Acknowledgement type.
 */
notifiable::acknowledgement_type notifiable::get_acknowledgement_type() const {
  return (_current_acknowledgement);
}

/**
 *  Check if notifiable is acknowledged.
 *
 *  @return True if acknowledged.
 */
bool notifiable::is_acknowledged() const {
  return (_current_acknowledgement != ACKNOWLEDGEMENT_NONE);
}

/**
 *  Acknowledge / disacknowledge.
 *
 *  @param[in] type  Acknowledgement type.
 */
void notifiable::set_acknowledged(acknowledgement_type type) {
  _current_acknowledgement = type;
  return ;
}

/**
 *  Get last time notifiable was acknowledged.
 *
 *  @return Timestamp.
 */
time_t notifiable::get_last_acknowledgement() const {
  return (_last_acknowledgement);
}

/**
 *  Set last time notifiable was acknowledged.
 *
 *  @param[in] last_acknowledgement  Timestamp.
 */
void notifiable::set_last_acknowledgement(time_t last_acknowledgement) {
  _last_acknowledgement = last_acknowledgement;
  return ;
}

/**
 *  Schedule acknowledgement expiration.
 */
void notifiable::schedule_acknowledgement_expiration() {
  int ack_timeout(get_acknowledgement_timeout());
  int last_ack(get_last_acknowledgement());
  if (ack_timeout > 0 && last_ack != (time_t)0) {
    int event_type;
    if (is_host())
      event_type = EVENT_EXPIRE_HOST_ACK;
    else
      event_type = EVENT_EXPIRE_SERVICE_ACK;
    schedule_new_event(
      event_type,
      false,
      last_ack + ack_timeout,
      false,
      0,
      NULL,
      true,
      this,
      NULL,
      0);
  }
}

/**
 *  Update acknowledgement on state change.
 */
void notifiable::update_acknowledgement_on_state_change() {
  // Normal acknowledgements will be removed on any state change.
  if (((get_acknowledgement_type() == ACKNOWLEDGEMENT_NORMAL)
       && (get_current_state() != get_last_state()))
  // Sticky acknowledgements will be removed when back to normal state.
      || ((get_acknowledgement_type() == ACKNOWLEDGEMENT_STICKY)
          && (get_current_state() == 0))) {
    set_acknowledged(ACKNOWLEDGEMENT_NONE);
    delete_acknowledgement_comments();
  }
  return ;
}

/**************************************
*                                     *
*              Downtime               *
*                                     *
**************************************/

/**
 *  Get the number of pending flexible downtimes targeting this object.
 *
 *  @return An integer.
 */
int notifiable::get_pending_flex_downtime() const {
  return (_pending_flex_downtime);
}

/**
 *  Increment the number of pending flexible downtimes targeting this
 *  object.
 */
void notifiable::inc_pending_flex_downtime() {
  ++_pending_flex_downtime;
  return ;
}

/**
 *  Decrement the number of pending flexible downtimes targeting this
 *  object.
 */
void notifiable::dec_pending_flex_downtime() {
  --_pending_flex_downtime;
  return ;
}

/**
 *  Get the number of active downtimes targeting this object.
 *
 *  @return An integer.
 */
int notifiable::get_scheduled_downtime_depth() const {
  return (_scheduled_downtime_depth);
}

/**
 *  Increment the number of active downtimes targeting this object.
 */
void notifiable::inc_scheduled_downtime_depth() {
  ++_scheduled_downtime_depth;
  return ;
}

/**
 *  Decrement the number of active downtimes targeting this object.
 */
void notifiable::dec_scheduled_downtime_depth() {
  --_scheduled_downtime_depth;
  return ;
}

/**
 *  Tell if the current notification is escalated
 *
 *  @return a boolean
 */
bool notifiable::should_be_escalated() const {

  logger(dbg_functions, basic)
    << "notifiable: should_be_escalated()";

  // FIXME DBR: not implemented
  return false;
}

void notifiable::delete_all_comments() {
  for (std::map<unsigned long, comment*>::iterator
         it(comment_list.begin()),
         next_it(comment_list.begin()),
         end(comment_list.end());
       it != end;
       it = next_it) {
    ++next_it;
    if (it->second->get_parent() == this) {
      delete it->second;
      comment_list.erase(it);
    }
  }
}

/**
 *  Deletes all non-persistent acknowledgement comments for a particular
 *  notifiable
 */
void notifiable::delete_acknowledgement_comments() {
  /* delete comments from memory */
  for (std::map<unsigned long, comment*>::iterator
         it(comment_list.begin()),
         next_it(comment_list.begin()),
         end(comment_list.end());
       it != end;
       it = next_it) {
    ++next_it;
    comment* temp_comment(it->second);
    if (temp_comment->get_comment_type() == comment::SERVICE_COMMENT
        && temp_comment->get_parent() == this
        && temp_comment->get_entry_type() == comment::ACKNOWLEDGEMENT_COMMENT
        && !temp_comment->get_persistent()) {
      delete temp_comment;
      comment_list.erase(it);
    }
  }
}

/**************************************
*                                     *
*           Static Objects            *
*                                     *
**************************************/

std::string const notifiable::_notification_string[] = {
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
 *  Checks for flexible (non-fixed) notifiable downtime that should start now.
 */
void notifiable::check_pending_flex_downtime() {
  logger(dbg_functions, basic)
    << "check_pending_flex_downtime()";

  // If notifiable is currently ok, nothing to do.
  if (get_current_state() == STATE_OK)
    return ;

  // Check all downtime entries.
  for (umap<unsigned long, downtime>::const_iterator
         it(downtime_manager::instance().get_downtimes().begin()),
         end(downtime_manager::instance().get_downtimes().end());
       it != end;
       ++it) {
    downtime const& temp_downtime(it->second);
    if (temp_downtime.get_fixed()
        || temp_downtime.get_in_effect()
        || temp_downtime.get_triggered_by() != 0)
      continue ;

    // This entry matches our notifiable and the time boundaries
    // are ok, start this scheduled downtime.
    time_t current_time(0L);
    time(&current_time);
    if ((temp_downtime.get_parent() == this)
        && (temp_downtime.get_start_time() <= current_time)
        && (current_time <= temp_downtime.get_end_time())) {
      logger(dbg_downtime, basic)
        << "Flexible downtime (id=" << temp_downtime.get_id()
        << ") for notifiable '" << get_info() << "' starting now...";
      downtime_manager::instance().start(temp_downtime.get_id());
    }
  }
}

/**
 *  Copy internal data members.
 *
 *  @param[in] other  Object to copy.
 */
void notifiable::_internal_copy(notifiable const& other) {
  _acknowledgement_timeout = other._acknowledgement_timeout;
  _contacts = other._contacts;
  _contact_groups = other._contact_groups;
  _current_acknowledgement = other._current_acknowledgement;
  _current_notification_id = other._current_notification_id;
  _current_notification_number = other._current_notification_number;
  _escalate_notification = other._escalate_notification;
  _first_notification = other._first_notification;
  _first_notification_delay = other._first_notification_delay;
  _last_acknowledgement = other._last_acknowledgement;
  _last_notification = other._last_notification;
  _next_notification = other._next_notification;
  _notification_interval = other._notification_interval;
  _notification_period = other._notification_period;
  _notifications_enabled = other._notifications_enabled;
  _notified_states = other._notified_states;
  _pending_flex_downtime = other._pending_flex_downtime;
  _recovery_notification_delay = other._recovery_notification_delay;
  _scheduled_downtime_depth = other._scheduled_downtime_depth;
  return ;
}

/**
 *  Check if notification is viable.
 *
 *  @param[in] type     Notification type.
 *  @param[in] options  Notification options.
 *
 *  @return True if notification is viable.
 */
bool notifiable::_is_notification_viable(int type, int options) {
  // Forced notification.
  if (options & NOTIFICATION_OPTION_FORCED) {
    logger(logging::dbg_notifications, logging::more)
      << "This is a forced service notification, so we'll send it out.";
    return (true);
  }

  // Are notifications enabled?
  if (!config->enable_notifications()) {
    logger(logging::dbg_notifications, logging::more)
      << "Notifications are disabled, so service notifications will "
         "not be sent out.";
    return (false);
  }

  // See if the service can have notifications sent out at this time.
  time_t now(time(NULL));
  {
    timezone_locker lock(get_timezone().c_str());
    if (check_time_against_period(now, get_notification_period()) == ERROR) {
      logger(logging::dbg_notifications, logging::more)
        << "This service shouldn't have notifications sent out "
        "at this time.";

      // Calculate the next acceptable notification time,
      // once the next valid time range arrives...
      if ((type == PROBLEM) || (type == RECOVERY)) {
        time_t next_valid_time(now);
        get_next_valid_time(
          now,
          &next_valid_time,
          get_notification_period());

        // Looks like there are no valid notification times defined, so
        // schedule the next one far into the future (one year)...
        if ((next_valid_time == (time_t)0) || (next_valid_time == (time_t)-1))
          set_next_notification(now + 60 * 60 * 24 * 365);
        // Else use the next valid notification time.
        else
          set_next_notification(next_valid_time);
        {
          time_t next_notification(get_next_notification());
          logger(logging::dbg_notifications, logging::more)
            << "Next possible notification time: "
            << my_ctime(&next_notification);
        }
      }

      return (false);
    }
  }

  // Are notifications temporarily disabled for this object?
  if (!get_notifications_enabled()) {
    logger(logging::dbg_notifications, logging::more)
      << "Notifications are temporarily disabled for this "
      << (is_host() ? "host" : "service")
      << ", so we won't send one out.";
    return (false);
  }

  //
  // SPECIAL CASE FOR CUSTOM NOTIFICATIONS
  //

  // Custom notifications are good to go at this point...
  if (type == CUSTOM) {
    if (get_scheduled_downtime_depth() > 0) {
      logger(logging::dbg_notifications, logging::more)
        << "We shouldn't send custom notification during "
           "scheduled downtime.";
      return (false);
    }
    return (true);
  }

  //
  // SPECIAL CASE FOR ACKNOWLEGEMENTS
  //

  // Acknowledgements only have to pass three general filters,
  // although they have another test of their own...
  if (type == ACKNOWLEDGEMENT) {
    // Don't send an acknowledgement if there isn't a problem...
    if (get_current_state() == STATE_OK) {
      logger(logging::dbg_notifications, logging::more)
        << "The service is currently OK, so we won't send an "
           "acknowledgement.";
      return (false);
    }
    // Acknowledgement viability test passed,
    // so the notification can be sent out.
    return (true);
  }

  //
  // SPECIAL CASE FOR FLAPPING ALERTS
  //

  // Flapping notifications only have to pass three general filters.
  if (type == FLAPPINGSTART
      || type == FLAPPINGSTOP
      || type == FLAPPINGDISABLED) {
    // Don't send a notification if we're not supposed to...
    if (!get_notify_on(ON_FLAPPING)) {
      logger(logging::dbg_notifications, logging::more)
        << "We shouldn't notify about FLAPPING events for this "
           "service.";
      return (false);
    }
    // Don't send notifications during scheduled downtime.
    if (get_scheduled_downtime_depth() > 0) {
      logger(logging::dbg_notifications, logging::more)
        << "We shouldn't notify about FLAPPING events during "
           "scheduled downtime.";
      return (false);
    }
    // Flapping viability test passed,
    // so the notification can be sent out.
    return (true);
  }

  //
  // SPECIAL CASE FOR DOWNTIME ALERTS
  //

  // Downtime notifications only have to pass three general filters.
  if (type == DOWNTIMESTART
      || type == DOWNTIMESTOP
      || type == DOWNTIMECANCELLED) {
    // Don't send a notification if we're not supposed to....
    if (!get_notify_on(ON_DOWNTIME)) {
      logger(logging::dbg_notifications, logging::more)
        << "We shouldn't notify about DOWNTIME events for "
           "this service.";
      return (false);
    }
    // Don't send notifications during scheduled downtime.
    if (get_scheduled_downtime_depth() > 0) {
      logger(logging::dbg_notifications, logging::more)
        << "We shouldn't notify about DOWNTIME events during "
           "scheduled downtime.";
      return (false);
    }
    // Downtime viability test passed,
    // so the notification can be sent out.
    return (true);
  }

  //
  // NORMAL NOTIFICATIONS
  //

  // Is this a hard problem/recovery?
  if (get_current_state_type() == SOFT_STATE) {
    logger(logging::dbg_notifications, logging::more)
      << "This service is in a soft state, so we won't send a "
         "notification out.";
    return (false);
  }
  // Has this problem already been acknowledged?
  if (is_acknowledged()) {
    logger(logging::dbg_notifications, logging::more)
      << "This service problem has already been acknowledged, "
         "so we won't send a notification out.";
    return (false);
  }
  /* XXX check service notification dependencies */
  // if (check_service_dependencies(
  //       svc,
  //       NOTIFICATION_DEPENDENCY) == DEPENDENCIES_FAILED) {
  //   logger(dbg_notifications, more)
  //     << "Service notification dependencies for this service "
  //     "have failed, so we won't sent a notification out.";
  //   return (ERROR);
  // }
  // /* check host notification dependencies */
  // if (check_host_dependencies(
  //       temp_host,
  //       NOTIFICATION_DEPENDENCY) == DEPENDENCIES_FAILED) {
  //   logger(dbg_notifications, more)
  //     << "Host notification dependencies for this service have failed, "
  //     "so we won't sent a notification out.";
  //   return (ERROR);
  // }

  // See if we should notify about problems with this host.
  if (is_host()) {
    host* hst(static_cast<host*>(this));
    switch (get_current_state()) {
     case HOST_DOWN:
      if (!hst->get_notify_on(ON_DOWN)) {
        logger(logging::dbg_notifications, logging::more)
          << "We shouldn't notify about DOWN states for this host.";
        return (false);
      }
      break ;
     case HOST_UNREACHABLE:
      if (!hst->get_notify_on(ON_UNREACHABLE)) {
        logger(logging::dbg_notifications, logging::more)
          << "We shouldn't notify about UNREACHABLE states for this host.";
        return (false);
      }
      break ;
     case STATE_UNKNOWN:
      if (!hst->get_notify_on(ON_UNKNOWN)) {
        logger(logging::dbg_notifications, logging::more)
          << "We shouldn't notify about UNKNOWN states for this host.";
        return (false);
      }
      break ;
     case HOST_UP:
      if (!hst->get_notify_on(ON_RECOVERY)) {
        logger(logging::dbg_notifications, logging::more)
          << "We shouldn't notify about UP states for this host.";
        return (false);
      }
      break ;
    }
  }
  else {
    service* svc(static_cast<service*>(this));
    switch (get_current_state()) {
     case STATE_UNKNOWN:
      if (!svc->get_notify_on(ON_UNKNOWN)) {
        logger(logging::dbg_notifications, logging::more)
          << "We shouldn't notify about UNKNOWN states for this service.";
        return (false);
      }
      break ;
     case STATE_WARNING:
      if (!svc->get_notify_on(ON_WARNING)) {
        logger(logging::dbg_notifications, logging::more)
          << "We shouldn't notify about WARNING states for this service.";
        return (false);
      }
      break ;
     case STATE_CRITICAL:
      if (!svc->get_notify_on(ON_CRITICAL)) {
        logger(logging::dbg_notifications, logging::more)
          << "We shouldn't notify about CRITICAL states for this service.";
        return (false);
      }
      break ;
     case STATE_OK:
      if (!svc->get_notify_on(ON_RECOVERY)) {
        logger(logging::dbg_notifications, logging::more)
          << "We shouldn't notify about RECOVERY states for this service.";
        return (false);
      }
      // XXX
      // if (!(_notification_is_activec->get_notified_on(ON_UNKNOWN)
      //       || svc->get_notified_on(ON_WARNING)
      //       || svc->get_notified_on(ON_CRITICAL))) {
      //   logger(logging::dbg_notifications, logging::more)
      //     << "We shouldn't notify about this recovery.";
      //   return (false);
      // }
    }
  }

  // See if enough time has elapsed for first notification.
  if ((type == PROBLEM)
      && (get_current_notification_number() == 0)
      && (get_last_hard_state_change() + get_first_notification_delay() > now)) {
    logger(logging::dbg_notifications, logging::more)
      << "First notification delay (" << get_first_notification_delay()
      << "s) is not yet elapsed for this service.";
    return (false);
  }

  // See if enough time has elapsed for recovery notification.
  if ((type == RECOVERY)
      && (get_first_notification() + get_recovery_notification_delay() > now)) {
    logger(logging::dbg_notifications, logging::more)
      << "Recovery notification delay (" << get_recovery_notification_delay()
      << "s) is not elapsed for this service.";
    return (false);
  }

  // if (((type == PROBLEM) || (type == RECOVERY))
  //         || (get_current_state() == STATE_OK
  //               && !service_other_props[std::make_pair(
  //                   svc->host_ptr->name,
  //                   svc->description)].recovery_been_sent))) {

  //   /* get the time at which a notification should have been sent */
  //   time_t& initial_notif_time(
  //             service_other_props[std::make_pair(
  //                                        svc->host_ptr->name,
  //                                        svc->description)].initial_notif_time);

  //   /* if not set, set it to now */
  //   if (!initial_notif_time)
  //     initial_notif_time = time(NULL);

  //   double notification_delay = (svc->current_state != STATE_OK ?
  //            svc->first_notification_delay
  //            : service_other_props[std::make_pair(
  //                svc->host_ptr->name,
  //                svc->description)].recovery_notification_delay)

  //   if (current_time
  //       < (time_t)(initial_notif_time
  //                  + (time_t)(notification_delay))) {
  //     if (svc->current_state == STATE_OK)
  //       logger(dbg_notifications, more)
  //         << "Not enough time has elapsed since the service changed to a "
  //         "OK state, so we should not notify about this problem yet";
  //     else
  //       logger(dbg_notifications, more)
  //         << "Not enough time has elapsed since the service changed to a "
  //         "non-OK state, so we should not notify about this problem yet";
  //     return (ERROR);
  //   }
  // }

  // If this service is currently flapping, don't send the notification.
  if (get_flapping()) {
    logger(logging::dbg_notifications, logging::more)
      << "This node is currently flapping, so we won't send "
         "notifications.";
    return (false);
  }

  // If this service is currently in a scheduled downtime period,
  // don't send the notification.
  if (get_scheduled_downtime_depth() > 0) {
    logger(logging::dbg_notifications, logging::more)
      << "This node is currently in a scheduled downtime, so "
         "we won't send notifications.";
    return (false);
  }

  // // If this host is currently in a scheduled downtime period, don't send the notification
  // if (temp_host->scheduled_downtime_depth > 0) {
  //   logger(dbg_notifications, more)
  //     << "The host this service is associated with is currently in "
  //     "a scheduled downtime, so we won't send notifications.";
  //   return (ERROR);
  // }

  // RECOVERY NOTIFICATIONS ARE GOOD TO GO AT THIS POINT IF
  // ANY OTHER NOTIFICATION WAS SENT.
  if (get_current_state() == STATE_OK) {
    if (get_current_notification_number() > 0) {
      return (true);
    }
    else {
      logger(logging::dbg_notifications, logging::more)
        << "We should not sent a recovery notification as there was"
        << " no problem notification sent initially.";
      return (false);
    }
  }

  // Don't notify contacts about this service problem again if the
  // notification interval is set to 0.
  if (get_no_more_notifications()) {
    logger(logging::dbg_notifications, logging::more)
      << "We shouldn't re-notify contacts about this "
      << (is_host() ? "host" : "service") << " problem.";
    return (false);
  }

  /* if the host is down or unreachable, don't notify contacts about service failures */
  // XXX
  // if (temp_host->current_state != HOST_UP) {
  //   logger(dbg_notifications, more)
  //     << "The host is either down or unreachable, so we won't "
  //     "notify contacts about this service.";
  //   return (ERROR);
  // }

  // Don't notify if we haven't waited long enough since the last time
  // (and the service is not marked as being volatile).
  if ((now < get_next_notification())
      && (is_host() || !static_cast<service*>(this)->get_volatile())) {
    logger(logging::dbg_notifications, logging::more)
      << "We haven't waited long enough to re-notify contacts "
         "about this " << (is_host() ? "host" : "service") << ".";
    {
      time_t next_notification(get_next_notification());
      logger(logging::dbg_notifications, logging::more)
        << "Next valid notification time: "
        << my_ctime(&next_notification);
    }
    return (false);
  }

  return (true);
}

/**
 *  Stores the id of the flapping comment associated to the notifiable.
 *
 *  @param id The flapping comment id.
 */
void notifiable::set_flapping_comment_id(unsigned int id) {
  _flapping_comment_id = id;
}

/**
 *  Gets the id of the flapping comment associated to the notifiable or 0 if
 *  there is no comment.
 *
 *  @return an unsigned integer.
 */
unsigned int notifiable::get_flapping_comment_id() const {
  return _flapping_comment_id;
}
