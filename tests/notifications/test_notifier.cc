#include "test_notifier.hh"

test_notifier::test_notifier()
  : __is_host(false) {}

void test_notifier::set_in_downtime(bool downtime) {
  _in_downtime = true;
}

void test_notifier::add_notification_flag(notifier::notification_type type) {
  _current_notifications |= (1 << type);
}

bool test_notifier::is_host() const {
  return __is_host;
}

void test_notifier::set_is_host(bool is_host) {
  __is_host = is_host;
}

void test_notifier::_checkable_macro_builder(nagios_macros& mac) {}
