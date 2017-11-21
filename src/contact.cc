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

#include <memory>
#include <sstream>
#include "com/centreon/engine/commands/command.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "find.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;


/**************************************                                         
*                                     *                                         
*           Public Methods            *                                         
*                                     *                                         
**************************************/                                         

/**
 * Constructor.
 */
contact::contact() {}

/**
 * Copy constructor.
 *
 * @param[in] other Object to copy.
 */
contact::contact(contact const& other) {}

/**
 * Assignment operator.
 *
 * @param[in] other Object to copy.
 *
 * @return This object
 */
contact& contact::operator=(contact const& other) {
  return (*this);
}

/**
 * Destructor.
 */
contact::~contact() {
}

/**
 *  Return the contact name
 *
 *  @return a reference to the name
 */
std::string contact::get_name() const {
  return _name;
}

bool contact::check(int* w, int* e) {
  int warnings(0);
  int errors(0);

  /* check service notification commands */
  if (get_service_notification_commands().empty()) {
    logger(log_verification_error, basic)
      << "Error: Contact '" << get_name() << "' has no service "
      "notification commands defined!";
    errors++;
  }
  else
    for (umap<std::string, commands::command*>::iterator
           it(get_service_notification_commands().begin()),
           end(get_service_notification_commands().end());
           it != end;
           ++it) {
      std::string buf(it->first);
      size_t index(buf.find(buf, '!'));
      std::string command_name(buf.substr(0, index));
      commands::command* command = find_command(command_name.c_str());

      if (command == NULL) {
        logger(log_verification_error, basic)
          << "Error: Service notification command '"
          << command_name << "' specified for contact '"
          << get_name() << "' is not defined anywhere!";
	errors++;
      }

      /* save pointer to the command for later */
      it->second = command;
    }

//    for (commandsmember* temp_commandsmember = cntct->service_notification_commands;
//	 temp_commandsmember != NULL;
//	 temp_commandsmember = temp_commandsmember->next) {
//
//      /* check the host notification command */
//      char* buf = string::dup(temp_commandsmember->cmd);
//
//      /* get the command name, leave any arguments behind */
//      char* temp_command_name = my_strtok(buf, "!");
//
//      command* temp_command = find_command(temp_command_name);
//      if (temp_command == NULL) {
//        logger(log_verification_error, basic)
//          << "Error: Service notification command '"
//          << temp_command_name << "' specified for contact '"
//          << cntct->name << "' is not defined anywhere!";
//	errors++;
//      }
//
//      /* save pointer to the command for later */
//      temp_commandsmember->command_ptr = temp_command;
//
//      delete[] buf;
//    }

  /* check host notification commands */
  if (get_host_notification_commands().empty()) {
    logger(log_verification_error, basic)
      << "Error: Contact '" << get_name() << "' has no host "
      "notification commands defined!";
    errors++;
  }
  else
    for (umap<std::string, commands::command*>::iterator
           it(_host_notification_commands.begin()),
           end(_host_notification_commands.end());
           it != end;
           ++it) {
      std::string buf(it->first);
      size_t index(buf.find('!'));
      std::string command_name(buf.substr(0, index));
      commands::command* command = find_command(command_name.c_str());

      if (command == NULL) {
        logger(log_verification_error, basic)
          << "Error: Host notification command '" << command_name
          << "' specified for contact '" << get_name()
          << "' is not defined anywhere!";
	errors++;
      }

      /* save pointer to the command for later */
      it->second = command;
    }
//    for (commandsmember* temp_commandsmember = cntct->host_notification_commands;
//	 temp_commandsmember != NULL;
//	 temp_commandsmember = temp_commandsmember->next) {
//
//      /* check the host notification command */
//      char* buf = string::dup(temp_commandsmember->cmd);
//
//      /* get the command name, leave any arguments behind */
//      char* temp_command_name = my_strtok(buf, "!");
//
//      command* temp_command = find_command(temp_command_name);
//      if (temp_command == NULL) {
//        logger(log_verification_error, basic)
//          << "Error: Host notification command '" << temp_command_name
//          << "' specified for contact '" << cntct->name
//          << "' is not defined anywhere!";
//	errors++;
//      }
//
//      /* save pointer to the command for later */
//      temp_commandsmember->command_ptr = temp_command;
//
//      delete[] buf;
//    }

  /* check service notification timeperiod */
  if (get_service_notification_period() == NULL) {
    logger(log_verification_error, basic)
      << "Warning: Contact '" << get_name() << "' has no service "
      "notification time period defined!";
    warnings++;
  }
  else {
    timeperiod_struct* temp_timeperiod(
      find_timeperiod(get_service_notification_period()));
    if (temp_timeperiod == NULL) {
      logger(log_verification_error, basic)
        << "Error: Service notification period '"
        << get_service_notification_period()
        << "' specified for contact '" << get_name()
        << "' is not defined anywhere!";
      errors++;
    }
  }
  set_service_notification_period(temp_timeperiod);

//  if (cntct->service_notification_period == NULL) {
//    logger(log_verification_error, basic)
//      << "Warning: Contact '" << cntct->name << "' has no service "
//      "notification time period defined!";
//    warnings++;
//  }
//
//  else {
//    timeperiod* temp_timeperiod
//      = find_timeperiod(cntct->service_notification_period);
//    if (temp_timeperiod == NULL) {
//      logger(log_verification_error, basic)
//        << "Error: Service notification period '"
//        << cntct->service_notification_period
//        << "' specified for contact '" << cntct->name
//        << "' is not defined anywhere!";
//      errors++;
//    }
//
//    /* save the pointer to the service notification timeperiod for later */
//    cntct->service_notification_period_ptr = temp_timeperiod;
//  }

  /* check host notification timeperiod */
  if (get_host_notification_period() == NULL) {
    logger(log_verification_error, basic)
      << "Warning: Contact '" << cntct->name << "' has no host "
      "notification time period defined!";
    warnings++;
  }
  else {
    timepriod* temp_timeperiod
      = find_timeperiod(get_host_notification_period());
    if (temp_timeperiod == NULL) {
      logger(log_verification_error, basic)
        << "Error: Host notification period '"
        << cntct->get_host_notification_period()
        << "' specified for contact '" << get_name()
        << "' is not defined anywhere!";
      errors++;
    }

    /* save the pointer to the host notification timeperiod for later */
    set_host_notification_period(temp_timeperiod);
  }
//  if (cntct->host_notification_period == NULL) {
//    logger(log_verification_error, basic)
//      << "Warning: Contact '" << cntct->name << "' has no host "
//      "notification time period defined!";
//    warnings++;
//  }
//
//  else {
//    timeperiod* temp_timeperiod
//      = find_timeperiod(cntct->host_notification_period);
//    if (temp_timeperiod == NULL) {
//      logger(log_verification_error, basic)
//        << "Error: Host notification period '"
//        << cntct->host_notification_period
//        << "' specified for contact '" << cntct->name
//        << "' is not defined anywhere!";
//      errors++;
//    }
//
//    /* save the pointer to the host notification timeperiod for later */
//    cntct->host_notification_period_ptr = temp_timeperiod;
//  }

  /* check for sane host recovery options */
  if (notify_on_host_recovery()
      && !notify_on_host_down()
      && !notify_on_host_unreachable()) {
    logger(log_verification_error, basic)
      << "Warning: Host recovery notification option for contact '"
      << get_name() << "' doesn't make any sense - specify down "
      "and/or unreachable options as well";
    warnings++;
  }
//  if (cntct->notify_on_host_recovery == true
//      && cntct->notify_on_host_down == false
//      && cntct->notify_on_host_unreachable == false) {
//    logger(log_verification_error, basic)
//      << "Warning: Host recovery notification option for contact '"
//      << cntct->name << "' doesn't make any sense - specify down "
//      "and/or unreachable options as well";
//    warnings++;
//  }

  /* check for sane service recovery options */
  if (notify_on_service_recovery()
      && !notify_on_service_critical()
      && !notify_on_service_warning()) {
    logger(log_verification_error, basic)
      << "Warning: Service recovery notification option for contact '"
      << get_name() << "' doesn't make any sense - specify critical "
      "and/or warning options as well";
    warnings++;
  }
//  if (cntct->notify_on_service_recovery == true
//      && cntct->notify_on_service_critical == false
//      && cntct->notify_on_service_warning == false) {
//    logger(log_verification_error, basic)
//      << "Warning: Service recovery notification option for contact '"
//      << cntct->name << "' doesn't make any sense - specify critical "
//      "and/or warning options as well";
//    warnings++;
//  }

  /* check for illegal characters in contact name */
  if (contains_illegal_object_chars()) {
    logger(log_verification_error, basic)
      << "Error: The name of contact '" << cntct->name
      << "' contains one or more illegal characters.";
    errors++;
  }
//  if (contains_illegal_object_chars(cntct->name) == true) {
//    logger(log_verification_error, basic)
//      << "Error: The name of contact '" << cntct->name
//      << "' contains one or more illegal characters.";
//    errors++;
//  }

  if (w != NULL)
    *w += warnings;
  if (e != NULL)
    *e += errors;
  return (errors == 0);
}

umap<std::string, command*>& get_host_notification_commands() const {
  return _host_notification_commands;
}

umap<std::string, command*>& get_service_notification_commands() const {
  return _service_notification_commands;
}

timeperiod* contact::get_host_notification_period() const {
  return _host_notification_period;
}

timeperiod* contact::get_service_notification_period() const {
  return _service_notification_period;
}

void contact::set_host_notification_period(timeperiod* tp) const {
  _host_notification_period = tp;
}

void contact::set_service_notification_period(timeperiod* tp) const {
  _service_notification_period = tp;
}
