/*===------------ Embedded Swift Platform Threading ABI ---------*- C -*-===*
 *
 * This source file is part of the Swift.org open source project
 *
 * Copyright (c) 2026 Apple Inc. and the Swift project authors
 * Licensed under Apache License v2.0 with Runtime Library Exception
 *
 * See https: *swift.org/LICENSE.txt for license information
 * See https: *swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 *
 *===----------------------------------------------------------------------=== *
 *
 * This file describes the threading, synchronization, once, and TLS functions
 * that an Embedded Swift platform can provide.
 *
 *===----------------------------------------------------------------------===*/

#ifndef EMBEDDED_SWIFT_PLATFORM_THREADING_H
#define EMBEDDED_SWIFT_PLATFORM_THREADING_H

#include <stdint.h>

#ifndef EMBEDDED_SWIFT_NONNULL
#if defined(__has_feature) && __has_feature(nullability)
#define EMBEDDED_SWIFT_NONNULL _Nonnull
#define EMBEDDED_SWIFT_NULLABLE _Nullable
#else
#define EMBEDDED_SWIFT_NONNULL
#define EMBEDDED_SWIFT_NULLABLE
#endif
#define EMBEDDED_SWIFT_THREADING_DEFINED_NULLABILITY
#endif

typedef uintptr_t __swift_uintptr_t;
typedef uint64_t __swift_uint64_t;
typedef int64_t __swift_int64_t;

typedef __swift_uintptr_t __swift_mutex_t;
typedef __swift_uintptr_t __swift_lazy_mutex_t;
typedef __swift_uintptr_t __swift_recursive_mutex_t;
typedef __swift_uintptr_t __swift_condition_t;
typedef __swift_uintptr_t __swift_once_t;
typedef __swift_uintptr_t __swift_tls_key_t;
typedef __swift_uintptr_t __swift_thread_id_t;

/**
 * Number of reserved TLS keys used by Embedded Swift runtime components.
 *
 * The numeric values are kept in sync with the reserved keys in
 * swift/Threading/TLSKeys.h.
 */
#define __SWIFT_TLS_KEY_COUNT 8

typedef void (*__swift_tls_dtor_t)(void * EMBEDDED_SWIFT_NULLABLE);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initializes a non-recursive mutex.
 *
 * Parameters:
 *   - `mutex`: platform-owned mutex storage. The value is initialized by this
 *     function and later passed to the other `_swift_mutex_*` functions.
 *   - `checked`: nonzero if the platform should diagnose mutex misuse when it
 *     can do so cheaply.
 *
 * [REQUIRED] This function is required when using Embedded Swift facilities that can be
 * accessed concurrently, including multicore Concurrency and Synchronization.
 */
void _swift_mutex_init(__swift_mutex_t * EMBEDDED_SWIFT_NONNULL mutex,
                       int checked);

/**
 * [REQUIRED] Destroys a mutex initialized by `_swift_mutex_init`.
 */
void _swift_mutex_destroy(__swift_mutex_t * EMBEDDED_SWIFT_NONNULL mutex);

/**
 * [REQUIRED] Acquires a non-recursive mutex, blocking or spinning until ownership is
 * obtained.
 */
void _swift_mutex_lock(__swift_mutex_t * EMBEDDED_SWIFT_NONNULL mutex);

/**
 * [REQUIRED] Releases a non-recursive mutex held by the current execution context.
 */
void _swift_mutex_unlock(__swift_mutex_t * EMBEDDED_SWIFT_NONNULL mutex);

/**
 * Attempts to acquire a non-recursive mutex without blocking.
 *
 * [REQUIRED] Returns nonzero if the mutex was acquired, or zero if it was not acquired.
 */
int _swift_mutex_tryLock(__swift_mutex_t * EMBEDDED_SWIFT_NONNULL mutex);

/**
 * [OPTIONAL] Acquires a mutex from error paths. Defaults may call
 * `_swift_mutex_lock`.
 */
void _swift_mutex_unsafeLock(__swift_mutex_t * EMBEDDED_SWIFT_NONNULL mutex);

