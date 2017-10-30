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

#ifndef CCE_SERVICE_HH
#  define CCE_SERVICE_HH

#  include <list>
#  include <string>
#  include "com/centreon/engine/monitorable.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

// Forward declaration.
class host;

/**
 *  @class service service.hh "com/centreon/engine/service.hh"
 *  @brief Service as a host's service.
 *
 *  This class represents a service. It is checkable and also a notifier.
 */
class                service : public monitorable {
 public:
                     service();
                     service(service const& other);
                     ~service();
  service&           operator=(service const& other);
  std::string const& get_description() const;
  host*              get_host() const;
  time_t             get_last_time_critical() const;
  time_t             get_last_time_ok() const;
  time_t             get_last_time_unknown() const;
  time_t             get_last_time_warning() const;
  bool               get_stalk_on_critical() const;
  bool               get_stalk_on_ok() const;
  bool               get_stalk_on_unknown() const;
  bool               get_stalk_on_warning() const;
  bool               is_volatile() const;
  void               set_last_time_critical(time_t last_critical);
  void               set_last_time_ok(time_t last_ok);
  void               set_last_time_unknown(time_t last_unknown);
  void               set_last_time_warning(time_t last_warning);
  void               set_notified_on_critical(bool notified);
  void               set_notified_on_unknown(bool notified);
  void               set_notified_on_warning(bool notified);
};

CCE_END()

using com::centreon::engine::service;

typedef std::list<service*> service_set;

#endif // !CCE_SERVICE_HH
