#include "test_notifier.hh"

void test_notifier::set_in_downtime(bool downtime) {
  _in_downtime = true;
}

void test_notifier::set_is_flapping(bool flapping) {
  _is_flapping = true;
}

void test_notifier::set_current_notification_type(notifier::notification_type type) {
  _type = type;
}

void test_notifier::set_notification_interval(long interval) {
  _notification_interval = interval;
}

void test_notifier::set_current_notification_number(int number) {
  _current_notification_number = number;
}
