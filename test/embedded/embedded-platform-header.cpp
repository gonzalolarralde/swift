// RUN: %empty-directory(%t)
// RUN: %target-clang -x c++ -std=c++17 -fsyntax-only -I %swift_obj_root/include %s

// REQUIRES: swift_in_compiler
// REQUIRES: swift_feature_Embedded
// REQUIRES: swift_embedded_platform

#include "swift/EmbeddedPlatform.h"

static_assert(__SWIFT_TLS_KEY_COUNT >= 1, "reserved TLS keys must exist");

void useEmbeddedPlatformHooks() {
  __swift_mutex_t m = 0;
  __swift_condition_t cond = 0;
  __swift_once_t once = 0;
  __swift_tls_key_t key = 0;
  __swift_thread_id_t threadId = 0;
  void *ptr = nullptr;
  void *low = nullptr;
  void *high = nullptr;

  _swift_mutex_init(&m, 0);
  _swift_mutex_destroy(&m);
  _swift_mutex_lock(&m);
  _swift_mutex_unlock(&m);
  (void)_swift_mutex_tryLock(&m);

  _swift_condition_init(&cond);
  _swift_condition_lock(&cond);
  _swift_condition_unlock(&cond);
  _swift_condition_signal(&cond);
  _swift_condition_broadcast(&cond);
  _swift_condition_wait(&cond);
  (void)_swift_condition_waitFor(&cond, 0);
  (void)_swift_condition_waitUntil(&cond, 0);
  _swift_condition_destroy(&cond);

  (void)once;
  _swift_once(&once, [](void *ctx) {
    (void)ctx;
  }, nullptr);

  _swift_tls_init(key, nullptr);
  _swift_tls_set(key, ptr);
  (void)_swift_tls_get(key);

  (void)_swift_thread_getCurrentId();
  (void)_swift_thread_isMain();
  (void)_swift_thread_getCurrentStackBounds(&low, &high);

  _swift_getExclusivityTLS();
  _swift_setExclusivityTLS(ptr);

  threadId = _swift_thread_getCurrentId();
  (void)threadId;
}