/**
 * [OPTIONAL] Releases a mutex from error paths. Defaults may call
 * `_swift_mutex_unlock`.
 */
void _swift_mutex_unsafeUnlock(__swift_mutex_t * EMBEDDED_SWIFT_NONNULL mutex);

/**
 * [OPTIONAL] Destroys a lazily initialized mutex. Defaults may no-op when lazy
 * mutexes do not allocate resources.
 */
void _swift_lazy_mutex_destroy(
    __swift_lazy_mutex_t * EMBEDDED_SWIFT_NONNULL mutex);

/**
 * [OPTIONAL] Acquires a lazily initialized mutex. Defaults may use the base
 * mutex behavior.
 */
void _swift_lazy_mutex_lock(
    __swift_lazy_mutex_t * EMBEDDED_SWIFT_NONNULL mutex);

/**
 * [OPTIONAL] Releases a lazily initialized mutex. Defaults may use the base
 * mutex behavior.
 */
void _swift_lazy_mutex_unlock(
    __swift_lazy_mutex_t * EMBEDDED_SWIFT_NONNULL mutex);

/**
 * [OPTIONAL] Attempts to acquire a lazily initialized mutex without blocking.
 * Defaults may use the base mutex behavior.
 */
int _swift_lazy_mutex_tryLock(
    __swift_lazy_mutex_t * EMBEDDED_SWIFT_NONNULL mutex);

/**
 * [OPTIONAL] Acquires a lazy mutex from error paths. Defaults may call
 * `_swift_lazy_mutex_lock`.
 */
void _swift_lazy_mutex_unsafeLock(
    __swift_lazy_mutex_t * EMBEDDED_SWIFT_NONNULL mutex);

/**
 * [OPTIONAL] Releases a lazy mutex from error paths. Defaults may call
 * `_swift_lazy_mutex_unlock`.
 */
void _swift_lazy_mutex_unsafeUnlock(
    __swift_lazy_mutex_t * EMBEDDED_SWIFT_NONNULL mutex);

/**
 * [OPTIONAL] Initializes a recursive mutex. Defaults may map this to the base
 * mutex behavior for single-threaded configurations.
 */
void _swift_recursive_mutex_init(
    __swift_recursive_mutex_t * EMBEDDED_SWIFT_NONNULL mutex,
    int checked);

/**
 * [OPTIONAL] Destroys a recursive mutex initialized by
 * `_swift_recursive_mutex_init`.
 */
void _swift_recursive_mutex_destroy(
    __swift_recursive_mutex_t * EMBEDDED_SWIFT_NONNULL mutex);

/**
 * [OPTIONAL] Acquires a recursive mutex.
 */
void _swift_recursive_mutex_lock(
    __swift_recursive_mutex_t * EMBEDDED_SWIFT_NONNULL mutex);

/**
 * [OPTIONAL] Releases a recursive mutex.
 */
void _swift_recursive_mutex_unlock(
    __swift_recursive_mutex_t * EMBEDDED_SWIFT_NONNULL mutex);

/**
 * [REQUIRED] Initializes a condition variable with an associated lock.
 */
void _swift_condition_init(
    __swift_condition_t * EMBEDDED_SWIFT_NONNULL condition);

/**
 * [REQUIRED] Destroys a condition variable initialized by `_swift_condition_init`.
 */
void _swift_condition_destroy(
    __swift_condition_t * EMBEDDED_SWIFT_NONNULL condition);

/**
 * [REQUIRED] Acquires the lock associated with a condition variable.
 */
void _swift_condition_lock(
    __swift_condition_t * EMBEDDED_SWIFT_NONNULL condition);

/**
 * [REQUIRED] Releases the lock associated with a condition variable.
 */
void _swift_condition_unlock(
    __swift_condition_t * EMBEDDED_SWIFT_NONNULL condition);

/**
 * [REQUIRED] Wakes one execution context waiting on a condition variable.
 */
void _swift_condition_signal(
    __swift_condition_t * EMBEDDED_SWIFT_NONNULL condition);

/**
 * [REQUIRED] Wakes every execution context waiting on a condition variable.
 */
