/*
** Copyright 2011 Merethis
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

#include "logging/logger.hh"
#include "objects/commandsmember.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::release
 */
commandsmember const* release_commandsmember(commandsmember const* obj) {
  try {
    return (objects::release(obj));
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << e.what();
  }
  catch (...) {
    logger(log_runtime_error, basic) << Q_FUNC_INFO << " unknow exception.";
  }
  return (NULL);
}

/**
 *  Cleanup memory of commandsmember.
 *
 *  @param[in] obj The command member to cleanup memory.
 *
 *  @return The next commandsmember.
 */
commandsmember const* objects::release(commandsmember const* obj) {
  if (obj == NULL)
    return (NULL);

  commandsmember const* next = obj->next;
  delete[] obj->cmd;
  delete obj;
  return (next);
}
