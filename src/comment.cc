/*
** Copyright 2017-2018 Centreon
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
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/timed_event.hh"
#include "com/centreon/engine/monitorable.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::checks;

// Chained hash limits.
int const COMMENT_HASHSLOTS = 1024;

unsigned long comment::_next_id(1);

/******************************************************************/
/********************** SEARCH FUNCTIONS **************************/
/******************************************************************/

/**
 *  Deletes a comment from its ID. If the comment does not exist, nothing
 *  is done.
 *
 * @param comment_id The comment ID.
 */
void comment::delete_comment(unsigned long comment_id) {

  /* find the comment we should remove */
  std::map<unsigned long, comment*>::iterator it = comment_list.find(comment_id);

  /* remove the comment from the list in memory */
  if (it != comment_list.end()) {
    comment* my_comment(it->second);

    /* then removed from linked list */
    comment_list.erase(it);

    /* free memory */
    delete my_comment;
  }
}

/* adds a new host or service comment */
comment* comment::add_new_comment(
      comment::comment_type type,
      comment::entry_type ent_type,
      notifications::notifier* parent,
      time_t entry_time,
      std::string const& author_name,
      std::string const& comment_data,
      int persistent,
      source_type source,
      bool expires,
      time_t expire_time) {
  comment* retval(new comment(
                    type,
                    ent_type,
                    parent,
                    entry_time,
                    author_name,
                    comment_data,
                    persistent,
                    expires,
                    expire_time,
                    source));

  unsigned long comment_id(retval->get_id());
  comment_list.insert(std::make_pair(comment_id, retval));

  /* send data to event broker */
  broker_comment_data(
    NEBTYPE_COMMENT_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    retval->get_comment_type(),
    ent_type,
    parent,
    entry_time,
    author_name,
    comment_data,
    persistent,
    source,
    expires,
    expire_time,
    comment_id,
    NULL);

  /* add an event to expire comment data if necessary... */
  if (expires)
    schedule_new_event(
      EVENT_EXPIRE_COMMENT,
      false,
      expire_time,
      false,
      0,
      NULL,
      true,
      (void*)comment_id,
      NULL,
      0);

  return retval;
}

/* checks for an expired comment (and removes it) */
void comment::check_for_expired_comment(unsigned long comment_id) {
  /* check all comments */
  std::map<unsigned long, comment*>::iterator it(comment_list.find(comment_id));
  if (it != comment_list.end()) {
    comment* my_comment(it->second);
    if (my_comment->get_expire_time() < time(NULL)) {
      comment_list.erase(it);
      delete my_comment;
    }
  }
}

comment::comment(
           comment::comment_type cmt_type,
           comment::entry_type ent_type,
           notifications::notifier* parent,
           time_t entry_time,
           std::string const& author,
           std::string const& comment_data,
           bool persistent,
           bool expires,
           time_t expire_time,
           source_type source,
           unsigned long comment_id)
  : _author(author),
    _comment_data(comment_data),
    _comment_id((comment_id == 0) ? _next_id++ : comment_id),
    _comment_type(cmt_type),
    _entry_time(entry_time),
    _entry_type(ent_type),
    _expires(expires),
    _expire_time(expire_time),
    _parent(parent),
    _persistent(persistent),
    _source(source) {
  if (_comment_id > _next_id)
    _next_id = comment_id + 1;

//  unsigned long comment_id(get_id());
//  comment_list.insert(std::make_pair(comment_id, retval));

  /* send data to event broker */
  broker_comment_data(
    NEBTYPE_COMMENT_LOAD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    get_comment_type(),
    get_entry_type(),
    parent,
    get_entry_time(),
    get_author(),
    get_comment_data(),
    get_persistent(),
    get_source(),
    get_expires(),
    get_expire_time(),
    get_id(),
    NULL);
}

comment::comment(comment const& other) {
  *this = other;
}

comment& comment::operator=(comment const& other) {
  if (this != &other) {
    _author = other._author;
    _comment_data = other._comment_data;
    _comment_id = other._comment_id;
    _comment_type = other._comment_type;
    _entry_time = other._entry_time;
    _entry_type = other._entry_type;
    _expires = other._expires;
    _expire_time = other._expire_time;
    _parent = other._parent;
    _persistent = other._persistent;
    _source = other._source;
  }
  return (*this);
}

comment::~comment() {
  /* send data to event broker */
  broker_comment_data(
      NEBTYPE_COMMENT_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      get_comment_type(),
      get_entry_type(),
      get_parent(),
      get_entry_time(),
      get_author(),
      get_comment_data(),
      get_persistent(),
      get_source(),
      get_expires(),
      get_expire_time(),
      get_id(),
      NULL);
}

comment::entry_type comment::get_entry_type() const {
  return _entry_type;
}

void comment::set_entry_type(entry_type ent_type) {
  _entry_type = ent_type;
}

notifications::notifier* comment::get_parent() const {
  return _parent;
}

std::string comment::get_host_name() const {
  if (get_comment_type() == HOST_COMMENT) {
    host* hst(static_cast<host*>(_parent));
    return hst->get_name();
  }
  else {
    service* svc(static_cast<service*>(_parent));
    return svc->get_host_name();
  }
}

std::string comment::get_service_description() const {
  if (get_comment_type() == SERVICE_COMMENT) {
    service* svc(static_cast<service*>(_parent));
    return svc->get_description();
  }
  else {
    return "";
  }
}


time_t comment::get_entry_time() const {
  return _entry_time;
}

void comment::set_entry_time(time_t ent_time) {
  _entry_time = ent_time;
}

std::string comment::get_author() const {
  return _author;
}

void comment::set_author(std::string const& author) {
  _author = author;
}

std::string comment::get_comment_data() const {
  return _comment_data;
}

void comment::set_comment_data(std::string const& comment_data) {
  _comment_data = comment_data;
}

unsigned long comment::get_id() const {
  return _comment_id;
}

void comment::set_id(unsigned long comment_id) {
  _comment_id = comment_id;

  /* Comment configuration forces the comment_id. So we must keep an internal
     id coherent with this one. */
  if (comment_id > _next_id)
    _next_id = comment_id + 1;
}

bool comment::get_persistent() const {
  return _persistent;
}

void comment::set_persistent(bool persistent) {
  _persistent = persistent;
}

int comment::get_source() const {
  return _source;
}

void comment::set_source(int source) {
  _source = source;
}

bool comment::get_expires() const {
  return _expires;
}

void comment::set_expires(bool expires) {
  _expires = expires;
}

time_t comment::get_expire_time() const {
  return _expire_time;
}

void comment::set_expire_time(time_t time) {
  _expire_time = time;
}

comment::comment_type comment::get_comment_type() const {
  return _comment_type;
}

void comment::set_comment_type(comment::comment_type comment_type) {
  _comment_type = comment_type;
}
