/*===--- EmbeddedPlatformFreestandingPThread.h ------------------*- C -*-===*
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

#ifndef EMBEDDED_PLATFORM_FREESTANDING_PTHREAD_H
#define EMBEDDED_PLATFORM_FREESTANDING_PTHREAD_H

#include "swift/EmbeddedPlatform.h"

/*
 * Minimal pthread-shaped ABI used by swiftEmbeddedPlatformMultiThreadedPOSIX.
 *
 * Freestanding embedded SDKs that link that library are expected to provide
 * these symbols with this ABI. This header intentionally does not include
 * system pthread headers, so the library can be built for bare-metal triples.
 */

typedef struct {
  __swift_ptrdiff_t _storage[6];
} pthread_mutex_t;

typedef __swift_ptrdiff_t pthread_mutexattr_t;
typedef __swift_ptrdiff_t pthread_key_t;
typedef __swift_ptrdiff_t pthread_t;

enum {
  PTHREAD_MUTEX_NORMAL = 0,
  PTHREAD_MUTEX_ERRORCHECK = 1,
  PTHREAD_MUTEX_RECURSIVE = 2,
  SWIFT_EMBEDDED_PTHREAD_EBUSY = 16
};

int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);

int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);

int pthread_key_create(pthread_key_t *key, __swift_tls_dtor_t destructor);
void *pthread_getspecific(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, const void *value);

pthread_t pthread_self(void);

#endif
