// RUN: %empty-directory(%t)
// RUN: %{python} %utils/split_file.py -o %t %s

// RUN: %target-swift-frontend %t/Main.swift -parse-as-library -import-bridging-header %t/BridgingHeader.h -enable-experimental-feature Embedded -disable-availability-checking -c -o %t/main.o
// RUN: %target-clang %target-clang-resource-dir-opt %t/main.o -o %t/a.out -dead_strip %target-embedded-pthreads-shim
// RUN: %target-run %t/a.out | %FileCheck %s

// REQUIRES: swift_in_compiler
// REQUIRES: executable_test
// REQUIRES: optimized_stdlib
// REQUIRES: synchronization
// REQUIRES: OS=macosx || OS=linux-gnu
// REQUIRES: swift_feature_Embedded

// BEGIN BridgingHeader.h

typedef void *pthread_t;
int pthread_create(pthread_t *thread, const void *attr, void *(*start_routine)(void *), void *arg);
int pthread_join(pthread_t thread, void **value_ptr);

// BEGIN Main.swift

import Synchronization

func incrementCounter(_ argument: UnsafeMutableRawPointer?) -> UnsafeMutableRawPointer? {
  Main.increment()
  return nil
}

@main
struct Main {
  static let counter = Mutex(0)

  static func main() {
    print("Start")

    var t1 = pthread_t(bitPattern: 0)
    pthread_create(&t1, nil, incrementCounter, nil)

    var t2 = pthread_t(bitPattern: 0)
    pthread_create(&t2, nil, incrementCounter, nil)

    pthread_join(t1, nil)
    pthread_join(t2, nil)

    counter.withLock {
      print($0)
    }

    // CHECK: Start
    // CHECK-NEXT: 2000
  }

  static func increment() {
    for _ in 0..<1000 {
      counter.withLock {
        $0 += 1
      }
    }
  }
}
