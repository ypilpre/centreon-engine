/*
** Copyright 2016-2018 Centreon
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

#include <cstdlib>
#include "com/centreon/engine/downtime.hh"
#include "com/centreon/engine/downtime_finder.hh"
#include "com/centreon/engine/downtime_manager.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon::engine;

// Helper macro.
#define ARE_STRINGS_MATCHING(stdstring, cstring) \
  ((cstring && (cstring == stdstring)) || (!cstring && stdstring.empty()))

/**
 *  Constructor.
 *
 *  @param[in] list  Active downtime list. The search will be performed
 *                   on this list.
 */
downtime_finder::downtime_finder() {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
downtime_finder::downtime_finder(downtime_finder const& other) {
  (void)other;
}

/**
 *  Destructor.
 */
downtime_finder::~downtime_finder() {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 */
downtime_finder& downtime_finder::operator=(downtime_finder const& other) {
  (void)other;
  return (*this);
}

/**
 *  Find downtimes that match all the criterias.
 *
 *  @param[in] criterias  Search criterias.
 */
downtime_finder::result_set downtime_finder::find_matching_all(
  downtime_finder::criteria_set const& criterias) {
  result_set result;
  // Process all downtimes.
  for (umap<unsigned long, downtime>::const_iterator
         dit(downtime_manager::instance().get_downtimes().begin()),
         dend(downtime_manager::instance().get_downtimes().end());
       dit != dend;
       ++dit) {
    // Process all criterias.
    bool matched_all(true);
    for (criteria_set::const_iterator
           cit(criterias.begin()), cend(criterias.end());
         cit != cend;
         ++cit) {
      if (!_match_criteria(dit->second, *cit)) {
        matched_all = false;
        break;
      }
    }

    // If downtime matched all criterias, add it to the result set.
    if (matched_all)
      result.push_back(dit->second.get_id());
  }
  return (result);
}

/**
 *  Check that a downtime match a specific criteria.
 *
 *  @param[in] dt    Downtime.
 *  @param[in] crit  Search criteria.
 *
 *  @return True if downtime matches the criteria.
 */
bool downtime_finder::_match_criteria(
                        downtime const& dt,
                        downtime_finder::criteria const& crit) {
  bool retval;
  host* hst(NULL);
  service* svc(NULL);

  if (dt.get_parent()->is_host()) {
    hst = static_cast<host*>(dt.get_parent());
  }
  else {
    svc = static_cast<service*>(dt.get_parent());
    hst = svc->get_host();
  }

  if (crit.first == "host") {
    retval = (hst && crit.second == hst->get_name());
  }
  else if (crit.first == "service") {
    retval = ((svc && crit.second == svc->get_description())
                 || (!svc && crit.second == ""));
  }
  else if (crit.first == "start") {
    time_t expected(strtoll(crit.second.c_str(), NULL, 0));
    retval = (expected == dt.get_start_time());
  }
  else if (crit.first == "end") {
    time_t expected(strtoll(crit.second.c_str(), NULL, 0));
    retval = (expected == dt.get_end_time());
  }
  else if (crit.first == "fixed") {
    bool expected(strtol(crit.second.c_str(), NULL, 0));
    retval = (expected == static_cast<bool>(dt.get_fixed()));
  }
  else if (crit.first == "triggered_by") {
    unsigned long expected(strtoul(crit.second.c_str(), NULL, 0));
    retval = (expected == dt.get_triggered_by());
  }
  else if (crit.first == "duration") {
    unsigned long expected(strtoul(crit.second.c_str(), NULL, 0));
    retval = (expected == dt.get_duration());
  }
  else if (crit.first == "author") {
    retval = (crit.second == dt.get_author());
  }
  else if (crit.first == "comment") {
    retval = (crit.second == dt.get_comment());
  }
  else {
    retval = false;
  }
  return (retval);
}
