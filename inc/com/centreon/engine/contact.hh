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

#ifndef CCE_CONTACT_HH
#  define CCE_CONTACT_HH

#  include <string>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/unordered_hash.hh"

#  define MAX_CONTACT_ADDRESSES 6


CCE_BEGIN()

// Forward declaration

namespace configuration {
  class timeperiod;
}

namespace commands {
  class command;
}

  /**
   *  @class contact contact.hh "com/centreon/engine/contact.hh
   *  @brief Object representing a contact user
   *
   */
class                          contact {
 public:
                               contact();
                               contact(contact const& other);
  virtual                      ~contact();
  contact&                     operator=(contact const& other);

  std::string                  get_name() const;
  bool                         check(int* w, int* e);
  umap<std::string, commands::command*>& get_host_notification_commands() const;
  umap<std::string, commands::command*>& get_service_notification_commands() const;
  configuration::timeperiod*   get_host_notification_period() const;
  configuration::timeperiod*   get_service_notification_period() const;
  void                         set_host_notification_period(
                                 configuration::timeperiod* tp);
  void                         set_service_notification_period(
                                 configuration::timeperiod* tp);

 private:
  std::string                  _name;
  umap<std::string, commands::command*> _host_notification_commands;
  umap<std::string, commands::command*> _service_notification_commands;
  configuration::timeperiod*   _host_timeperiod;
  configuration::timeperiod*   _service_timeperiod;
};

CCE_END()

#endif // !CCE_CONTACT_HH
