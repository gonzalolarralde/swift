//===--- ThreadDeferDefaults.cpp - Default defer hooks ----------*- C++ -*-===//
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
// Provides weak no-thread defaults for the SWIFT_THREADING_DEFER hook surface.
// Embedded platforms can provide strong definitions with the same C symbol
// names to override these defaults.
//
//===----------------------------------------------------------------------===//

#if SWIFT_THREADING_DEFER

#include <cstdint>
#include <cstring>

#include "swift/Threading/TLSKeys.h"

#if defined(__GNUC__) || defined(__clang__)
#define SWIFT_THREADING_DEFER_DEFAULT __attribute__((weak))
#else
#define SWIFT_THREADING_DEFER_DEFAULT
#endif

namespace {

constexpr uintptr_t getReservedTLSKeyCount() {
  return static_cast<uintptr_t>(swift::tls_key::exclusivity) + 1;
}

void *ReservedTLS[getReservedTLSKeyCount()];

} // namespace

extern "C" {

SWIFT_THREADING_DEFER_DEFAULT
uintptr_t swift_threading_defer_current_thread_id() {
  return 0;
}

SWIFT_THREADING_DEFER_DEFAULT
bool swift_threading_defer_is_main_thread() {
  return true;
}

SWIFT_THREADING_DEFER_DEFAULT
bool swift_threading_defer_current_stack_bounds(void **low, void **high) {
  if (low)
    *low = nullptr;
  if (high)
    *high = nullptr;
  return false;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_mutex_init(uintptr_t *handle, bool checked) {
  (void)checked;
  if (handle)
    *handle = 0;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_mutex_destroy(uintptr_t *handle) {
  (void)handle;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_mutex_lock(uintptr_t *handle) {
  (void)handle;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_mutex_unlock(uintptr_t *handle) {
  (void)handle;
}

SWIFT_THREADING_DEFER_DEFAULT
bool swift_threading_defer_mutex_try_lock(uintptr_t *handle) {
  (void)handle;
  return true;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_mutex_unsafe_lock(uintptr_t *handle) {
  swift_threading_defer_mutex_lock(handle);
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_mutex_unsafe_unlock(uintptr_t *handle) {
  swift_threading_defer_mutex_unlock(handle);
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_lazy_mutex_destroy(uintptr_t *handle) {
  (void)handle;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_lazy_mutex_lock(uintptr_t *handle) {
  swift_threading_defer_mutex_lock(handle);
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_lazy_mutex_unlock(uintptr_t *handle) {
  swift_threading_defer_mutex_unlock(handle);
}

SWIFT_THREADING_DEFER_DEFAULT
bool swift_threading_defer_lazy_mutex_try_lock(uintptr_t *handle) {
  return swift_threading_defer_mutex_try_lock(handle);
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_lazy_mutex_unsafe_lock(uintptr_t *handle) {
  swift_threading_defer_mutex_lock(handle);
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_lazy_mutex_unsafe_unlock(uintptr_t *handle) {
  swift_threading_defer_mutex_unlock(handle);
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_recursive_mutex_init(uintptr_t *storage,
                                                bool checked) {
  (void)checked;
  if (storage)
    std::memset(storage, 0, sizeof(uintptr_t) * 4);
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_recursive_mutex_destroy(uintptr_t *storage) {
  (void)storage;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_recursive_mutex_lock(uintptr_t *storage) {
  (void)storage;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_recursive_mutex_unlock(uintptr_t *storage) {
  (void)storage;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_cond_init(uintptr_t *handle) {
  if (handle)
    *handle = 0;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_cond_destroy(uintptr_t *handle) {
  (void)handle;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_cond_lock(uintptr_t *handle) {
  (void)handle;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_cond_unlock(uintptr_t *handle) {
  (void)handle;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_cond_signal(uintptr_t *handle) {
  (void)handle;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_cond_broadcast(uintptr_t *handle) {
  (void)handle;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_cond_wait(uintptr_t *handle) {
  (void)handle;
}

SWIFT_THREADING_DEFER_DEFAULT
bool swift_threading_defer_cond_wait_for(uintptr_t *handle, uint64_t ns) {
  (void)handle;
  (void)ns;
  return true;
}

SWIFT_THREADING_DEFER_DEFAULT
bool swift_threading_defer_cond_wait_until(uintptr_t *handle, int64_t epochNs) {
  (void)handle;
  (void)epochNs;
  return true;
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_once(uintptr_t *predicate, void (*fn)(void *),
                                void *ctx) {
  if (!predicate || *predicate)
    return;

  *predicate = 1;
  fn(ctx);
}

SWIFT_THREADING_DEFER_DEFAULT
void *swift_threading_defer_tls_get(uintptr_t key) {
  if (key >= getReservedTLSKeyCount())
    return nullptr;
  return ReservedTLS[key];
}

SWIFT_THREADING_DEFER_DEFAULT
void swift_threading_defer_tls_set(uintptr_t key, void *value) {
  if (key >= getReservedTLSKeyCount())
    return;
  ReservedTLS[key] = value;
}

} // extern "C"

#endif // SWIFT_THREADING_DEFER
