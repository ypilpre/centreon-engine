#include "test_notifier.hh"

void test_notifier::set_in_downtime(bool downtime) {
  _in_downtime = true;
}

long test_notifier::get_last_notification_date() {
  return _last_notification_date;
}

void test_notifier::set_is_flapping(bool flapping) {
  _is_flapping = true;
}

void test_notifier::set_state(int state) {
  _state = state;
}
