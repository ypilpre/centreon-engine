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
#  include "com/centreon/engine/checks/checkable.hh"

CCE_BEGIN()

namespace           notifications {

  /**
   *  @class notifier notifier.hh "com/centreon/engine/notifications/notifier.hh"
   *  @brief Object validating notifications and sending them if needed.
   *
   */
  class               notifier : public checks::checkable {
   public:
    typedef bool (notifier::* notifier_filter)();

    enum              notification_type {
                      NONE,
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
                      ~notifier();
    notifier&         operator=(notifier const& other);
    bool              enabled() const;
    bool              state_notification_enabled(int state) const;
    void              notify(notification_type type);
    void              enable_state_notification(int state);

   private:
    static notifier_filter
                      _filter[];

    notifier_filter   _get_filter(notification_type type) const;
    bool              _problem_filter();
    bool              _recovery_filter();
    int               _get_notification_number() const;
    long              _get_notification_interval() const;
    long              _get_last_notification_date() const;

    int               _type;
    int               _notification_number;
    long              _notification_interval;
    int               _notified_states;

   protected:
    time_t            _last_notification_date;
  };
}

CCE_END()

#endif // !CCE_NOTIFICATIONS_NOTIFIER_HH
