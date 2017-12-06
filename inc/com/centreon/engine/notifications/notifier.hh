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

#ifndef CCE_NOTIFICATIONS_NOTIFIER_HH
#  define CCE_NOTIFICATIONS_NOTIFIER_HH

#  include <ctime>
#  include <list>
#  include "com/centreon/engine/checks/checkable.hh"
#  include "com/centreon/engine/globals.hh"
#  include "com/centreon/engine/macros/defines.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"

CCE_BEGIN()

// Forward declaration
class contact;
class downtime;

namespace           notifications {

  /**
   *  @class notifier notifier.hh "com/centreon/engine/notifications/notifier.hh"
   *  @brief Object validating notifications and sending them if needed.
   *
   */
  class               notifier : public checks::checkable {
   public:
    typedef bool (notifier::* notifier_filter)();
    typedef void (notifier::* macro_builder)(struct nagios_macros&);

    enum              notifier_type {
      HOST_NOTIFICATION,
      SERVICE_NOTIFICATION
    };

    enum              notification_type {
      NONE                 = 0,
      PROBLEM              = 1,
      RECOVERY             = 2,
      ACKNOWLEDGEMENT      = 3,
      FLAPPINGSTART        = 4,
      FLAPPINGSTOP         = 5,
      FLAPPINGDISABLED     = 6,
      DOWNTIMESTART        = 7,
      DOWNTIMESTOP         = 8,
      DOWNTIMECANCELLED    = 9,
      CUSTOM               = 10,
    };
    enum              action_on {
      ON_NONE = 0,
      ON_RECOVERY = (1 << 0),
      ON_UP = (1 << 1),
      ON_DOWN = (1 << 2),
      ON_UNREACHABLE = (1 << 3),
      ON_FLAPPING = (1 << 4),
      ON_DOWNTIME = (1 << 5),
      ON_OK = (1 << 6),
      ON_WARNING = (1 << 7),
      ON_UNKNOWN = (1 << 8),
      ON_CRITICAL = (1 << 9),
    };

    enum              acknowledgement_type {
      ACKNOWLEDGEMENT_NONE,
      ACKNOWLEDGEMENT_NORMAL,
      ACKNOWLEDGEMENT_STICKY
    };
                      notifier();
                      notifier(notifier const& other);
    virtual           ~notifier();
    notifier&         operator=(notifier const& other);
    bool              contains_contact(engine::contact* user) const;
    bool              contains_contact(std::string const& username) const;
    void              add_contact(shared_ptr<engine::contact> user);
    void              add_contactgroup(shared_ptr<engine::contactgroup> cg);
    void              set_notifications_enabled(bool enabled);
    bool              get_notifications_enabled() const;
    int               get_acknowledgement_type() const;
    void              clear_contacts();
    void              clear_contactgroups();
    void              enable_state_notification(int state);
    std::list<shared_ptr<engine::contact> >
                      get_contacts_list();
    umap<std::string, shared_ptr<engine::contact> > const&
                      get_contacts() const;
    umap<std::string, shared_ptr<engine::contact> >&
                      get_contacts();
    umap<std::string, shared_ptr<engine::contactgroup> > const&
                      get_contactgroups() const;
    umap<std::string, shared_ptr<engine::contactgroup> >&
                      get_contactgroups();

    int               get_current_notification_id() const;
    int               get_current_notification_number() const;
    notification_type get_current_notification_type() const;
    time_t            get_last_notification() const;
    time_t            get_next_notification() const;
    bool              get_no_more_notifications() const;
    int               get_pending_flex_downtime() const;
    bool              check_pending_flex_downtime();
    int               get_scheduled_downtime_depth() const;
    bool              is_in_downtime() const;
    bool              is_state_notification_enabled(int state) const;
    void              notify(
                        notification_type type,
                        std::string const& author = "",
                        std::string const& comment = "");

    void              set_last_notification(time_t last_notification);
    void              set_next_notification(time_t next_notification);
    bool              should_be_escalated() const;
    bool              is_acknowledged() const;
    void              set_acknowledged(bool acked);
    void              set_acknowledgement_type(acknowledgement_type type);
    void              set_last_acknowledgement(time_t last_acknowledgement);
    bool              get_recovery_been_sent() const;
    bool              set_notified_on_down(bool value);
    bool              set_notified_on_unreachable(bool value);
    void              set_current_notification_number(int number);
    void              set_initial_notif_time(time_t initial);
//    void              set_no_more_notifications(bool no_more);
    void              set_recovery_been_sent(bool sent);
    timeperiod*       get_notification_period() const;
    void              set_notification_period(timeperiod* tperiod);

   protected:
    virtual void      _checkable_macro_builder(nagios_macros& mac) = 0;

    time_t            _last_notification;
    time_t            _next_notification;
    long              _notification_interval;
    int               _current_notification_id;
    int               _current_notification_number;
    notification_type _type;
    bool              _in_downtime;

   private:
    notifier_filter   _get_filter(notification_type type) const;
    macro_builder     _get_macro_builder(notification_type type) const;
    bool              _problem_filter();
    bool              _recovery_filter();

    void              _problem_macro_builder(nagios_macros& mac);
    void              _recovery_macro_builder(nagios_macros& mac);
    long              _get_notification_interval() const;

    time_t            _last_acknowledgement;

    static notifier_filter
                      _filter[];
    static macro_builder
                      _macro_builder[];
    static std::string
                      _notification_string[];
    bool              _escalate_notification;

    int               _notified_states;
    umap<std::string, shared_ptr<engine::contact> >
                      _contacts;
    umap<std::string, shared_ptr<engine::contactgroup> >
                      _contact_groups;
    bool              _notifications_enabled;
  };
}

CCE_END()

#endif // !CCE_NOTIFICATIONS_NOTIFIER_HH
