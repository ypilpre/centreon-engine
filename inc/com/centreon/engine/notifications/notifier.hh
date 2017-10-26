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

#  include "com/centreon/engine/checks/checkable.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace           notifications {

  /**
   *  @class notifier notifier.hh "com/centreon/engine/notifications/notifier.hh"
   *  @brief Object validating notifications and sending them if needed.
   *
   */
  class             notifier : public com::centreon::engine::checks::checkable {
   public:
    enum            notification_type {
                    PROBLEM,
                    RECOVERY,
                    ACKNOWLEDGEMENT,
                    FLAPPINGSTART,
                    FLAPPINGSTOP,
                    FLAPPINGDISABLED,
                    DOWNTIMESTART,
                    DOWNTIMESTOP,
                    DOWNTIMECANCELLED
    };

                    notifier();
                    notifier(notifier const& other);
    virtual         ~notifier();
    notifier&       operator=(notifier const& other);
    void            get_acknowledged() const;
    int             get_acknowledgement_type() const;
    int             get_pending_flex_downtime() const;
    bool            get_recovery_been_sent() const;
    bool            in_downtime() const;
    bool            notifications_enabled();
    void            notify(notification_type type);
    void            set_acknowledged(bool acked);
    void            set_acknowledgement_type(int type);
    void            set_current_notification_number(int number);
    void            set_initial_notif_time(time_t initial);
    void            set_last_notification(time_t last_notification);
    void            set_next_notification(time_t next_notification);
    void            set_no_more_notifications(bool no_more);
    void            set_recovery_been_sent(bool sent);
  };
}

CCE_END()

#endif // !CCE_NOTIFICATIONS_NOTIFIER_HH
