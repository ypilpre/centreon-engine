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

#ifndef CCE_CHECKS_CHECKABLE_HH
#  define CCE_CHECKS_CHECKABLE_HH

#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace           checks {

  /**
   *  @class checkable checkable.hh "com/centreon/engine/checks/checkable.hh"
   *  @brief Object executing checks.
   *
   */
  class             checkable {
   public:
                    checkable();
                    checkable(checkable const& other);
                    ~checkable();
    checkable&      operator=(checkable const& other);
    bool            is_flapping() const;
    int             get_current_state() const;
    int             get_last_hard_state() const;
    int             get_last_state() const;
    void            set_current_state(int state);
    void            set_last_hard_state(int state);
    void            set_last_state(int state);

   protected:
    bool            _in_downtime;
    bool            _is_flapping;
    int             _current_state;
    int             _last_state;
    int             _last_hard_state;
  };
}

CCE_END()

#endif // !CCE_CHECKS_CHECKABLE_HH
