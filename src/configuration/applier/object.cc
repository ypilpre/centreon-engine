/*
** Copyright 2013 Merethis
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

#include <cstring>
#include <string>
#include <vector>
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

void applier::modify_cstr_if_different(char*& s1, char const* s2) {
  if (s1 != s2) {
    if (!s2) {
      delete[] s1;
      s1 = NULL;
    }
    else if (!s1 || strcmp(s1, s2))
      string::setstr(s1, s2);
  }
  return ;
}
