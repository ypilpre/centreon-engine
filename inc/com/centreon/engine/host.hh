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

CCE_BEGIN()

/**
 *  @class host host.hh "com/centreon/engine/host.hh"
 *  @brief This class represents a host.
 *
 *  A host is checkable and also a notifier.
 */
class                     host : public monitorable {
 public:
                          host();
                          host(host const& other);
                          ~host();
  host&                   operator=(host const& other);
  std::list<host*> const& get_children() const;
  time_t                  get_last_time_down() const;
  time_t                  get_last_time_unreachable() const;
  time_t                  get_last_time_up() const;
  std::list<host*> const& get_parents() const;
  bool                    get_stalk_on_down() const;
  bool                    get_stalk_on_unreachable() const;
  bool                    get_stalk_on_up() const;
  bool                    get_should_reschedule_current_check() const;
  bool                    is_notified_on_down() const;
  bool                    is_notified_on_unreachable() const;
  void                    set_last_time_down(time_t last_time);
  void                    set_last_time_unreachable(time_t last_time);
  void                    set_last_time_up(time_t last_time);
  void                    set_notified_on_down(bool notify);
  void                    set_notified_on_unreachable(bool notify);
  void                    set_should_reschedule_current_check(
                            bool reschedule);
};

CCE_END()

using com::centreon::engine::host;

typedef std::list<host*> host_set;

#endif // !CCE_HOST_HH
