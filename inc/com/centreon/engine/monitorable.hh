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

#ifndef CCE_MONITORABLE_HH
#  define CCE_MONITORABLE_HH

#  include <string>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/notifications/notifier.hh"

CCE_BEGIN()

/**
 *  @class monitorable monitorable.hh "com/centreon/engine/monitorable.hh"
 *  @brief Monitorable node.
 *
 *  Monitorable nodes are checked and notified of problems.
 */
class                monitorable : public notifications::notifier {
 public:
                     monitorable();
                     monitorable(monitorable const& other);
  virtual            ~monitorable();
  monitorable&       operator=(monitorable const& other);
  std::string const& get_host_name() const;
};

CCE_END()

#endif // !CCE_MONITORABLE_HH
