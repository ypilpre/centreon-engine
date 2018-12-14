/*
** Copyright 2016,2018 Centreon
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

#ifndef CCE_DOWNTIME_FINDER_HH
#  define CCE_DOWNTIME_FINDER_HH

#  include <list>
#  include <string>
#  include <vector>
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

// Forward declarations
class downtime;

/**
 *  @class downtime_finder downtime_finder.hh "com/centreon/engine/downtime_finder.hh"
 *  @brier Find active downtimes.
 *
 *  This class can find active downtimes according to some criterias.
 */
class                 downtime_finder {
 public:
  typedef std::pair<std::string, std::string>  criteria;
  typedef std::vector<criteria>                criteria_set;
  typedef std::vector<unsigned long>           result_set;

                      downtime_finder();
                      downtime_finder(downtime_finder const& other);
                      ~downtime_finder();
  downtime_finder&    operator=(downtime_finder const& other);
  result_set          find_matching_all(criteria_set const& criterias);

 private:
  bool                _match_criteria(
                        downtime const& dt,
                        criteria const& crit);
};

CCE_END()

#endif // !CCE_DOWNTIME_FINDER_HH
