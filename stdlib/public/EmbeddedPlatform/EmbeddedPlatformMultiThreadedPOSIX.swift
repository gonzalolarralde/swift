//===------- EmbeddedPlatformMultiThreadedPOSIX.swift ---------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2026 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

@inline(__always)
func _swift_embedded_platform_mutex(
  _ mutex: UnsafeMutableRawPointer
) -> UnsafeMutablePointer<pthread_mutex_t> {
  mutex.assumingMemoryBound(to: pthread_mutex_t.self)
}

func _swift_embedded_platform_mutex_init(
  _ pthreadMutex: UnsafeMutablePointer<pthread_mutex_t>,
  _ flags: SwiftMutexFlags
) {
  if flags.contains(.recursive) || flags.contains(.checked) {
    var attr = pthread_mutexattr_t()
    if pthread_mutexattr_init(&attr) != 0 {
      fatalError("pthread_mutexattr_init failed while initializing an embedded mutex")
    }
    if flags.contains(.recursive) {
      if pthread_mutexattr_settype(&attr, CInt(PTHREAD_MUTEX_RECURSIVE)) != 0 {
        fatalError("pthread_mutexattr_settype failed while requesting a recursive embedded mutex")
      }
    } else {
      if pthread_mutexattr_settype(&attr, CInt(PTHREAD_MUTEX_ERRORCHECK)) != 0 {
        fatalError("pthread_mutexattr_settype failed while requesting a checked embedded mutex")
      }
    }
    if pthread_mutex_init(pthreadMutex, &attr) != 0 {
      fatalError("pthread_mutex_init failed while initializing an embedded mutex")
    }
    if pthread_mutexattr_destroy(&attr) != 0 {
      fatalError("pthread_mutexattr_destroy failed while initializing an embedded mutex")
    }
  } else {
    if pthread_mutex_init(pthreadMutex, nil) != 0 {
      fatalError("pthread_mutex_init failed while initializing an embedded mutex")
    }
  }
}

@implementation @c
public func _swift_mutex_init(
  _ mutex: UnsafeMutableRawPointer,
  _ flags: SwiftMutexFlags
) {
  _swift_embedded_platform_mutex_init(
    _swift_embedded_platform_mutex(mutex),
    flags)
}

@implementation @c
public func _swift_mutex_destroy(_ mutex: UnsafeMutableRawPointer) {
  if pthread_mutex_destroy(_swift_embedded_platform_mutex(mutex)) != 0 {
    fatalError("pthread_mutex_destroy failed while destroying an embedded mutex")
  }
}

@implementation @c
public func _swift_mutex_lock(_ mutex: UnsafeMutableRawPointer) {
  if pthread_mutex_lock(_swift_embedded_platform_mutex(mutex)) != 0 {
    fatalError("pthread_mutex_lock failed while locking an embedded mutex")
  }
}

@implementation @c
public func _swift_mutex_unlock(_ mutex: UnsafeMutableRawPointer) {
  if pthread_mutex_unlock(_swift_embedded_platform_mutex(mutex)) != 0 {
    fatalError("pthread_mutex_unlock failed while unlocking an embedded mutex")
  }
}

@implementation @c
public func _swift_mutex_tryLock(_ mutex: UnsafeMutableRawPointer) -> Int {
  let result = pthread_mutex_trylock(_swift_embedded_platform_mutex(mutex))
  if result == 0 {
    return 1
  }
  if result == SWIFT_EMBEDDED_PTHREAD_EBUSY {
    return 0
  }
  fatalError("pthread_mutex_trylock failed while trying to lock an embedded mutex")
}

var _swift_embedded_platform_once_lock: pthread_mutex_t = {
  var mutex = pthread_mutex_t()
  if pthread_mutex_init(&mutex, nil) != 0 {
    fatalError("pthread_mutex_init failed while initializing embedded once state")
  }
  return mutex
}()

