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
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/timed_event.hh"
#include "com/centreon/engine/monitorable.hh"
#include "com/centreon/engine/xcddefault.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::checks;

// Chained hash limits.
int const COMMENT_HASHSLOTS = 1024;

static comment* comment_hashlist[COMMENT_HASHSLOTS];

/******************************************************************/
/********************** SEARCH FUNCTIONS **************************/
/******************************************************************/

/* find a service comment by id */
comment* comment::find_service_comment(unsigned long comment_id) {
  return (comment::find_comment(comment_id, comment::SERVICE_COMMENT));
}

/* find a host comment by id */
comment* comment::find_host_comment(unsigned long comment_id) {
  return (comment::find_comment(comment_id, comment::HOST_COMMENT));
}

/* find a comment by id */
comment* comment::find_comment(
           unsigned long comment_id,
           comment::comment_type comment_type) {
  comment* temp_comment = NULL;

  //FIXME DBR: to review...
//  for (temp_comment = comment_list;
//       temp_comment != NULL;
//       temp_comment = temp_comment->next) {
//    if (temp_comment->comment_id == comment_id
//        && temp_comment->comment_type == comment_type)
//      return (temp_comment);
//  }
  return (NULL);
}
/* deletes a host comment */
int comment::delete_host_comment(unsigned long comment_id) {
  /* delete the comment from memory */
  return (delete_comment(comment::HOST_COMMENT, comment_id));
}

/* deletes a service comment */
int comment::delete_service_comment(unsigned long comment_id) {
  /* delete the comment from memory */
  return (delete_comment(comment::SERVICE_COMMENT, comment_id));
}

/* deletes a host or service comment */
int comment::delete_comment(unsigned int type, unsigned long comment_id) {
  int result = OK;

  /* find the comment we should remove */
  std::map<unsigned long, comment*>::iterator it = comment_list.find(comment_id);

  /* remove the comment from the list in memory */
  if (it != comment_list.end()) {
    comment* this_comment(it->second);

    /* send data to event broker */
    broker_comment_data(
      NEBTYPE_COMMENT_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      type, this_comment->get_entry_type(),
      this_comment->get_host_name(),
      this_comment->get_service_description(),
      this_comment->get_entry_time(),
      this_comment->get_author(),
      this_comment->get_comment_data(),
      this_comment->get_persistent(),
      this_comment->get_source(),
      this_comment->get_expires(),
      this_comment->get_expire_time(),
      comment_id,
      NULL);

    /* first remove from chained hash list */
    // FIXME DBR: hashlist to review...
//    hashslot = hashfunc(
//                 this_comment->get_host_name().c_str(),
//                 NULL,
//                 COMMENT_HASHSLOTS);
//    last_hash = NULL;
//    for (this_hash = comment_hashlist[hashslot];
//	 this_hash;
//	 this_hash = this_hash->nexthash) {
//      if (this_hash == this_comment) {
//        if (last_hash)
//          last_hash->nexthash = this_hash->nexthash;
//        else
//	  comment_hashlist[hashslot] = this_hash->nexthash;
//        break;
//      }
//      last_hash = this_hash;
//    }

    /* then removed from linked list */
    comment_list.erase(it);

    /* free memory */
    delete this_comment;

    result = OK;    // FIXME DBR: stupidity, result is overwrite in the next lines
  }
  else
    result = ERROR;   // FIXME DBR: idem than above.

  if (type == comment::HOST_COMMENT)
    result = xcddefault_delete_host_comment(comment_id);
  else
    result = xcddefault_delete_service_comment(comment_id);
  return (result);
}

/******************************************************************/
/***************** COMMENT DELETION FUNCTIONS *********************/
/******************************************************************/

/* deletes all comments for a particular host or service */
int comment::delete_all_comments(
      unsigned int type,
      std::string const& host_name,
      std::string const& svc_description) {
  if (type == comment::HOST_COMMENT)
    return (comment::delete_all_host_comments(host_name));
  return (comment::delete_all_service_comments(host_name, svc_description));
}

