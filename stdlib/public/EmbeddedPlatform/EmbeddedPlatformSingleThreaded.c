/*===----------- EmbeddedPlatformSingleThreaded.c -------------------------===*
 *
 * This source file is part of the Swift.org open source project
 *
 * Copyright (c) 2026 Apple Inc. and the Swift project authors
 * Licensed under Apache License v2.0 with Runtime Library Exception
 *
 * See https://swift.org/LICENSE.txt for license information
 * See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 *
 *===----------------------------------------------------------------------===*/

#include "swift/EmbeddedPlatform.h"

#define SWIFT_EMBEDDED_PLATFORM_DYNAMIC_TLS_KEY_COUNT 16
#define SWIFT_EMBEDDED_PLATFORM_TLS_KEY_COUNT \
  (__SWIFT_TLS_KEY_COUNT + SWIFT_EMBEDDED_PLATFORM_DYNAMIC_TLS_KEY_COUNT)

static void *TLS[SWIFT_EMBEDDED_PLATFORM_TLS_KEY_COUNT];
static __swift_tls_dtor_t TLSDtors[SWIFT_EMBEDDED_PLATFORM_TLS_KEY_COUNT];
static __swift_tls_key_t NextDynamicTLSKey = __SWIFT_TLS_KEY_COUNT;

void _swift_mutex_init(__swift_mutex_t *mutex, int checked) {
  (void)checked;
  if (mutex)
    *mutex = 0;
}

void _swift_mutex_destroy(__swift_mutex_t *mutex) {
  (void)mutex;
}

void _swift_mutex_lock(__swift_mutex_t *mutex) {
  (void)mutex;
}

void _swift_mutex_unlock(__swift_mutex_t *mutex) {
  (void)mutex;
}

int _swift_mutex_tryLock(__swift_mutex_t *mutex) {
  (void)mutex;
  return 1;
}

void _swift_mutex_unsafeLock(__swift_mutex_t *mutex) {
  _swift_mutex_lock(mutex);
}

void _swift_mutex_unsafeUnlock(__swift_mutex_t *mutex) {
  _swift_mutex_unlock(mutex);
}

void _swift_recursive_mutex_init(
    __swift_recursive_mutex_t *mutex, int checked) {
  _swift_mutex_init((__swift_mutex_t *)mutex, checked);
}

void _swift_recursive_mutex_destroy(__swift_recursive_mutex_t *mutex) {
  _swift_mutex_destroy((__swift_mutex_t *)mutex);
}

void _swift_recursive_mutex_lock(__swift_recursive_mutex_t *mutex) {
  _swift_mutex_lock((__swift_mutex_t *)mutex);
}

void _swift_recursive_mutex_unlock(__swift_recursive_mutex_t *mutex) {
  _swift_mutex_unlock((__swift_mutex_t *)mutex);
}

void _swift_condition_init(__swift_condition_t *condition) {
  if (condition)
    *condition = 0;
}

void _swift_condition_destroy(__swift_condition_t *condition) {
  (void)condition;
}

void _swift_condition_lock(__swift_condition_t *condition) {
  (void)condition;
}

void _swift_condition_unlock(__swift_condition_t *condition) {
  (void)condition;
}

void _swift_condition_signal(__swift_condition_t *condition) {
  (void)condition;
}

void _swift_condition_broadcast(__swift_condition_t *condition) {
  (void)condition;
}

void _swift_condition_wait(__swift_condition_t *condition) {
  (void)condition;
}

int _swift_condition_waitFor(__swift_condition_t *condition,
                             uint64_t nanoseconds) {
  (void)condition;
  (void)nanoseconds;
  return 1;
}

int _swift_condition_waitUntil(__swift_condition_t *condition,
                               int64_t epochNanoseconds) {
  (void)condition;
  (void)epochNanoseconds;
  return 1;
}

void _swift_once(__swift_once_t *predicate, void (*function)(void *),
                 void *context) {
  if (!predicate || *predicate)
    return;

  *predicate = 1;
  function(context);
}

int _swift_tls_init(__swift_tls_key_t key, __swift_tls_dtor_t destructor) {
  if (key >= SWIFT_EMBEDDED_PLATFORM_TLS_KEY_COUNT)
    return 0;
  TLSDtors[key] = destructor;
  return 1;
}

int _swift_tls_alloc(__swift_tls_key_t *key, __swift_tls_dtor_t destructor) {
  if (!key || NextDynamicTLSKey >= SWIFT_EMBEDDED_PLATFORM_TLS_KEY_COUNT)
    return 0;
  *key = NextDynamicTLSKey++;
  TLSDtors[*key] = destructor;
  return 1;
}

void *_swift_tls_get(__swift_tls_key_t key) {
  if (key >= SWIFT_EMBEDDED_PLATFORM_TLS_KEY_COUNT)
    return 0;
  return TLS[key];
}

void _swift_tls_set(__swift_tls_key_t key, void *value) {
  if (key >= SWIFT_EMBEDDED_PLATFORM_TLS_KEY_COUNT)
    return;
  TLS[key] = value;
}

__swift_thread_id_t _swift_thread_getCurrentId(void) {
  return 0;
}

int _swift_thread_isMain(void) {
  return 1;
}

int _swift_thread_getCurrentStackBounds(void **low, void **high) {
  if (low)
    *low = 0;
  if (high)
    *high = 0;
  return 0;
}

void _swift_exit(int code) {
  (void)code;
#if defined(__GNUC__) || defined(__clang__)
  __builtin_trap();
#else
  while (1)
    ;
#endif
}
