// RUN: %empty-directory(%t)
// RUN: %target-clang -x c -std=c11 -fsyntax-only -I %swift_obj_root/include %s

// REQUIRES: swift_in_compiler
// REQUIRES: swift_feature_Embedded
// REQUIRES: swift_embedded_platform

#include "swift/EmbeddedPlatform.h"

static void swiftPlatformOnceFn(void *context) {
  (void)context;
}

void exerciseSingleThreadedShim() {
  __swift_mutex_t mutex = 0;
  __swift_condition_t condition = 0;
  __swift_once_t once = 0;
  __swift_tls_key_t key = 0;
  void *value = (void *)0x1;

  _swift_mutex_init(&mutex, 0);
  _swift_mutex_destroy(&mutex);
  _swift_mutex_lock(&mutex);
  _swift_mutex_unlock(&mutex);
  (void)_swift_mutex_tryLock(&mutex);

  _swift_condition_init(&condition);
  _swift_condition_lock(&condition);
  _swift_condition_unlock(&condition);
  _swift_condition_signal(&condition);
  _swift_condition_broadcast(&condition);
  _swift_condition_wait(&condition);
  (void)_swift_condition_waitFor(&condition, 0);
  (void)_swift_condition_waitUntil(&condition, 0);
  _swift_condition_destroy(&condition);

  _swift_once(&once, swiftPlatformOnceFn, 0);

  (void)_swift_tls_init(key, 0);
  _swift_tls_set(key, value);
  (void)_swift_tls_get(key);

  (void)_swift_thread_getCurrentId();
  (void)_swift_thread_isMain();
  void *low = 0;
  void *high = 0;
  (void)_swift_thread_getCurrentStackBounds(&low, &high);

  _swift_getExclusivityTLS();
  _swift_setExclusivityTLS(value);
  _swift_exit(0);
}
