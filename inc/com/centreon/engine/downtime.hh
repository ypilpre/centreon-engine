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
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

// Forward declarations
namespace notifications {
  class notifier;
}

/**
 *  @class downtime downtime.hh "com/centreon/engine/downtime.hh"
 *  @brief Service as a host's downtime.
 *
 *  This class represents a downtime.
 */
class                downtime {
 public:
  enum               downtime_type {
    SERVICE_DOWNTIME = 1,
    HOST_DOWNTIME    = 2,
    ANY_DOWNTIME     = 3
  };
  static int         schedule_downtime(
                       downtime_type type,
                       std::string const& host_name,
                       std::string const& service_description,
                       time_t entry_time,
                       std::string const& author,
                       std::string const& comment_data,
                       time_t start_time,
                       time_t end_time,
                       int fixed,
                       unsigned long triggered_by,
                       unsigned long duration);
  int                  unschedule();

  static void        add_new_host_downtime(
                       std::string const& host_name,
                       time_t entry_time,
                       std::string const& author,
                       std::string const& comment,
                       time_t start_time,
                       time_t end_time,
                       int fixed,
                       unsigned long triggered_by,
                       unsigned long duration);

  static void        add_new_service_downtime(
                       std::string const& host_name,
                       std::string const& description,
                       time_t entry_time,
                       std::string const& author,
                       std::string const& comment,
                       time_t start_time,
                       time_t end_time,
                       int fixed,
                       unsigned long triggered_by,
                       unsigned long duration);

                     downtime(
                       downtime_type type,
                       std::string const& hostname,
                       std::string const& service_description,
                       time_t entry_time,
                       std::string const& author,
                       std::string const& comment_data,
                       time_t start_time,
                       time_t end_time,
                       int fixed,
                       unsigned long triggered_by,
                       unsigned long duration);
                     downtime(downtime const& other);
                     ~downtime();
  downtime&          operator=(downtime const& other);

  unsigned long      get_comment_id() const;
  void               set_comment_id(unsigned long id);
  std::string const& get_host_name() const;
  downtime_type      get_type() const;
  unsigned long      get_id() const;
  void               set_id(unsigned long id);
  int                get_incremented_pending_downtime() const;
  void               inc_incremented_pending_downtime();
  void               dec_incremented_pending_downtime();
  bool               is_in_effect() const;
  std::string const& get_service_description() const;
  time_t             get_entry_time() const;
  time_t             get_start_time() const;
  time_t             get_end_time() const;
  int                get_fixed() const;
  unsigned long      get_triggered_by() const;
  unsigned long      get_duration() const;
  unsigned long      get_downtime_id() const;
  std::string const& get_author() const;
  std::string const& get_comment() const;

 private:
  void               _internal_copy(downtime const& other);
  int                _register();

  time_t             _entry_time;
  std::string        _author;
  std::string        _comment_data;
  unsigned long      _comment_id;
  std::string        _host_name;
  int                _incremented_pending_downtime;
  bool               _in_effect;
  time_t             _start_time;
  time_t             _end_time;
  bool               _fixed;
  unsigned long      _triggered_by;
  unsigned long      _duration;
  unsigned long      _downtime_id;
  downtime_type      _type;
  std::string        _service_description;
};

CCE_END()

#endif // !CCE_DOWNTIME_HH
