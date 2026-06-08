/*===------------------ Embedded Swift Platform Declarations ------*- C -*-===*
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
 * This file provides a description of the functions that a platform is expected
 * to provide to work with Embedded Swift. A "platform" in this case is broadly
 * construed, and can be anything from a couple of function implementations
 * within a standalone Embedded Swift program to a full-fledged SDK for an
 * OS. Either way, any of these functions can be implemented either in Swift
 * (using `@c`) or in C and linked into the final Embedded Swift executable.
 *
 * Many of these functions precisely match the signatures of functions defined
 * by POSIX. This is deliberate, because it allows simple pass-through
 * implementations for POSIX systems.
 *
 *===----------------------------------------------------------------------===*/

#ifndef EMBEDDED_SWIFT_PLATFORM_H
#define EMBEDDED_SWIFT_PLATFORM_H

#ifdef __SIZE_TYPE__
typedef __SIZE_TYPE__ __swift_size_t;
#else
#include <stddef.h>
typedef size_t __swift_size_t;
#endif

#ifdef __UINTPTR_TYPE__
typedef __UINTPTR_TYPE__ __swift_uintptr_t;
#else
#include <stdint.h>
typedef uintptr_t __swift_uintptr_t;
#endif

#ifdef __UINT64_TYPE__
typedef __UINT64_TYPE__ __swift_uint64_t;
#else
#include <stdint.h>
typedef uint64_t __swift_uint64_t;
#endif

#ifdef __INT64_TYPE__
typedef __INT64_TYPE__ __swift_int64_t;
#else
#include <stdint.h>
typedef int64_t __swift_int64_t;
#endif

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

#if defined(__has_feature) && __has_feature(nullability)
#define EMBEDDED_SWIFT_NONNULL _Nonnull
#define EMBEDDED_SWIFT_NULLABLE _Nullable
#else
#define EMBEDDED_SWIFT_NONNULL
#define EMBEDDED_SWIFT_NULLABLE
#endif

typedef void (*__swift_tls_dtor_t)(void * EMBEDDED_SWIFT_NULLABLE);

#if defined(__has_feature) && (__has_feature(bounds_attributes) || __has_feature(bounds_safety_attributes))
#define EMBEDDED_SWIFT_COUNTED_BY(N) __attribute__((__counted_by__(N)))
#define EMBEDDED_SWIFT_SIZED_BY(N) __attribute__((__sized_by__(N)))
#define EMBEDDED_SWIFT_SINGLE __attribute__((__single__))
#else
#define EMBEDDED_SWIFT_COUNTED_BY(N)
#define EMBEDDED_SWIFT_SIZED_BY(N)
#define EMBEDDED_SWIFT_SINGLE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocates memory and places the resulting pointer in `*memptr`.
 *
 * Parameters:
 *   - `memptr`: the resulting pointer will be written into *memptr on success.
 *   - `size`: the minimum number of bytes to allocate.
 *   - `alignment`: the minimum alignment of the resulting pointer, which must
 *     be a power of at least as large as `sizeof(void *)`.
 *
 * Returns 0 on success, any other value on failure.
 *
 * [REQUIRED] This function is required when using any Embedded Swift facility that
 * requires memory allocation from the heap, whether explicitly (e.g., via the
 * `allocate` operation on unsafe pointers) or implicitly (e.g., creating a
 * copy-on-write array or an instance of a class type).
 * 
 * This function can be implemented as a direct call to `posix_memalign`.
 */
int _swift_alignedAllocate(void * EMBEDDED_SWIFT_NULLABLE * EMBEDDED_SWIFT_NONNULL EMBEDDED_SWIFT_SINGLE memptr, __swift_size_t alignment, __swift_size_t size);

/**
 * Frees the memory referenced by `ptr`.
 *
 * Parameters:
 *   - `ptr`: The pointer to be freed. If it is NULL, the operation does
 *     nothing.
 *   - `size`: the number of allocated bytes, which may be -1 if it is not
 *     known.
 *   - `alignment`: the minimum alignment of the resulting pointer, which must
 *     be a power of at least as large as `sizeof(void *)`, or be zero to
 *     indicate that the alignment is not known.
 *
 * [REQUIRED] This function is required when using any Embedded Swift facility that
 * requires memory allocation from the heap, whether explicitly (e.g., via the
 * `allocate` operation on unsafe pointers) or implicitly (e.g., creating a
 * copy-on-write array or an instance of a class type).
 * 
 * This function can be implemented as a direct call to `free`.
 */
void _swift_alignedFree(void * EMBEDDED_SWIFT_NONNULL ptr, __swift_size_t alignment, __swift_size_t size);

/**
 * Allocates memory and places the resulting pointer in `*memptr`.
 *
 * Parameters:
 *   - `memptr`: the resulting pointer will be written into *memptr on success.
 *   - `size`: the minimum number of bytes to allocate.
 *   - `alignment`: the minimum alignment of the resulting pointer, which must
 *     be a power of at least as large as `sizeof(void *)`.
 *   - `typeId`: an identifier used by a typed allocator to e.g. place the
 *     allocation in a particular bucket.
 *
 * [REQUIRED] This function is required when using any Embedded Swift facility that
 * requires typed memory allocation from the heap, e.g. class instance
 * allocations when the TypedAllocation feature is enabled.
 *
 * This function can be implemented as a direct call to `posix_memalign`,
 * if the target platform does not support typed allocations.
 */