/* deletes all comments for a particular host */
int comment::delete_all_host_comments(std::string const& host_name) {
  comment* temp_comment = NULL;
  comment* next_comment = NULL;

  if (host_name.empty())
    return (ERROR);

  /* delete host comments from memory */
  //FIXME DBR: we must review the list
//  for (temp_comment = get_first_comment_by_host(host_name);
//       temp_comment != NULL;
//       temp_comment = next_comment) {
//    next_comment = get_next_comment_by_host(host_name, temp_comment);
//    if (temp_comment->comment_type == HOST_COMMENT)
//      delete_comment(HOST_COMMENT, temp_comment->comment_id);
//  }

  return (OK);
}

/* deletes all non-persistent acknowledgement comments for a particular host */
int comment::delete_host_acknowledgement_comments(host* hst) {
  comment* temp_comment = NULL;
  comment* next_comment = NULL;

  if (hst == NULL)
    return (ERROR);

  /* delete comments from memory */
  // FIXME DBR: the hash table must be review for comments
//  for (temp_comment = get_first_comment_by_host(hst->get_host_name().c_str());
//       temp_comment != NULL;
//       temp_comment = next_comment) {
//    next_comment = get_next_comment_by_host(
//                     hst->get_host_name().c_str(),
//                     temp_comment);
//    if (temp_comment->comment_type == HOST_COMMENT
//        && temp_comment->entry_type == ACKNOWLEDGEMENT_COMMENT
//        && temp_comment->persistent == false)
//      delete_comment(HOST_COMMENT, temp_comment->comment_id);
//  }
  return (OK);
}

/* deletes all comments for a particular service */
int comment::delete_all_service_comments(
      std::string const& host_name,
      std::string const& svc_description) {
  comment* temp_comment = NULL;
  comment* next_comment = NULL;

  if (host_name.empty() || svc_description.empty())
    return (ERROR);

  /* delete service comments from memory */
  // FIXME DBR: the comments container is broken
//  for (temp_comment = comment_list;
//       temp_comment != NULL;
//       temp_comment = next_comment) {
//    next_comment = temp_comment->next;
//    if (temp_comment->comment_type == SERVICE_COMMENT
//        && !strcmp(temp_comment->host_name, host_name)
//        && !strcmp(temp_comment->service_description, svc_description))
//      delete_comment(SERVICE_COMMENT, temp_comment->comment_id);
//  }
  return (OK);
}

/* deletes all non-persistent acknowledgement comments for a particular service */
int comment::delete_service_acknowledgement_comments(service* svc) {
  comment* temp_comment = NULL;
  comment* next_comment = NULL;

  if (svc == NULL)
    return (ERROR);

  /* delete comments from memory */
  // FIXME DBR: the comments list has to be rewritten...
//  for (temp_comment = comment_list;
//       temp_comment != NULL;
//       temp_comment = next_comment) {
//    next_comment = temp_comment->next;
//    if (temp_comment->comment_type == SERVICE_COMMENT
//        && !strcmp(temp_comment->host_name, svc->get_host_name().c_str())
//        && !strcmp(temp_comment->service_description, svc->get_description().c_str())
//        && temp_comment->entry_type == ACKNOWLEDGEMENT_COMMENT
//        && temp_comment->persistent == false)
//      delete_comment(SERVICE_COMMENT, temp_comment->comment_id);
//  }
  return (OK);
}

/* adds a host comment to the list in memory */
int comment::add_host_comment(
      comment::entry_type entry_type,
      std::string const& host_name,
      time_t entry_time,
      std::string const& author,
      std::string const& comment_data,
      unsigned long comment_id,
      int persistent,
      int expires,
      time_t expire_time,
      source_type source) {
  return (comment::add_comment(
            comment::HOST_COMMENT,
            entry_type,
            host_name,
            "",
            entry_time,
            author,
            comment_data,
            comment_id,
            persistent,
            expires,
            expire_time,
            source));
}

/* adds a service comment to the list in memory */
int comment::add_service_comment(
      comment::entry_type entry_type,
      std::string const& host_name,
      std::string const& svc_description,
      time_t entry_time,
      std::string const& author,
      std::string const& comment_data,
      unsigned long comment_id,
      int persistent,
      int expires,
      time_t expire_time,
      source_type source) {
  return (comment::add_comment(
            comment::SERVICE_COMMENT,
            entry_type,
            host_name,
            svc_description,
            entry_time,
            author,
            comment_data,
            comment_id,
            persistent,
            expires,
            expire_time,
            source));
}

