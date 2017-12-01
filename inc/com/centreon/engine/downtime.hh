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

#ifndef CCE_DOWNTIME_HH
#  define CCE_DOWNTIME_HH

#  include <ctime>
#  include <string>
#  include "com/centreon/engine/notifications/notifier.hh"

CCE_BEGIN()

/**
 *  @class downtime downtime.hh "com/centreon/engine/downtime.hh"
 *  @brief Service as a host's downtime.
 *
 *  This class represents a downtime.
 */
class                downtime {
 public:
//  enum               downtime_type {
//    SERVICE_DOWNTIME,
//    HOST_DOWNTIME
//  };
                     downtime(
                       notifications::notifier* parent,
                       time_t entry_time,
                       std::string const& author,
                       std::string const& comment_data,
                       time_t start_time,
                       time_t end_time,
                       int fixed,
                       unsigned long triggered_by,
                       unsigned long duration,
                       unsigned long downtime_id);
                     downtime(downtime const& other);
                     ~downtime();
  downtime&          operator=(downtime const& other);

  std::string const& get_host_name() const;
  //downtime_type      get_type() const;

 private:
  void               _internal_copy(downtime const& other);

  notifications::notifier*
                     _parent;

  time_t             _entry_time;
  std::string        _author;
  std::string        _comment_data;
  time_t             _start_time;
  time_t             _end_time;
  int                _fixed;
  unsigned long      _triggered_by;
  unsigned long      _duration;
  unsigned long      _downtime_id;
};

CCE_END()

#endif // !CCE_DOWNTIME_HH
