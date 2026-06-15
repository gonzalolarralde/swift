//===----------- EmbeddedPlatformSingleThreaded.swift ---------------------===//
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

@_extern(c, "_swift_exit")
func _swift_exit(_: Int)

func _swift_single_threaded_trap() -> Never {
  _swift_exit(1)
  while true {}
}

fileprivate struct SingleThreadedMutex {
  var checked: Bool
  var recursive: Bool
  var lockCount: UInt
}

@implementation @c
public func _swift_mutex_init(
  _ mutex: UnsafeMutableRawPointer,
  _ flags: SwiftMutexFlags
) {
  let storage = mutex.assumingMemoryBound(to: SingleThreadedMutex.self)
  storage.pointee = SingleThreadedMutex(
    checked: flags.contains(.checked),
    recursive: flags.contains(.recursive),
    lockCount: 0)
}

@implementation @c
public func _swift_mutex_destroy(_ mutex: UnsafeMutableRawPointer) {
  let storage = mutex.assumingMemoryBound(to: SingleThreadedMutex.self)
  if storage.pointee.checked && storage.pointee.lockCount != 0 {
    _swift_single_threaded_trap()
  }

  storage.pointee = SingleThreadedMutex(
    checked: false,
    recursive: false,
    lockCount: 0)
}

@implementation @c
public func _swift_mutex_lock(_ mutex: UnsafeMutableRawPointer) {
  let storage = mutex.assumingMemoryBound(to: SingleThreadedMutex.self)
  if storage.pointee.checked {
    if storage.pointee.lockCount != 0 && !storage.pointee.recursive {
      _swift_single_threaded_trap()
    }
    storage.pointee.lockCount += 1
  }
}

@implementation @c
public func _swift_mutex_unlock(_ mutex: UnsafeMutableRawPointer) {
  let storage = mutex.assumingMemoryBound(to: SingleThreadedMutex.self)
  if storage.pointee.checked {
    if storage.pointee.lockCount == 0 {
      _swift_single_threaded_trap()
    }
    storage.pointee.lockCount -= 1
  }
}

@implementation @c
public func _swift_mutex_tryLock(_ mutex: UnsafeMutableRawPointer) -> Int {
  let storage = mutex.assumingMemoryBound(to: SingleThreadedMutex.self)
  if storage.pointee.checked {
    if storage.pointee.lockCount != 0 && !storage.pointee.recursive {
      return 0
    }
    storage.pointee.lockCount += 1
  }
  return 1
}

@implementation @c
public func _swift_condition_init(_ condition: UnsafeMutablePointer<Int>) {
  condition.pointee = 0
}

@implementation @c
public func _swift_condition_destroy(_ condition: UnsafeMutablePointer<Int>) {}

@implementation @c
public func _swift_condition_lock(_ condition: UnsafeMutablePointer<Int>) {}

@implementation @c
public func _swift_condition_unlock(_ condition: UnsafeMutablePointer<Int>) {}

@implementation @c
public func _swift_condition_signal(_ condition: UnsafeMutablePointer<Int>) {}

@implementation @c
public func _swift_condition_broadcast(_ condition: UnsafeMutablePointer<Int>) {}

@implementation @c
public func _swift_condition_wait(_ condition: UnsafeMutablePointer<Int>) {}

@implementation @c
public func _swift_once(
  _ predicate: UnsafeMutablePointer<Int>,
  _ function: @convention(c) (UnsafeMutableRawPointer?) -> Void,
  _ context: UnsafeMutableRawPointer?
) {
  if predicate.pointee != 0 {
    return
  }

  predicate.pointee = 1
  function(context)
}

let _swift_embedded_platform_tls_key_count = 24
var _swift_embedded_platform_next_dynamic_tls_key = 8
var _swift_embedded_platform_tls_values = InlineArray<24, UnsafeMutableRawPointer?>(repeating: nil)

@implementation @c
public func _swift_tls_init(
  _ key: Int,
  _ destructor: (@convention(c) (UnsafeMutableRawPointer?) -> Void)?
) -> Int {
  key < _swift_embedded_platform_tls_key_count ? 1 : 0
}

@implementation @c
public func _swift_tls_alloc(
  _ key: UnsafeMutablePointer<Int>,
  _ destructor: (@convention(c) (UnsafeMutableRawPointer?) -> Void)?
) -> Int {
  if _swift_embedded_platform_next_dynamic_tls_key >= _swift_embedded_platform_tls_key_count {
    return 0
  }

  key.pointee = _swift_embedded_platform_next_dynamic_tls_key
  _swift_embedded_platform_next_dynamic_tls_key += 1
  return 1
}

@implementation @c
public func _swift_tls_get(_ key: Int) -> UnsafeMutableRawPointer? {
  if key >= _swift_embedded_platform_tls_key_count {
    return nil
  }
  return _swift_embedded_platform_tls_values[key]
}

@implementation @c
public func _swift_tls_set(_ key: Int, _ value: UnsafeMutableRawPointer?) {
  if key >= _swift_embedded_platform_tls_key_count {
    return
  }
  _swift_embedded_platform_tls_values[key] = value
}

@implementation @c
public func _swift_thread_getCurrentId() -> Int {
  0
}

@implementation @c
public func _swift_thread_isMain() -> Int {
  1
}

@implementation @c
public func _swift_thread_getCurrentStackBounds(
  _ low: UnsafeMutablePointer<UnsafeMutableRawPointer?>,
  _ high: UnsafeMutablePointer<UnsafeMutableRawPointer?>
) -> Int {
  low.pointee = nil
  high.pointee = nil
  return 0
}
