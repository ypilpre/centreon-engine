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

#ifndef CCE_CONTACTGROUP_HH
#  define CCE_CONTACTGROUP_HH

#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"
#  include "com/centreon/unordered_hash.hh"

CCE_BEGIN()

// Forward declaration
class contact;

namespace configuration {
  class contactgroup;
}

  /**
   *  @class contactgroup contactgroup.hh "com/centreon/engine/contactgroup.hh
   *  @brief Object representing a contactgroup user
   *
   */
class                          contactgroup {
 public:
  static contactgroup*         add_contactgroup(
                                 char const* name,
                                 char const* alias);
                               contactgroup(
                                 char const* name,
                                 char const* alias);
                               contactgroup();
                               contactgroup(contactgroup const& other);
  virtual                      ~contactgroup();
  contactgroup&                operator=(contactgroup const& other);
  bool                         operator<(contactgroup const& other) const;
  bool                         operator!=(contactgroup const& other) const;
  std::string const&           get_name() const;
  std::string const&           get_alias() const;
  bool                         check(int* w, int* e);
  umap<std::string, com::centreon::shared_ptr<contact> > const&
                               get_members() const;

  umap<std::string, com::centreon::shared_ptr<contact> >&
                               get_members();

  bool                         contains_member(std::string const& name) const;
  bool                         contains_illegal_object_chars() const;
  void                         add_contact(std::string const& contact_name);
  void                         update_config(
                                 configuration::contactgroup const& obj);
  void                         clear_members();

 private:
  std::string                  _name;
  std::string                  _alias;
  umap<std::string, com::centreon::shared_ptr<contact> >
                               _members;
};

CCE_END()

#endif // !CCE_CONTACTGROUP_HH
