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

#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

  /**
   *  @class contactgroup contactgroup.hh "com/centreon/engine/contactgroup.hh
   *  @brief Object representing a contactgroup user
   *
   */
class               contactgroup {
 public:
                    contactgroup();
                    contactgroup(contactgroup const& other);
  virtual           ~contactgroup();
  contactgroup&     operator=(contactgroup const& other);
  std::string&      get_name() const;
  bool              check(int* w, int* e);

 private:
  std::string       _name;
};

CCE_END()

#endif // !CCE_CONTACTGROUP_HH