/* adds a comment to the list in memory */
int comment::add_comment(
      comment_type comment_type,
      entry_type entry_type,
      std::string const& host_name,
      std::string const& svc_description,
      time_t entry_time,
      std::string const& author,
      std::string const& comment_data,
      unsigned long comment_id,
      int persistent,
      int expires,
      time_t expire_time,
      source_type source) {
  /* make sure we have the data we need */
//  if (host_name == NULL
//      || author == NULL
//      || comment_data == NULL
//      || (comment_type == SERVICE_COMMENT
//          && svc_description == NULL))
//    return (ERROR);

  /* allocate memory for the comment */
  // FIXME DBR: bad allocation...
//  comment* new_comment = NULL;
//  comment* new_comment(new comment);
//  memset(new_comment, 0, sizeof(*new_comment));

  /* duplicate vars */
//  new_comment->set_host_name(host_name);
//  if (comment_type == comment::SERVICE_COMMENT)
//    new_comment->set_service_description(svc_description);
//  new_comment->set_author(author);
//  new_comment->set_comment_data(comment_data);
//  new_comment->set_comment_type(comment_type);
//  new_comment->set_entry_type(entry_type);
//  new_comment->set_source(source);
//  new_comment->set_entry_time(entry_time);
//  new_comment->set_comment_id(comment_id);
//  new_comment->set_persistent(persistent);
//  new_comment->set_expires(expires);
//  new_comment->set_expire_time(expire_time);

  comment* new_comment(new comment(
                             comment_type,
                             entry_type,
                             host_name,
                             (comment_type == comment::SERVICE_COMMENT)
                               ? svc_description : "",
                             entry_time,
                             author,
                             comment_data,
                             comment_id,
                             persistent,
                             expires,
                             expire_time,
                             source));

  /* add comment to hash list */
  //FIXME DBR: list to rewrite...
  //add_comment_to_hashlist(new_comment);

  comment_list.insert(std::make_pair(new_comment->get_comment_id(), new_comment));

  /* send data to event broker */
  broker_comment_data(
    NEBTYPE_COMMENT_LOAD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    comment_type,
    entry_type,
    host_name,
    svc_description,
    entry_time,
    author,
    comment_data,
    persistent,
    source,
    expires,
    expire_time,
    comment_id,
    NULL);
  return (OK);
}

/* adds a new host or service comment */
int comment::add_new_comment(
      comment::comment_type type,
      comment::entry_type ent_type,
      std::string const& host_name,
      std::string const& svc_description,
      time_t entry_time,
      std::string const& author_name,
      std::string const& comment_data,
      int persistent,
      source_type source,
      int expires,
      time_t expire_time,
      unsigned long* comment_id) {
  int result = 0;
  unsigned long new_comment_id = 0L;

  if (type == comment::HOST_COMMENT)
    result = comment::add_new_host_comment(
               ent_type,
               host_name,
               entry_time,
               author_name,
               comment_data,
               persistent,
               source,
               expires,
               expire_time,
               &new_comment_id);
  else
    result = comment::add_new_service_comment(
               ent_type,
               host_name,
               svc_description,
               entry_time,
               author_name,
               comment_data,
               persistent,
               source,
               expires,
               expire_time,
               &new_comment_id);

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
      (void*)new_comment_id,
      NULL,
      0);

  /* save comment id */
  if (comment_id != NULL)
    *comment_id = new_comment_id;

  return (result);
}

/* adds a new host comment */
int comment::add_new_host_comment(
      comment::entry_type ent_type,
      std::string const& host_name,
      time_t entry_time,
      std::string const& author_name,
      std::string const& comment_data,
      int persistent,
      source_type source,
      int expires,
      time_t expire_time,
      unsigned long* comment_id) {
  int result = OK;
  unsigned long new_comment_id = 0L;

  result = xcddefault_add_new_host_comment(
             ent_type,
             host_name,
             entry_time,
             author_name,
             comment_data,
             persistent,
             source,
             expires,
             expire_time,
             &new_comment_id);

  /* save comment id */
  if (comment_id != NULL)
    *comment_id = new_comment_id;

  /* send data to event broker */
  broker_comment_data(
    NEBTYPE_COMMENT_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    comment::HOST_COMMENT,
    ent_type,
    host_name,
    "",
    entry_time,
    author_name,
    comment_data,
    persistent,
    source,
    expires,
    expire_time,
    new_comment_id,
    NULL);
  return (result);
}

