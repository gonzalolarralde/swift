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

fileprivate struct SingleThreadedTLS {
  static let reservedKeyCount = 8
  static let keyCount = 24
  static var nextDynamicKey = reservedKeyCount
  static var values = [24 of UnsafeMutableRawPointer?](repeating: nil)
}

@implementation @c
public func _swift_tls_init(
  _ key: Int,
  _ destructor: (@convention(c) (UnsafeMutableRawPointer?) -> Void)?
) -> Int {
  key < SingleThreadedTLS.keyCount ? 1 : 0
}

@implementation @c
public func _swift_tls_alloc(
  _ key: UnsafeMutablePointer<Int>,
  _ destructor: (@convention(c) (UnsafeMutableRawPointer?) -> Void)?
) -> Int {
  if SingleThreadedTLS.nextDynamicKey >= SingleThreadedTLS.keyCount {
    return 0
  }

  key.pointee = SingleThreadedTLS.nextDynamicKey
  SingleThreadedTLS.nextDynamicKey += 1
  return 1
}

@implementation @c
public func _swift_tls_get(_ key: Int) -> UnsafeMutableRawPointer? {
  if key >= SingleThreadedTLS.keyCount {
    return nil
  }
  return SingleThreadedTLS.values[key]
}

@implementation @c
public func _swift_tls_set(_ key: Int, _ value: UnsafeMutableRawPointer?) {
  if key >= SingleThreadedTLS.keyCount {
    return
  }
  SingleThreadedTLS.values[key] = value
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