void _swift_typedAllocate(
    void *EMBEDDED_SWIFT_NULLABLE *EMBEDDED_SWIFT_NONNULL EMBEDDED_SWIFT_SINGLE ptr,
    __swift_size_t size, __swift_size_t alignment, unsigned long long typeId);

/**
 * Writes a sequence of UTF-8 code points to standard output.
 *
 * Parameters:
 *   - `chars`: the UTF-8 code points to standard output. It is not
 *     NULL-terminated.
 *   - `count`: the number of UTF-8 code points.
 *
 * Returns the number of characters that were written.
 *
 * [REQUIRED] This function is required when using the Embedded Swift print() facilities.
 *
 * This function can be implemented as a call to fwrite or printf with the
 * specified number of code points.
 */
int _swift_writeToStandardOutput(
    const unsigned char * EMBEDDED_SWIFT_NULLABLE EMBEDDED_SWIFT_COUNTED_BY(count) chars,
    __swift_size_t count);

/**
 * Generates random bytes into the given buffer.
 *
 * Parameters:
 *   - `buffer`: the buffer into which the random bytes should be generated.
 *   - `nbytes`: the number of bytes that should be generated into the buffer.
 *
 * [REQUIRED] This function is required when using Swift's SystemRandomNumberGenerator, the
 * default random number generator used for shuffling elements and producing
 * random values. While this function is encouraged to use a cryptographically
 * secure algorithm, it is not required to do so.
 *
 * Note that this function is not used to provide random seeding for the hash
 * functions used in Set and Dictionary. Those operations use
 * `_swift_generateRandomHashSeed`.
 *
 * This function can be implemented as a direct call to `arc4random_buf`.
 */
void _swift_generateRandom(void * EMBEDDED_SWIFT_NONNULL EMBEDDED_SWIFT_SIZED_BY(nbytes) buffer, __swift_size_t nbytes);

/**
 * Generates random bytes intended for a hashing seed into the given buffer.
 *
 * Parameters:
 *   - `buffer`: the buffer into which the random bytes should be generated.
 *   - `nbytes`: the number of bytes that should be generated into the buffer.
 *
 * [REQUIRED] This function is required when using Swift's hashed collections, such as Set
 * and Dictionary, to provide random seeding for the hash functions. Random
 * seeding makes hash values differ from one execute to the next, mitigating
 * against denial-of-service attacks that target a known hash function. The
 * random number generator provided here need not be cryptographically
 * secure. An implementation may choose to provide constant values random than a
 * random seed to make hashing deterministic.
 *
 * This function can be implemented as a direct call to `arc4random_buf`.
 */
void _swift_generateRandomHashSeed(void * EMBEDDED_SWIFT_NONNULL EMBEDDED_SWIFT_SIZED_BY(nbytes) buffer, __swift_size_t nbytes);

/**
 * Retrieve a pointer that will be used to retain information needed for Swift's
 * dynamic exclusivity checking.
 *
 * Returns the pointer most recently passed to `_swift_setExclusivityTLS` on
 * this thread. If `_swift_setExclusivityTLS` has not been called on this
 * thread, returns NULL.
 *
 * In a single-threaded environment, the `_swift_getExclusivityTLS` and
 * `_swift_setExclusivityTLS` functions can get and set a global variable that
 * is initialized to NULL. In a multi-threaded environment, the variable will
 * need to be in thread-local storage (e.g., using C11 `_Thread_local`) or a
 * similar facility.
 *
 * [REQUIRED] This function is required when using Swift's dynamic exclusivity checking,
 * which is enabled by the Swift compiler option `-enforce-exclusivity=checked`
 * and required when the compiler cannot statically prove that all accesses to a
 * given variable (such as a global variable or a stored instance property of a
 * class) respect the exclusivity model.
 */
void * EMBEDDED_SWIFT_NULLABLE _swift_getExclusivityTLS(void);

/**
 * Set the pointer that will be used to retain information needed for Swift's
 * dynamic exclusivity checking.
 *
 * Parameters:
 *   - `ptr`: the pointer to set. A subsequent call to
 *     `_swift_getExclusivityTLS` on the same thread (without an intervening
 *     call to `_swift_setExclusivityTLS`) shall return `ptr`.
 *
 * [REQUIRED] See `_swift_getExclusivityTLS` for more information about dynamic
 * exclusivity checking.
 */
void _swift_setExclusivityTLS(void * EMBEDDED_SWIFT_NULLABLE ptr);

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

/**
 * Exit the program.
 *
 * Parameters:
 * - `code`: the exit code, which is typically 0 for normal termination.
 *
 * This function must not return.
 *
 * [REQUIRED] This function can be implemented directly with a call to the POSIX exit()
 * function.
 */
void _swift_exit(int code);

#ifdef __cplusplus
} // extern "C"
#endif

#undef EMBEDDED_SWIFT_SINGLE
#undef EMBEDDED_SWIFT_SIZED_BY
#undef EMBEDDED_SWIFT_COUNTED_BY
#undef EMBEDDED_SWIFT_NULLABLE
#undef EMBEDDED_SWIFT_NONNULL

#endif
