/*
** Copyright 2011-2013 Merethis
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

#ifndef CCE_OBJECTS_SERVICEGROUP_HH
#  define CCE_OBJECTS_SERVICEGROUP_HH

#  include "com/centreon/shared_ptr.hh"
#  include "com/centreon/unordered_hash.hh"

/* Forward declaration. */
CCE_BEGIN()
  class service;
CCE_END()

struct host_struct;
struct service_struct;

typedef struct                servicegroup_struct {
  char*                       group_name;
  char*                       alias;
  umap<std::pair<std::string, std::string>, com::centreon::shared_ptr<com::centreon::engine::service> >
                              members;
  char*                       notes;
  char*                       notes_url;
  char*                       action_url;
  struct servicegroup_struct* next;
  struct servicegroup_struct* nexthash;
}                             servicegroup;

/* Other HOSTGROUP structure. */
struct                        servicegroup_other_properties {
  unsigned int                servicegroup_id;
};

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

servicegroup* add_servicegroup(
                char const* name,
                char const* alias,
                char const* notes,
                char const* notes_url,
                char const* action_url);
int           is_host_member_of_servicegroup(
                servicegroup_struct* group,
                host_struct* hst);
int           is_service_member_of_servicegroup(
                servicegroup_struct* group,
                service_struct* svc);

#  ifdef __cplusplus
}

#    include <ostream>
#    include <string>
#    include "com/centreon/engine/namespace.hh"

bool          operator==(
                servicegroup const& obj1,
                servicegroup const& obj2) throw ();
bool          operator!=(
                servicegroup const& obj1,
                servicegroup const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, servicegroup const& obj);

CCE_BEGIN()

bool          add_service_to_servicegroup(
                servicegroup_struct* grp,
                std::string const& host_name,
                std::string const& description);

bool          is_servicegroup_exist(std::string const& name) throw ();
unsigned int  get_servicegroup_id(char const* name);

CCE_END()

#  endif /* C++ */

#endif // !CCE_OBJECTS_SERVICEGROUP_HH