/* adds a new service comment */
int comment::add_new_service_comment(
      comment::entry_type ent_type,
      std::string const& host_name,
      std::string const& svc_description,
      time_t entry_time,
      std::string const& author_name,
      std::string const& comment_data,
      int persistent,
      source_type source,
      int expires,
      time_t expire_time,
      unsigned long* comment_id) {
  int result = OK;
  unsigned long new_comment_id = 0L;

  result = xcddefault_add_new_service_comment(
             ent_type,
             host_name,
             svc_description,
             entry_time,
             author_name,
             comment_data,
             persistent,
             source,
             expires,
             expire_time,
             &new_comment_id);

  /* save comment id */
  if (comment_id != NULL)
    *comment_id = new_comment_id;

  /* send data to event broker */
  broker_comment_data(
    NEBTYPE_COMMENT_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    comment::SERVICE_COMMENT,
    ent_type,
    host_name,
    svc_description,
    entry_time,
    author_name,
    comment_data,
    persistent,
    source,
    expires,
    expire_time,
    new_comment_id,
    NULL);
  return (result);
}

/* checks for an expired comment (and removes it) */
int comment::check_for_expired_comment(unsigned long comment_id) {
  comment* temp_comment = NULL;

  /* check all comments */
  //FIXME DBR: comment list to review
//  for (temp_comment = comment_list;
//       temp_comment != NULL;
//       temp_comment = temp_comment->next) {
//
//    /* delete the now expired comment */
//    if (temp_comment->comment_id == comment_id
//        && temp_comment->expires == true
//        && temp_comment->expire_time < time(NULL)) {
//      delete_comment(temp_comment->comment_type, comment_id);
//      break;
//    }
//  }
  return (OK);
}


comment::comment(
           comment::comment_type cmt_type,
           comment::entry_type ent_type,
           std::string const& host_name,
           std::string const& service_description,
           time_t entry_time,
           std::string const& author,
           std::string const& comment_data,
           unsigned long comment_id,
           bool persistent,
           bool expires,
           time_t expire_time,
           source_type source)
  : _comment_type(cmt_type),
    _entry_type(ent_type),
    _host_name(host_name),
    _service_description(service_description),
    _entry_time(entry_time),
    _author(author),
    _comment_data(comment_data),
    _comment_id(comment_id),
    _persistent(persistent),
    _expires(expires),
    _expire_time(expire_time),
    _source(source) {}

comment::comment(comment const& other)
  : _author(other._author),
    _comment_data(other._comment_data),
    _comment_type(other._comment_type),
    _entry_type(other._entry_type),
    _source(other._source),
    _entry_time(other._entry_time),
    _comment_id(other._comment_id),
    _persistent(other._persistent),
    _expires(other._expires),
    _expire_time(other._expire_time) {}

comment& comment::operator=(comment const& other) {
  if (this != &other) {
    _author = other._author;
    _comment_data = other._comment_data;
    _comment_type = other._comment_type;
    _entry_type = other._entry_type;
    _source = other._source;
    _entry_time = other._entry_time;
    _comment_id = other._comment_id;
    _persistent = other._persistent;
    _expires = other._expires;
    _expire_time = other._expire_time;
  }
  return *this;
}

comment::entry_type comment::get_entry_type() const {
  return _entry_type;
}

void comment::set_entry_type(entry_type ent_type) {
  _entry_type = ent_type;
}

std::string comment::get_host_name() const {
  return _host_name;
}

void comment::set_host_name(std::string const& host_name) {
  _host_name = host_name;
}

std::string comment::get_service_description() const {
  return _service_description;
}

void comment::set_service_description(std::string const& service_description) {
  _service_description = service_description;
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

unsigned long comment::get_comment_id() const {
  return _comment_id;
}

void comment::set_comment_id(unsigned long comment_id) {
  _comment_id = comment_id;
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

unsigned int comment::get_comment_type() const {
  return _comment_type;
}

void comment::set_comment_type(unsigned int comment_type) {
  _comment_type = comment_type;
}
