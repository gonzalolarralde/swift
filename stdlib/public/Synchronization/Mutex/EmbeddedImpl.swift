//===----------------------------------------------------------------------===//
//
// This source file is part of the Swift Atomics open source project
//
// Copyright (c) 2026 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

@_extern(c, "_swift_mutex_init")
@usableFromInline
internal func _swift_mutex_init(
  _ mutex: UnsafeMutablePointer<UInt>,
  _ checked: CInt
)

@_extern(c, "_swift_mutex_destroy")
@usableFromInline
internal func _swift_mutex_destroy(_ mutex: UnsafeMutablePointer<UInt>)

@_extern(c, "_swift_mutex_lock")
@usableFromInline
internal func _swift_mutex_lock(_ mutex: UnsafeMutablePointer<UInt>)

@_extern(c, "_swift_mutex_unlock")
@usableFromInline
internal func _swift_mutex_unlock(_ mutex: UnsafeMutablePointer<UInt>)

@_extern(c, "_swift_mutex_tryLock")
@usableFromInline
internal func _swift_mutex_tryLock(_ mutex: UnsafeMutablePointer<UInt>) -> CInt

@available(SwiftStdlib 6.0, *)
@frozen
@_staticExclusiveOnly
public struct _MutexHandle: ~Copyable {
  @usableFromInline
  let storage: _Cell<UInt>

  @available(SwiftStdlib 6.0, *)
  @_alwaysEmitIntoClient
  @_transparent
  public init() {
    storage = _Cell(0)
    unsafe _swift_mutex_init(storage._address, 0)
  }

  deinit {
    unsafe _swift_mutex_destroy(storage._address)
  }

  @available(SwiftStdlib 6.0, *)
  @_alwaysEmitIntoClient
  @_transparent
  internal borrowing func _lock() {
    unsafe _swift_mutex_lock(storage._address)
  }

  @available(SwiftStdlib 6.0, *)
  @_alwaysEmitIntoClient
  @_transparent
  internal borrowing func _tryLock() -> Bool {
    unsafe _swift_mutex_tryLock(storage._address) != 0
  }

  @available(SwiftStdlib 6.0, *)
  @_alwaysEmitIntoClient
  @_transparent
  internal borrowing func _unlock() {
    unsafe _swift_mutex_unlock(storage._address)
  }
}
