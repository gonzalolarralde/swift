// RUN: %empty-directory(%t)
// RUN: %{python} %utils/split_file.py -o %t %s

// RUN: %target-clang %target-clang-resource-dir-opt -x c -c %t/FreestandingPThread.c -I %S/../../stdlib/public/EmbeddedPlatform -o %t/pthread.o
// RUN: %target-swift-frontend -parse-as-library -enable-experimental-feature Embedded -enable-experimental-feature Extern -wmo %t/Main.swift -c -o %t/main.o
// RUN: %target-embedded-link %target-clang-resource-dir-opt %t/main.o %t/pthread.o %target-embedded-multithreaded-posix-shim -o %t/a.out -dead_strip
// RUN: %target-run %t/a.out

// REQUIRES: swift_in_compiler
// REQUIRES: executable_test
// REQUIRES: optimized_stdlib
// REQUIRES: swift_feature_Embedded
// REQUIRES: swift_embedded_platform

// BEGIN FreestandingPThread.c

#include "EmbeddedPlatformFreestandingPThread.h"

static pthread_key_t nextKey = 1;
static void *tlsValues[24];

int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *attr) {
  mutex->_storage[0] = 0;
  mutex->_storage[1] = attr ? *attr : PTHREAD_MUTEX_NORMAL;
  return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex) {
  mutex->_storage[0] = 0;
  mutex->_storage[1] = PTHREAD_MUTEX_NORMAL;
  return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex) {
  if (mutex->_storage[1] == PTHREAD_MUTEX_RECURSIVE) {
    mutex->_storage[0] += 1;
    return 0;
  }
  if (mutex->_storage[0] != 0) {
    return 1;
  }
  mutex->_storage[0] = 1;
  return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
  if (mutex->_storage[0] == 0) {
    return 1;
  }
  mutex->_storage[0] -= 1;
  return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex) {
  if (mutex->_storage[0] != 0 && mutex->_storage[1] != PTHREAD_MUTEX_RECURSIVE) {
    return SWIFT_EMBEDDED_PTHREAD_EBUSY;
  }
  mutex->_storage[0] += 1;
  return 0;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr) {
  *attr = PTHREAD_MUTEX_NORMAL;
  return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr) {
  return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type) {
  *attr = type;
  return 0;
}

int pthread_key_create(pthread_key_t *key, __swift_tls_dtor_t destructor) {
  *key = nextKey++;
  return *key < 24 ? 0 : 1;
}

void *pthread_getspecific(pthread_key_t key) {
  return tlsValues[key];
}

int pthread_setspecific(pthread_key_t key, const void *value) {
  tlsValues[key] = (void *)value;
  return 0;
}

pthread_t pthread_self(void) {
  return 1;
}

__swift_ptrdiff_t _swift_thread_isMain(void) {
  return 1;
}

void test_swift_once(__swift_once_t *predicate,
                     void (*function)(void *),
                     void *context) {
  _swift_once(predicate, function, context);
}

// BEGIN Main.swift

@_extern(c, "_swift_mutex_init")
func _swift_mutex_init(_ mutex: UnsafeMutableRawPointer, _ flags: UInt64)

@_extern(c, "_swift_mutex_destroy")
func _swift_mutex_destroy(_ mutex: UnsafeMutableRawPointer)

@_extern(c, "_swift_mutex_lock")
func _swift_mutex_lock(_ mutex: UnsafeMutableRawPointer)

@_extern(c, "_swift_mutex_unlock")
func _swift_mutex_unlock(_ mutex: UnsafeMutableRawPointer)

@_extern(c, "_swift_mutex_tryLock")
func _swift_mutex_tryLock(_ mutex: UnsafeMutableRawPointer) -> Int

@_extern(c, "test_swift_once")
func test_swift_once(
  _ predicate: UnsafeMutablePointer<Int>,
  _ function: @convention(c) (UnsafeMutableRawPointer?) -> Void,
  _ context: UnsafeMutableRawPointer?
)

@_extern(c, "_swift_tls_init")
func _swift_tls_init(
  _ key: Int,
  _ destructor: (@convention(c) (UnsafeMutableRawPointer?) -> Void)?
) -> Int

@_extern(c, "_swift_tls_alloc")
func _swift_tls_alloc(
  _ key: UnsafeMutablePointer<Int>,
  _ destructor: (@convention(c) (UnsafeMutableRawPointer?) -> Void)?
) -> Int

@_extern(c, "_swift_tls_get")
func _swift_tls_get(_ key: Int) -> UnsafeMutableRawPointer?

@_extern(c, "_swift_tls_set")
func _swift_tls_set(_ key: Int, _ value: UnsafeMutableRawPointer?)

@_extern(c, "_swift_thread_getCurrentId")
func _swift_thread_getCurrentId() -> Int

@_extern(c, "_swift_thread_isMain")
func _swift_thread_isMain() -> Int

func check(_ condition: Bool) {
  if !condition {
    fatalError("check failed")
  }
}

func withMutexStorage(_ body: (UnsafeMutableRawPointer) -> Void) {
  var storage: [6 of UInt] = [0, 0, 0, 0, 0, 0]
  withUnsafeMutablePointer(to: &storage) {
    body(UnsafeMutableRawPointer($0))
  }
}

let swiftMutexChecked: UInt64 = 0x01
let swiftMutexRecursive: UInt64 = 0x02

@main
struct Main {
  static func main() {
    withMutexStorage { mutex in
      _swift_mutex_init(mutex, 0)
      _swift_mutex_lock(mutex)
      check(_swift_mutex_tryLock(mutex) == 0)
      _swift_mutex_unlock(mutex)
      check(_swift_mutex_tryLock(mutex) != 0)
      _swift_mutex_unlock(mutex)
      _swift_mutex_destroy(mutex)
    }

    withMutexStorage { mutex in
      _swift_mutex_init(mutex, swiftMutexChecked | swiftMutexRecursive)
      _swift_mutex_lock(mutex)
      _swift_mutex_lock(mutex)
      _swift_mutex_unlock(mutex)
      _swift_mutex_unlock(mutex)
      _swift_mutex_destroy(mutex)
    }

    var oncePredicate = 0
    var onceCount = 0
    test_swift_once(&oncePredicate, { context in
      let count = context!.assumingMemoryBound(to: Int.self)
      count.pointee += 1
    }, &onceCount)
    test_swift_once(&oncePredicate, { context in
      let count = context!.assumingMemoryBound(to: Int.self)
      count.pointee += 1
    }, &onceCount)
    check(onceCount == 1)

    check(_swift_tls_init(0, nil) != 0)
    _swift_tls_set(0, UnsafeMutableRawPointer(bitPattern: 0x1234))
    check(_swift_tls_get(0) == UnsafeMutableRawPointer(bitPattern: 0x1234))

    var dynamicKey = 0
    check(_swift_tls_alloc(&dynamicKey, nil) != 0)
    _swift_tls_set(dynamicKey, UnsafeMutableRawPointer(bitPattern: 0x5678))
    check(_swift_tls_get(dynamicKey) == UnsafeMutableRawPointer(bitPattern: 0x5678))

    check(_swift_thread_getCurrentId() == 1)
    check(_swift_thread_isMain() != 0)
  }
}