void _swift_condition_broadcast(
    __swift_condition_t * EMBEDDED_SWIFT_NONNULL condition);

/**
 * [REQUIRED] Releases the associated lock and waits until signaled. The associated lock
 * must be held on entry and must be held again before this function returns.
 */
void _swift_condition_wait(
    __swift_condition_t * EMBEDDED_SWIFT_NONNULL condition);

/**
 * [REQUIRED] Releases the associated lock and waits until signaled or until `nanoseconds`
 * elapsed. Returns nonzero if the wait ended before the timeout elapsed.
 */
int _swift_condition_waitFor(
    __swift_condition_t * EMBEDDED_SWIFT_NONNULL condition,
    __swift_uint64_t nanoseconds);

/**
 * [REQUIRED] Releases the associated lock and waits until signaled or until the deadline,
 * expressed as nanoseconds since the platform's system-clock epoch. Returns
 * nonzero if the wait ended before the deadline elapsed.
 */
int _swift_condition_waitUntil(
    __swift_condition_t * EMBEDDED_SWIFT_NONNULL condition,
    __swift_int64_t epochNanoseconds);

/**
 * Runs `function(context)` exactly once for a statically allocated predicate.
 *
 * [REQUIRED] This hook is used by the embedded threading implementation. It is distinct
 * from the compiler/runtime `swift_once` entry point.
 */
void _swift_once(__swift_once_t * EMBEDDED_SWIFT_NONNULL predicate,
                 void (* EMBEDDED_SWIFT_NONNULL function)(
                     void * EMBEDDED_SWIFT_NULLABLE),
                 void * EMBEDDED_SWIFT_NULLABLE context);

/**
 * Initializes a reserved TLS key. `key` is one of the numeric reserved keys
 * described by `__SWIFT_TLS_KEY_COUNT`. `destructor` may be NULL.
 *
 * [REQUIRED] Returns nonzero if the platform supports the requested key.
 */
int _swift_tls_init(__swift_tls_key_t key,
                    __swift_tls_dtor_t EMBEDDED_SWIFT_NULLABLE destructor);

/**
 * Allocates a dynamic TLS key and writes it into `key`.
 *
 * [REQUIRED] Returns nonzero if the key was allocated, or zero on failure.
 */
int _swift_tls_alloc(
    __swift_tls_key_t * EMBEDDED_SWIFT_NONNULL key,
    __swift_tls_dtor_t EMBEDDED_SWIFT_NULLABLE destructor);

/**
 * [REQUIRED] Returns the value stored for a reserved TLS key in the current
 * execution context, or NULL if no value has been stored.
 */
void * EMBEDDED_SWIFT_NULLABLE _swift_tls_get(__swift_tls_key_t key);

/**
 * [REQUIRED] Stores a value for a reserved TLS key in the current execution context.
 */
void _swift_tls_set(__swift_tls_key_t key,
                    void * EMBEDDED_SWIFT_NULLABLE value);

/**
 * [REQUIRED] Returns an identifier for the current execution context. Identifiers only
 * need to compare equal for the same context during its lifetime.
 */
__swift_thread_id_t _swift_thread_getCurrentId(void);

/**
 * [REQUIRED] Returns nonzero when the current execution context is the platform's main
 * execution context.
 */
int _swift_thread_isMain(void);

/**
 * Writes the current execution context's stack bounds into `low` and `high`.
 *
 * [OPTIONAL] Returns nonzero if bounds were written, or zero if stack bounds are
 * not available on this platform.
 */
int _swift_thread_getCurrentStackBounds(
    void * EMBEDDED_SWIFT_NULLABLE * EMBEDDED_SWIFT_NONNULL low,
    void * EMBEDDED_SWIFT_NULLABLE * EMBEDDED_SWIFT_NONNULL high);

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef EMBEDDED_SWIFT_THREADING_DEFINED_NULLABILITY
#undef EMBEDDED_SWIFT_NULLABLE
#undef EMBEDDED_SWIFT_NONNULL
#undef EMBEDDED_SWIFT_THREADING_DEFINED_NULLABILITY
#endif

#endif
