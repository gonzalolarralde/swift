/*===--------------- EmbeddedPlatformPthreads.c --------------------------===*
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

#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include "swift/EmbeddedPlatform.h"

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#if defined(__APPLE__)
#include <pthread/pthread.h>
#endif

#define SWIFT_EMBEDDED_PLATFORM_DYNAMIC_TLS_KEY_COUNT 16
#define SWIFT_EMBEDDED_PLATFORM_TLS_KEY_COUNT                                  \
  (__SWIFT_TLS_KEY_COUNT + SWIFT_EMBEDDED_PLATFORM_DYNAMIC_TLS_KEY_COUNT)

typedef struct {
  pthread_cond_t condition;
  pthread_mutex_t mutex;
} SwiftPthreadCondition;

static pthread_mutex_t OnceMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t OnceCondition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t TLSMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_key_t TLSKeys[SWIFT_EMBEDDED_PLATFORM_TLS_KEY_COUNT];
static int TLSKeyInitialized[SWIFT_EMBEDDED_PLATFORM_TLS_KEY_COUNT];
static __swift_tls_key_t NextDynamicTLSKey = __SWIFT_TLS_KEY_COUNT;
static pthread_key_t ExclusivityKey;
static pthread_once_t ExclusivityKeyOnce = PTHREAD_ONCE_INIT;
static pthread_t MainThread;

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noreturn))
#endif
static void swift_embedded_platform_trap(void) {
#if defined(__GNUC__) || defined(__clang__)
  __builtin_trap();
#else
  abort();
#endif
}

static void swift_embedded_platform_check(int result) {
  if (result != 0)
    swift_embedded_platform_trap();
}

static void *swift_embedded_platform_malloc(__swift_size_t size) {
  void *result = malloc(size);
  if (!result)
    swift_embedded_platform_trap();
  return result;
}

static pthread_mutex_t *swift_embedded_platform_mutex(__swift_mutex_t *mutex) {
  return (pthread_mutex_t *)(uintptr_t)*mutex;
}

static pthread_mutex_t *
swift_embedded_platform_recursive_mutex(__swift_recursive_mutex_t *mutex) {
  return (pthread_mutex_t *)(uintptr_t)*mutex;
}

static SwiftPthreadCondition *
swift_embedded_platform_condition(__swift_condition_t *condition) {
  return (SwiftPthreadCondition *)(uintptr_t)*condition;
}

static struct timespec
swift_embedded_platform_timespec_from_ns(int64_t nanoseconds) {
  struct timespec result;
  result.tv_sec = (time_t)(nanoseconds / 1000000000);
  result.tv_nsec = (long)(nanoseconds % 1000000000);
  if (result.tv_nsec < 0) {
    result.tv_sec -= 1;
    result.tv_nsec += 1000000000;
  }
  return result;
}

static struct timespec
swift_embedded_platform_now_plus_ns(uint64_t nanoseconds) {
  struct timespec result;
  swift_embedded_platform_check(clock_gettime(CLOCK_REALTIME, &result));

  result.tv_sec += (time_t)(nanoseconds / 1000000000);
  result.tv_nsec += (long)(nanoseconds % 1000000000);
  if (result.tv_nsec >= 1000000000) {
    result.tv_sec += 1;
    result.tv_nsec -= 1000000000;
  }
  return result;
}

__attribute__((constructor)) static void
swift_embedded_platform_init_main_thread(void) {
  MainThread = pthread_self();
}

static void swift_embedded_platform_init_exclusivity_key(void) {
  swift_embedded_platform_check(pthread_key_create(&ExclusivityKey, 0));
}

void *_swift_getExclusivityTLS(void) {
  swift_embedded_platform_check(pthread_once(
      &ExclusivityKeyOnce, swift_embedded_platform_init_exclusivity_key));
  return pthread_getspecific(ExclusivityKey);
}

void _swift_setExclusivityTLS(void *ptr) {
  swift_embedded_platform_check(pthread_once(
      &ExclusivityKeyOnce, swift_embedded_platform_init_exclusivity_key));
  swift_embedded_platform_check(pthread_setspecific(ExclusivityKey, ptr));
}

void _swift_mutex_init(__swift_mutex_t *mutex, int checked) {
  pthread_mutex_t *pthreadMutex =
      swift_embedded_platform_malloc(sizeof(pthread_mutex_t));

  if (checked) {
    pthread_mutexattr_t attr;
    swift_embedded_platform_check(pthread_mutexattr_init(&attr));
    swift_embedded_platform_check(
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK));
    swift_embedded_platform_check(pthread_mutex_init(pthreadMutex, &attr));
    swift_embedded_platform_check(pthread_mutexattr_destroy(&attr));
  } else {
    swift_embedded_platform_check(pthread_mutex_init(pthreadMutex, 0));
  }

  *mutex = (uintptr_t)pthreadMutex;
}

void _swift_mutex_destroy(__swift_mutex_t *mutex) {
  pthread_mutex_t *pthreadMutex = swift_embedded_platform_mutex(mutex);
  swift_embedded_platform_check(pthread_mutex_destroy(pthreadMutex));
  free(pthreadMutex);
  *mutex = 0;
}

void _swift_mutex_lock(__swift_mutex_t *mutex) {
  swift_embedded_platform_check(
      pthread_mutex_lock(swift_embedded_platform_mutex(mutex)));
}

void _swift_mutex_unlock(__swift_mutex_t *mutex) {
  swift_embedded_platform_check(
      pthread_mutex_unlock(swift_embedded_platform_mutex(mutex)));
}

int _swift_mutex_tryLock(__swift_mutex_t *mutex) {
  int result = pthread_mutex_trylock(swift_embedded_platform_mutex(mutex));
  if (result == 0)
    return 1;
  if (result == EBUSY)
    return 0;
  swift_embedded_platform_trap();
}

void _swift_mutex_unsafeLock(__swift_mutex_t *mutex) {
  (void)pthread_mutex_lock(swift_embedded_platform_mutex(mutex));
}

void _swift_mutex_unsafeUnlock(__swift_mutex_t *mutex) {
  (void)pthread_mutex_unlock(swift_embedded_platform_mutex(mutex));
}

void _swift_recursive_mutex_init(__swift_recursive_mutex_t *mutex,
                                 int checked) {
  (void)checked;
  pthread_mutex_t *pthreadMutex =
      swift_embedded_platform_malloc(sizeof(pthread_mutex_t));
  pthread_mutexattr_t attr;
  swift_embedded_platform_check(pthread_mutexattr_init(&attr));
  swift_embedded_platform_check(
      pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE));
  swift_embedded_platform_check(pthread_mutex_init(pthreadMutex, &attr));
  swift_embedded_platform_check(pthread_mutexattr_destroy(&attr));
  *mutex = (uintptr_t)pthreadMutex;
}

void _swift_recursive_mutex_destroy(__swift_recursive_mutex_t *mutex) {
  pthread_mutex_t *pthreadMutex =
      swift_embedded_platform_recursive_mutex(mutex);
  swift_embedded_platform_check(pthread_mutex_destroy(pthreadMutex));
  free(pthreadMutex);
  *mutex = 0;
}

void _swift_recursive_mutex_lock(__swift_recursive_mutex_t *mutex) {
  swift_embedded_platform_check(
      pthread_mutex_lock(swift_embedded_platform_recursive_mutex(mutex)));
}

void _swift_recursive_mutex_unlock(__swift_recursive_mutex_t *mutex) {
  swift_embedded_platform_check(
      pthread_mutex_unlock(swift_embedded_platform_recursive_mutex(mutex)));
}

void _swift_condition_init(__swift_condition_t *condition) {
  SwiftPthreadCondition *pthreadCondition =
      swift_embedded_platform_malloc(sizeof(SwiftPthreadCondition));
  swift_embedded_platform_check(
      pthread_cond_init(&pthreadCondition->condition, 0));
  swift_embedded_platform_check(
      pthread_mutex_init(&pthreadCondition->mutex, 0));
  *condition = (uintptr_t)pthreadCondition;
}

void _swift_condition_destroy(__swift_condition_t *condition) {
  SwiftPthreadCondition *pthreadCondition =
      swift_embedded_platform_condition(condition);
  swift_embedded_platform_check(
      pthread_cond_destroy(&pthreadCondition->condition));
  swift_embedded_platform_check(
      pthread_mutex_destroy(&pthreadCondition->mutex));
  free(pthreadCondition);
  *condition = 0;
}

void _swift_condition_lock(__swift_condition_t *condition) {
  swift_embedded_platform_check(
      pthread_mutex_lock(&swift_embedded_platform_condition(condition)->mutex));
}

void _swift_condition_unlock(__swift_condition_t *condition) {
  swift_embedded_platform_check(pthread_mutex_unlock(
      &swift_embedded_platform_condition(condition)->mutex));
}

void _swift_condition_signal(__swift_condition_t *condition) {
  swift_embedded_platform_check(pthread_cond_signal(
      &swift_embedded_platform_condition(condition)->condition));
}

void _swift_condition_broadcast(__swift_condition_t *condition) {
  swift_embedded_platform_check(pthread_cond_broadcast(
      &swift_embedded_platform_condition(condition)->condition));
}

void _swift_condition_wait(__swift_condition_t *condition) {
  SwiftPthreadCondition *pthreadCondition =
      swift_embedded_platform_condition(condition);
  swift_embedded_platform_check(pthread_cond_wait(&pthreadCondition->condition,
                                                  &pthreadCondition->mutex));
}

int _swift_condition_waitFor(__swift_condition_t *condition,
                             uint64_t nanoseconds) {
  SwiftPthreadCondition *pthreadCondition =
      swift_embedded_platform_condition(condition);
  struct timespec deadline = swift_embedded_platform_now_plus_ns(nanoseconds);
  int result = pthread_cond_timedwait(&pthreadCondition->condition,
                                      &pthreadCondition->mutex, &deadline);
  if (result == 0)
    return 1;
  if (result == ETIMEDOUT)
    return 0;
  swift_embedded_platform_trap();
}

int _swift_condition_waitUntil(__swift_condition_t *condition,
                               int64_t epochNanoseconds) {
  SwiftPthreadCondition *pthreadCondition =
      swift_embedded_platform_condition(condition);
  struct timespec deadline =
      swift_embedded_platform_timespec_from_ns(epochNanoseconds);
  int result = pthread_cond_timedwait(&pthreadCondition->condition,
                                      &pthreadCondition->mutex, &deadline);
  if (result == 0)
    return 1;
  if (result == ETIMEDOUT)
    return 0;
  swift_embedded_platform_trap();
}

void _swift_once(__swift_once_t *predicate, void (*function)(void *),
                 void *context) {
  swift_embedded_platform_check(pthread_mutex_lock(&OnceMutex));
  while (*predicate == 1)
    swift_embedded_platform_check(
        pthread_cond_wait(&OnceCondition, &OnceMutex));

  if (*predicate == 2) {
    swift_embedded_platform_check(pthread_mutex_unlock(&OnceMutex));
    return;
  }

  *predicate = 1;
  swift_embedded_platform_check(pthread_mutex_unlock(&OnceMutex));

  function(context);

  swift_embedded_platform_check(pthread_mutex_lock(&OnceMutex));
  *predicate = 2;
  swift_embedded_platform_check(pthread_cond_broadcast(&OnceCondition));
  swift_embedded_platform_check(pthread_mutex_unlock(&OnceMutex));
}

int _swift_tls_init(__swift_tls_key_t key, __swift_tls_dtor_t destructor) {
  if (key >= __SWIFT_TLS_KEY_COUNT)
    return 0;

  swift_embedded_platform_check(pthread_mutex_lock(&TLSMutex));
  if (!TLSKeyInitialized[key]) {
    if (pthread_key_create(&TLSKeys[key], destructor) != 0) {
      swift_embedded_platform_check(pthread_mutex_unlock(&TLSMutex));
      return 0;
    }
    TLSKeyInitialized[key] = 1;
  }
  swift_embedded_platform_check(pthread_mutex_unlock(&TLSMutex));
  return 1;
}

int _swift_tls_alloc(__swift_tls_key_t *key, __swift_tls_dtor_t destructor) {
  if (!key)
    return 0;

  swift_embedded_platform_check(pthread_mutex_lock(&TLSMutex));
  if (NextDynamicTLSKey >= SWIFT_EMBEDDED_PLATFORM_TLS_KEY_COUNT) {
    swift_embedded_platform_check(pthread_mutex_unlock(&TLSMutex));
    return 0;
  }

  __swift_tls_key_t allocatedKey = NextDynamicTLSKey++;
  if (pthread_key_create(&TLSKeys[allocatedKey], destructor) != 0) {
    NextDynamicTLSKey--;
    swift_embedded_platform_check(pthread_mutex_unlock(&TLSMutex));
    return 0;
  }
  TLSKeyInitialized[allocatedKey] = 1;
  *key = allocatedKey;
  swift_embedded_platform_check(pthread_mutex_unlock(&TLSMutex));
  return 1;
}

void *_swift_tls_get(__swift_tls_key_t key) {
  if (key >= SWIFT_EMBEDDED_PLATFORM_TLS_KEY_COUNT || !TLSKeyInitialized[key])
    return 0;
  return pthread_getspecific(TLSKeys[key]);
}

void _swift_tls_set(__swift_tls_key_t key, void *value) {
  if (key >= SWIFT_EMBEDDED_PLATFORM_TLS_KEY_COUNT || !TLSKeyInitialized[key])
    return;
  swift_embedded_platform_check(pthread_setspecific(TLSKeys[key], value));
}

__swift_thread_id_t _swift_thread_getCurrentId(void) {
  return (__swift_thread_id_t)(uintptr_t)pthread_self();
}

int _swift_thread_isMain(void) {
  return pthread_equal(pthread_self(), MainThread) != 0;
}

int _swift_thread_getCurrentStackBounds(void **low, void **high) {
#if defined(__APPLE__)
  pthread_t thread = pthread_self();
  void *stackHigh = pthread_get_stackaddr_np(thread);
  void *stackLow = (char *)stackHigh - pthread_get_stacksize_np(thread);
  if (low)
    *low = stackLow;
  if (high)
    *high = stackHigh;
  return 1;
#elif defined(__linux__)
  pthread_attr_t attr;
  void *stackLow = 0;
  size_t stackSize = 0;
  if (pthread_getattr_np(pthread_self(), &attr) != 0)
    return 0;
  if (pthread_attr_getstack(&attr, &stackLow, &stackSize) != 0) {
    (void)pthread_attr_destroy(&attr);
    return 0;
  }
  (void)pthread_attr_destroy(&attr);
  if (low)
    *low = stackLow;
  if (high)
    *high = (char *)stackLow + stackSize;
  return 1;
#else
  if (low)
    *low = 0;
  if (high)
    *high = 0;
  return 0;
#endif
}
