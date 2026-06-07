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

#if defined(__GNUC__) || defined(__clang__)
#define SWIFT_EMBEDDED_PLATFORM_WEAK __attribute__((weak))
#else
#define SWIFT_EMBEDDED_PLATFORM_WEAK
#endif

static void *ReservedTLS[__SWIFT_TLS_KEY_COUNT];
static void *exclusivityTLS = 0;

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_mutex_init(__swift_mutex_t *mutex, int checked) {
  (void)checked;
  if (mutex)
    *mutex = 0;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_mutex_destroy(__swift_mutex_t *mutex) {
  (void)mutex;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_mutex_lock(__swift_mutex_t *mutex) {
  (void)mutex;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_mutex_unlock(__swift_mutex_t *mutex) {
  (void)mutex;
}

SWIFT_EMBEDDED_PLATFORM_WEAK int _swift_mutex_tryLock(__swift_mutex_t *mutex) {
  (void)mutex;
  return 1;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_mutex_unsafeLock(__swift_mutex_t *mutex) {
  _swift_mutex_lock(mutex);
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_mutex_unsafeUnlock(__swift_mutex_t *mutex) {
  _swift_mutex_unlock(mutex);
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_lazy_mutex_destroy(
    __swift_lazy_mutex_t *mutex) {
  (void)mutex;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_lazy_mutex_lock(
    __swift_lazy_mutex_t *mutex) {
  (void)mutex;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_lazy_mutex_unlock(
    __swift_lazy_mutex_t *mutex) {
  (void)mutex;
}

SWIFT_EMBEDDED_PLATFORM_WEAK int _swift_lazy_mutex_tryLock(
    __swift_lazy_mutex_t *mutex) {
  (void)mutex;
  return 1;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_lazy_mutex_unsafeLock(
    __swift_lazy_mutex_t *mutex) {
  _swift_lazy_mutex_lock(mutex);
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_lazy_mutex_unsafeUnlock(
    __swift_lazy_mutex_t *mutex) {
  _swift_lazy_mutex_unlock(mutex);
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_recursive_mutex_init(
    __swift_recursive_mutex_t *mutex, int checked) {
  _swift_mutex_init((__swift_mutex_t *)mutex, checked);
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_recursive_mutex_destroy(
    __swift_recursive_mutex_t *mutex) {
  _swift_mutex_destroy((__swift_mutex_t *)mutex);
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_recursive_mutex_lock(
    __swift_recursive_mutex_t *mutex) {
  _swift_mutex_lock((__swift_mutex_t *)mutex);
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_recursive_mutex_unlock(
    __swift_recursive_mutex_t *mutex) {
  _swift_mutex_unlock((__swift_mutex_t *)mutex);
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_condition_init(__swift_condition_t *condition) {
  if (condition)
    *condition = 0;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_condition_destroy(__swift_condition_t *condition) {
  (void)condition;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_condition_lock(__swift_condition_t *condition) {
  (void)condition;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_condition_unlock(__swift_condition_t *condition) {
  (void)condition;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_condition_signal(__swift_condition_t *condition) {
  (void)condition;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_condition_broadcast(__swift_condition_t *condition) {
  (void)condition;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_condition_wait(__swift_condition_t *condition) {
  (void)condition;
}

SWIFT_EMBEDDED_PLATFORM_WEAK int _swift_condition_waitFor(__swift_condition_t *condition,
                             __swift_uint64_t nanoseconds) {
  (void)condition;
  (void)nanoseconds;
  return 1;
}

SWIFT_EMBEDDED_PLATFORM_WEAK int _swift_condition_waitUntil(__swift_condition_t *condition,
                               __swift_int64_t epochNanoseconds) {
  (void)condition;
  (void)epochNanoseconds;
  return 1;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_once(__swift_once_t *predicate, void (*function)(void *),
                 void *context) {
  if (!predicate || *predicate)
    return;

  *predicate = 1;
  function(context);
}

SWIFT_EMBEDDED_PLATFORM_WEAK int _swift_tls_init(__swift_tls_key_t key,
                                                 __swift_tls_dtor_t destructor) {
  (void)destructor;
  return key < __SWIFT_TLS_KEY_COUNT;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void *_swift_tls_get(__swift_tls_key_t key) {
  if (key >= __SWIFT_TLS_KEY_COUNT)
    return 0;
  return ReservedTLS[key];
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_tls_set(__swift_tls_key_t key,
                                                 void *value) {
  if (key >= __SWIFT_TLS_KEY_COUNT)
    return;
  ReservedTLS[key] = value;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void *_swift_getExclusivityTLS(void) {
  return exclusivityTLS;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_setExclusivityTLS(void *ptr) {
  exclusivityTLS = ptr;
}

SWIFT_EMBEDDED_PLATFORM_WEAK __swift_thread_id_t _swift_thread_getCurrentId(void) {
  return 0;
}

SWIFT_EMBEDDED_PLATFORM_WEAK int _swift_thread_isMain(void) {
  return 1;
}

SWIFT_EMBEDDED_PLATFORM_WEAK int _swift_thread_getCurrentStackBounds(void **low,
                                                                   void **high) {
  if (low)
    *low = 0;
  if (high)
    *high = 0;
  return 0;
}

SWIFT_EMBEDDED_PLATFORM_WEAK void _swift_exit(int code) {
  (void)code;
#if defined(__GNUC__) || defined(__clang__)
  __builtin_trap();
#else
  while (1)
    ;
#endif
}
