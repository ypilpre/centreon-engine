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

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/macros/process.hh"
#include "com/centreon/engine/string.hh"
#include "test/macros/minimal_setup.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

// Stringification macros.
#define XSTR(x) #x
#define STR(x) XSTR(x)

// Values that will be set in host.
#define OUTPUT                  my recursive $LONGHOSTOUTPUT$
#define LONG_OUTPUT             my second recursive $HOSTPERFDATA$
#define PERF_DATA               end of recursion

// Expected result
#define EXPECTED                my recursive my second recursive end of recursion

/**
 *  Check that the process_macros_r is correctly recursive.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // Return value.
  int retval = 0;

  // Create minimal context.
  test::minimal_setup();

  // macro object.
  nagios_macros mac;
  memset(&mac, 0, sizeof(mac));

  delete [] host_list->plugin_output;
  host_list->plugin_output = string::dup(STR(OUTPUT));
  delete [] host_list->long_plugin_output;
  host_list->long_plugin_output = string::dup(STR(LONG_OUTPUT));
  delete [] host_list->perf_data;
  host_list->perf_data = string::dup(STR(PERF_DATA));
  mac.host_ptr = host_list;

  char const* str = "$HOSTOUTPUT$";
  char* result = NULL;
  process_macros_r(&mac, str, &result, RECURSIVE_MACRO_EVALUATION);

  if (::strcmp(result, STR(EXPECTED)) != 0) {
    std::cerr
      << "failing recursion: expected '" << STR(EXPECTED)
      << "', got '" << result << "'" << std::endl;
    retval |= 1;
  }

  delete [] result;

  return (retval);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