@implementation @c
public func _swift_once(
  _ predicate: UnsafeMutablePointer<Int>,
  _ function: @convention(c) (UnsafeMutableRawPointer?) -> Void,
  _ context: UnsafeMutableRawPointer?
) {
  while true {
    if pthread_mutex_lock(&_swift_embedded_platform_once_lock) != 0 {
      fatalError("pthread_mutex_lock failed while entering embedded once state")
    }

    if predicate.pointee == 2 {
      if pthread_mutex_unlock(&_swift_embedded_platform_once_lock) != 0 {
        fatalError("pthread_mutex_unlock failed while leaving completed embedded once state")
      }
      return
    }

    if predicate.pointee == 0 {
      predicate.pointee = 1
      if pthread_mutex_unlock(&_swift_embedded_platform_once_lock) != 0 {
        fatalError("pthread_mutex_unlock failed before running embedded once initializer")
      }

      function(context)

      if pthread_mutex_lock(&_swift_embedded_platform_once_lock) != 0 {
        fatalError("pthread_mutex_lock failed after running embedded once initializer")
      }
      predicate.pointee = 2
      if pthread_mutex_unlock(&_swift_embedded_platform_once_lock) != 0 {
        fatalError("pthread_mutex_unlock failed after completing embedded once initializer")
      }
      return
    }

    if pthread_mutex_unlock(&_swift_embedded_platform_once_lock) != 0 {
      fatalError("pthread_mutex_unlock failed while waiting for embedded once initializer")
    }
  }
}

fileprivate struct MultiThreadedTLS {
  static let reservedKeyCount = 8
  static let keyCount = 24
  static var nextDynamicKey = reservedKeyCount
  static var lock: pthread_mutex_t = {
    var mutex = pthread_mutex_t()
    if pthread_mutex_init(&mutex, nil) != 0 {
      fatalError("pthread_mutex_init failed while initializing embedded TLS state")
    }
    return mutex
  }()
  static var keys = [24 of pthread_key_t](repeating: pthread_key_t())
  static var initialized = [24 of Bool](repeating: false)
}

@implementation @c
public func _swift_tls_init(
  _ key: Int,
  _ destructor: (@convention(c) (UnsafeMutableRawPointer?) -> Void)?
) -> Int {
  if key < 0 || key >= MultiThreadedTLS.keyCount {
    return 0
  }

  if pthread_mutex_lock(&MultiThreadedTLS.lock) != 0 {
    fatalError("pthread_mutex_lock failed while initializing an embedded TLS key")
  }
  defer {
    if pthread_mutex_unlock(&MultiThreadedTLS.lock) != 0 {
      fatalError("pthread_mutex_unlock failed after initializing an embedded TLS key")
    }
  }

  if MultiThreadedTLS.initialized[key] {
    return 1
  }

  var pthreadKey = pthread_key_t()
  if pthread_key_create(&pthreadKey, destructor) != 0 {
    return 0
  }

  MultiThreadedTLS.keys[key] = pthreadKey
  MultiThreadedTLS.initialized[key] = true
  return 1
}

@implementation @c
public func _swift_tls_alloc(
  _ key: UnsafeMutablePointer<Int>,
  _ destructor: (@convention(c) (UnsafeMutableRawPointer?) -> Void)?
) -> Int {
  if pthread_mutex_lock(&MultiThreadedTLS.lock) != 0 {
    fatalError("pthread_mutex_lock failed while allocating an embedded TLS key")
  }
  defer {
    if pthread_mutex_unlock(&MultiThreadedTLS.lock) != 0 {
      fatalError("pthread_mutex_unlock failed after allocating an embedded TLS key")
    }
  }

  if MultiThreadedTLS.nextDynamicKey >= MultiThreadedTLS.keyCount {
    return 0
  }

  var pthreadKey = pthread_key_t()
  if pthread_key_create(&pthreadKey, destructor) != 0 {
    return 0
  }

  key.pointee = MultiThreadedTLS.nextDynamicKey
  MultiThreadedTLS.keys[key.pointee] = pthreadKey
  MultiThreadedTLS.initialized[key.pointee] = true
  MultiThreadedTLS.nextDynamicKey += 1
  return 1
}

@implementation @c
public func _swift_tls_get(_ key: Int) -> UnsafeMutableRawPointer? {
  if key < 0 ||
      key >= MultiThreadedTLS.keyCount ||
      !MultiThreadedTLS.initialized[key] {
    return nil
  }

  return pthread_getspecific(MultiThreadedTLS.keys[key])
}

@implementation @c
public func _swift_tls_set(_ key: Int, _ value: UnsafeMutableRawPointer?) {
  if key < 0 ||
      key >= MultiThreadedTLS.keyCount ||
      !MultiThreadedTLS.initialized[key] {
    return
  }

  if pthread_setspecific(MultiThreadedTLS.keys[key], value) != 0 {
    fatalError("pthread_setspecific failed while setting an embedded TLS value")
  }
}

@implementation @c
public func _swift_thread_getCurrentId() -> Int {
  pthread_self()
}
