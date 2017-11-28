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

#ifndef CCE_HOST_HH
#  define CCE_HOST_HH

#  include <list>
#  include "com/centreon/engine/monitorable.hh"
#  include "com/centreon/engine/namespace.hh"

// Forward declaration.
struct hostgroup_struct;

CCE_BEGIN()

// Forward declarations.
namespace configuration { class host; }
class service;

/**
 *  @class host host.hh "com/centreon/engine/host.hh"
 *  @brief This class represents a host.
 *
 *  A host is checkable and also a notifier.
 */
class                        host : public monitorable {
 public:
                             host(configuration::host const& cfg);
                             host(host const& other);
                             ~host();
  host&                      operator=(host const& other);

  // Configuration.
  int                        get_circular_path_checked() const;
  void                       set_circular_path_checked(int check_level);
  int                        get_initial_state() const;
  bool                       get_stalk_on_down() const;
  bool                       get_stalk_on_unreachable() const;
  bool                       get_stalk_on_up() const;

  // Links with other objects.
  std::list<host*> const&    get_children() const;
  void                       add_hostgroup(hostgroup_struct* hg);
  std::list<host*> const&    get_parents() const;
  void                       add_service(service* svc);
  void                       clear_services();
  std::list<service*> const& get_services() const;

  // State runtime.
  time_t                     get_last_time_down() const;
  void                       set_last_time_down(time_t last_time);
  time_t                     get_last_time_unreachable() const;
  void                       set_last_time_unreachable(time_t last_time);
  time_t                     get_last_time_up() const;
  void                       set_last_time_up(time_t last_time);
  bool                       get_should_reschedule_current_check() const;
  void                       set_should_reschedule_current_check(
                               bool reschedule);

  // Flap detection.
  bool                       get_flap_detection_on_up() const;
  bool                       get_flap_detection_on_down() const;
  bool                       get_flap_detection_on_unreachable() const;
  time_t                     get_last_historical_state_update() const;
  void                       set_last_historical_state_update(
                               time_t last_update);

  // Notification.
  bool                       get_notify_on_down() const;
  void                       set_notify_on_down(bool notify);
  bool                       get_notify_on_unreachable() const;
  void                       set_notify_on_unreachable(bool notify);

 private:
  void                       _internal_copy(host const& other);

  std::list<host*>           _children;
  int                        _circular_path_checked;
  bool                       _flap_detection_on_down;
  bool                       _flap_detection_on_unreachable;
  bool                       _flap_detection_on_up;
  int                        _initial_state;
  time_t                     _last_historical_state_update;
  time_t                     _last_time_down;
  time_t                     _last_time_unreachable;
  time_t                     _last_time_up;
  bool                       _notify_on_down;
  bool                       _notify_on_unreachable;
  std::list<host*>           _parents;
  bool                       _should_reschedule_current_check;
  bool                       _stalk_on_down;
  bool                       _stalk_on_unreachable;
  bool                       _stalk_on_up;
};

CCE_END()

using com::centreon::engine::host;

typedef std::list<host*> host_set;

#endif // !CCE_HOST_HH
