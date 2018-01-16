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

#  include <list>
#  include <vector>
#  include "com/centreon/engine/customvar.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/objects/timeperiod.hh"
#  include "com/centreon/shared_ptr.hh"
#  include "com/centreon/unordered_hash.hh"



CCE_BEGIN()

// Forward declaration
namespace commands {
  class command;
}

namespace configuration {
  class contact;
}
class contactgroup;

  /**
   *  @class contact contact.hh "com/centreon/engine/contact.hh
   *  @brief Object representing a contact user
   *
   */
class                           contact {
 public:
                                contact(configuration::contact const& obj);
                                contact();
                                contact(contact const& other);
  virtual                       ~contact();
  contact&                      operator=(contact const& other);
  bool                          operator<(contact const& other);

  // Base properties.
  std::string const&            get_name() const;
  std::string const&            get_address(int index) const;
  std::string const&            get_alias() const;
  void                          set_alias(std::string const& alias);
  bool                          get_can_submit_commands() const;
  void                          set_can_submit_commands(bool can_submit);
  std::string const&            get_email() const;
  void                          set_email(std::string const& email);
  std::string const&            get_pager() const;
  void                          set_pager(std::string const& pager);
  bool                          get_retain_status_information() const;
  void                          set_retain_status_information(bool retain);
  bool                          get_retain_nonstatus_information() const;
  void                          set_retain_nonstatus_information(bool retain);
  std::string const&            get_timezone() const;
  void                          set_timezone(std::string const& timezone);

  // Host notification properties.
  command_map const&            get_host_notification_commands() const;
  command_map&                  get_host_notification_commands();
  bool                          get_host_notifications_enabled() const;
  void                          set_host_notifications_enabled(bool enabled);
  unsigned int                  get_host_notified_states() const;
  void                          set_host_notified_states(
                                  unsigned int notified_states);
  timeperiod_struct*            get_host_notification_period() const;
  void                          set_host_notification_period(
                                  timeperiod* tp);

  void                          enable_host_notifications();
  void                          disable_host_notifications();

  time_t                        get_last_host_notification() const;
  void                          set_last_host_notification(time_t t);
  unsigned long                 get_modified_host_attributes() const;
  void                          set_modified_host_attributes(
                                  unsigned long attr);

  bool                          notify_on_host_critical() const;
  bool                          notify_on_host_down() const;
  bool                          notify_on_host_recovery() const;
  bool                          notify_on_host_unreachable() const;
  bool                          notify_on_host_warning() const;
  void                          add_host_notification_command(
                                  std::string const& command_name);
  void                          clear_host_notification_commands();

  // Service notification properties.
  command_map const&            get_service_notification_commands() const;
  command_map&                  get_service_notification_commands();
  bool                          get_service_notifications_enabled() const;
  void                          set_service_notifications_enabled(bool enabled);
  unsigned int                  get_service_notified_states() const;
  void                          set_service_notified_states(
                                  unsigned int notified_states);
  void                          set_service_notification_period(
                                  timeperiod* tp);

  bool                          notify_on_service_recovery() const;
  bool                          notify_on_service_critical() const;
  bool                          notify_on_service_warning() const;
  time_t                        get_last_service_notification() const;
  void                          set_last_service_notification(time_t t);
  unsigned long                 get_modified_service_attributes() const;
  void                          set_modified_service_attributes(
                                  unsigned long attr);
  timeperiod_struct*            get_service_notification_period() const;
  void                          enable_service_notifications();
  void                          disable_service_notifications();
  void                          add_service_notification_command(
                                  std::string const& command_name);
  void                          clear_service_notification_commands();

  bool                          contains_illegal_object_chars() const;

  std::list<shared_ptr<contactgroup> > const&
                                get_contactgroups() const;

  std::list<shared_ptr<contactgroup> >&
                                get_contactgroups();

  void                          add_custom_variable(
                                  char const* varname,
                                  char const* varvalue);

  void                          clear_custom_variables();
  customvar_set const&          get_customvars() const;
  void                          set_customvar(customvar const& var);
  unsigned long                 get_modified_attributes() const;
  void                          set_modified_attributes(unsigned long attr);
  void                          update_status(int aggregated_dump);


  bool                          check(int* w, int* e);

 private:

  std::string                   _name;
  std::string                   _alias;
  std::string                   _email;
  std::string                   _pager;
  bool                          _can_submit_commands;

  std::vector<std::string>      _address;
  std::string                   _timezone;
  command_map                   _host_notification_commands;
  command_map                   _service_notification_commands;
  timeperiod*                   _host_notification_period;
  timeperiod*                   _service_notification_period;
  unsigned long                 _modified_attributes;
  unsigned long                 _modified_host_attributes;
  unsigned long                 _modified_service_attributes;
  time_t                        _last_host_notification;
  time_t                        _last_service_notification;
  unsigned int                  _service_notified_states;
  unsigned int                  _host_notified_states;
  std::list<shared_ptr<contactgroup> >
                                _contact_groups;

  customvar_set                 _vars;
  bool                          _host_notifications_enabled;
  bool                          _service_notifications_enabled;
  bool                          _retain_nonstatus_information;
  bool                          _retain_status_information;
};


CCE_END()

#endif // !CCE_CONTACT_HH
