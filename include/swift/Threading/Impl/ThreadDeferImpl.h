//===--- ThreadDeferImpl.h - Deferred threading implementation -*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2022 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// Implements the Swift threading abstraction for embedded platforms that want
// to provide platform TLS without pulling in pthreads or C11 threads.
//
// This implementation intentionally does not define SWIFT_THREAD_LOCAL.  That
// makes ThreadLocalStorage.h use its key-based ThreadLocal wrapper instead of
// compiling reserved TLS slots as ordinary globals.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_THREADING_IMPL_THREADDEFER_H
#define SWIFT_THREADING_IMPL_THREADDEFER_H

#include <chrono>
#include <cstdint>
#include <optional>

#include "chrono_utils.h"

// Use Swift's reserved TLS key enum as stable platform key numbers.  This is
// the deferred-threading TLS mode: do not define SWIFT_THREAD_LOCAL here.
// ThreadLocalStorage.h will route SWIFT_THREAD_LOCAL_TYPE(TYPE, KEY) through
// key-backed ThreadLocal storage and call tls_get/tls_set below.
#define SWIFT_THREADING_USE_RESERVED_TLS_KEYS 1

namespace swift {
namespace threading_impl {

using thread_id = uintptr_t;

struct recursive_mutex_handle {
  uintptr_t storage[4];
};

extern "C" {

thread_id swift_threading_defer_current_thread_id();
bool swift_threading_defer_is_main_thread();
bool swift_threading_defer_current_stack_bounds(void **low, void **high);

// Expected: initialize a non-recursive mutex; checked may request misuse checks.
void swift_threading_defer_mutex_init(uintptr_t *handle, bool checked);
// Expected: destroy a mutex initialized by swift_threading_defer_mutex_init.
void swift_threading_defer_mutex_destroy(uintptr_t *handle);
// Expected: acquire a non-recursive mutex, blocking or spinning until success.
void swift_threading_defer_mutex_lock(uintptr_t *handle);
// Expected: release a non-recursive mutex held by the current context.
void swift_threading_defer_mutex_unlock(uintptr_t *handle);
// Expected: try to acquire a non-recursive mutex without blocking.
bool swift_threading_defer_mutex_try_lock(uintptr_t *handle);
// Optional: acquire from error paths; default calls mutex_lock.
void swift_threading_defer_mutex_unsafe_lock(uintptr_t *handle);
// Optional: release from error paths; default calls mutex_unlock.
void swift_threading_defer_mutex_unsafe_unlock(uintptr_t *handle);

// Optional: destroy lazy backing state; default is a no-op.
void swift_threading_defer_lazy_mutex_destroy(uintptr_t *handle);
// Optional: lazily initialize and acquire; default calls mutex_lock.
void swift_threading_defer_lazy_mutex_lock(uintptr_t *handle);
// Optional: lazily initialize and release; default calls mutex_unlock.
void swift_threading_defer_lazy_mutex_unlock(uintptr_t *handle);
// Optional: lazily initialize and try-lock; default calls mutex_try_lock.
bool swift_threading_defer_lazy_mutex_try_lock(uintptr_t *handle);
// Optional: lazily initialize and acquire from error paths; default calls mutex_lock.
void swift_threading_defer_lazy_mutex_unsafe_lock(uintptr_t *handle);
// Optional: lazily initialize and release from error paths; default calls mutex_unlock.
void swift_threading_defer_lazy_mutex_unsafe_unlock(uintptr_t *handle);

void swift_threading_defer_recursive_mutex_init(uintptr_t *storage,
                                                bool checked);
void swift_threading_defer_recursive_mutex_destroy(uintptr_t *storage);
void swift_threading_defer_recursive_mutex_lock(uintptr_t *storage);
void swift_threading_defer_recursive_mutex_unlock(uintptr_t *storage);

void swift_threading_defer_cond_init(uintptr_t *handle);
void swift_threading_defer_cond_destroy(uintptr_t *handle);
void swift_threading_defer_cond_lock(uintptr_t *handle);
void swift_threading_defer_cond_unlock(uintptr_t *handle);
void swift_threading_defer_cond_signal(uintptr_t *handle);
void swift_threading_defer_cond_broadcast(uintptr_t *handle);
void swift_threading_defer_cond_wait(uintptr_t *handle);
bool swift_threading_defer_cond_wait_for(uintptr_t *handle, uint64_t ns);
bool swift_threading_defer_cond_wait_until(uintptr_t *handle, int64_t epochNs);

void swift_threading_defer_once(uintptr_t *predicate, void (*fn)(void *),
                                void *ctx);

void *swift_threading_defer_tls_get(uintptr_t key);
void swift_threading_defer_tls_set(uintptr_t key, void *value);

} // extern "C"

// .. Thread related things ..................................................

inline thread_id thread_get_current() {
  return swift_threading_defer_current_thread_id();
}

inline bool thread_is_main() {
  return swift_threading_defer_is_main_thread();
}

inline bool threads_same(thread_id a, thread_id b) {
  return a == b;
}

inline std::optional<stack_bounds> thread_get_current_stack_bounds() {
  void *low = nullptr;
  void *high = nullptr;
  if (!swift_threading_defer_current_stack_bounds(&low, &high))
    return {};
  return stack_bounds{low, high};
}

// .. Mutex support ..........................................................

using mutex_handle = uintptr_t;

inline void mutex_init(mutex_handle &handle, bool checked = false) {
  swift_threading_defer_mutex_init(&handle, checked);
}

inline void mutex_destroy(mutex_handle &handle) {
  swift_threading_defer_mutex_destroy(&handle);
}

inline void mutex_lock(mutex_handle &handle) {
  swift_threading_defer_mutex_lock(&handle);
}

inline void mutex_unlock(mutex_handle &handle) {
  swift_threading_defer_mutex_unlock(&handle);
}

inline bool mutex_try_lock(mutex_handle &handle) {
  return swift_threading_defer_mutex_try_lock(&handle);
}

inline void mutex_unsafe_lock(mutex_handle &handle) {
  swift_threading_defer_mutex_unsafe_lock(&handle);
}

inline void mutex_unsafe_unlock(mutex_handle &handle) {
  swift_threading_defer_mutex_unsafe_unlock(&handle);
}

using lazy_mutex_handle = uintptr_t;

#define SWIFT_LAZY_MUTEX_INITIALIZER 0

inline void lazy_mutex_destroy(lazy_mutex_handle &handle) {
  swift_threading_defer_lazy_mutex_destroy(&handle);
}

inline void lazy_mutex_lock(lazy_mutex_handle &handle) {
  swift_threading_defer_lazy_mutex_lock(&handle);
}

inline void lazy_mutex_unlock(lazy_mutex_handle &handle) {
  swift_threading_defer_lazy_mutex_unlock(&handle);
}

inline bool lazy_mutex_try_lock(lazy_mutex_handle &handle) {
  return swift_threading_defer_lazy_mutex_try_lock(&handle);
}

inline void lazy_mutex_unsafe_lock(lazy_mutex_handle &handle) {
  swift_threading_defer_lazy_mutex_unsafe_lock(&handle);
}

inline void lazy_mutex_unsafe_unlock(lazy_mutex_handle &handle) {
  swift_threading_defer_lazy_mutex_unsafe_unlock(&handle);
}

// .. Recursive mutex support ................................................

inline void recursive_mutex_init(recursive_mutex_handle &handle,
                                 bool checked = false) {
  swift_threading_defer_recursive_mutex_init(handle.storage, checked);
}

inline void recursive_mutex_destroy(recursive_mutex_handle &handle) {
  swift_threading_defer_recursive_mutex_destroy(handle.storage);
}

inline void recursive_mutex_lock(recursive_mutex_handle &handle) {
  swift_threading_defer_recursive_mutex_lock(handle.storage);
}

inline void recursive_mutex_unlock(recursive_mutex_handle &handle) {
  swift_threading_defer_recursive_mutex_unlock(handle.storage);
}

// .. ConditionVariable support ..............................................

using cond_handle = uintptr_t;

inline void cond_init(cond_handle &handle) {
  swift_threading_defer_cond_init(&handle);
}

inline void cond_destroy(cond_handle &handle) {
  swift_threading_defer_cond_destroy(&handle);
}

inline void cond_lock(cond_handle &handle) {
  swift_threading_defer_cond_lock(&handle);
}

inline void cond_unlock(cond_handle &handle) {
  swift_threading_defer_cond_unlock(&handle);
}

inline void cond_signal(cond_handle &handle) {
  swift_threading_defer_cond_signal(&handle);
}

inline void cond_broadcast(cond_handle &handle) {
  swift_threading_defer_cond_broadcast(&handle);
}

inline void cond_wait(cond_handle &handle) {
  swift_threading_defer_cond_wait(&handle);
}

template <class Rep, class Period>
inline bool cond_wait(cond_handle &handle,
                      std::chrono::duration<Rep, Period> duration) {
  auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
  auto count = ns.count();
  return swift_threading_defer_cond_wait_for(
      &handle, count < 0 ? 0 : static_cast<uint64_t>(count));
}

inline bool cond_wait(cond_handle &handle,
                      std::chrono::system_clock::time_point deadline) {
  auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
      deadline.time_since_epoch());
  return swift_threading_defer_cond_wait_until(&handle,
                                               static_cast<int64_t>(ns.count()));
}

// .. Once ...................................................................

using once_t = uintptr_t;

inline void once_impl(once_t &predicate, void (*fn)(void *), void *ctx) {
  swift_threading_defer_once(&predicate, fn, ctx);
}

// .. Thread local storage ...................................................

using tls_key_t = uintptr_t;
using tls_dtor_t = void (*)(void *);

inline tls_key_t tls_get_key(swift::tls_key key) {
  return static_cast<tls_key_t>(key);
}

inline bool tls_init(tls_key_t key, tls_dtor_t dtor) {
  (void)key;
  (void)dtor;
  return true;
}

inline bool tls_alloc(tls_key_t &key, tls_dtor_t dtor) {
  (void)key;
  (void)dtor;
  return false;
}

inline void *tls_get(tls_key_t key) {
  return swift_threading_defer_tls_get(key);
}

inline void tls_set(tls_key_t key, void *ptr) {
  swift_threading_defer_tls_set(key, ptr);
}

} // namespace threading_impl
} // namespace swift

#endif // SWIFT_THREADING_IMPL_THREADDEFER_H
