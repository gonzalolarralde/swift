// RUN: %empty-directory(%t)
// RUN: split-file %s %t
// RUN: %target-clang -I %swift_obj_root/include -c %t/clock.c -o %t/clock.o
// RUN: %target-swift-frontend -enable-experimental-feature Embedded -O -parse-as-library -c %t/main.swift -o %t/main.o
// RUN: %target-clang %target-clang-resource-dir-opt %t/main.o %t/clock.o -L%swift_obj_root/lib/swift/embedded/%module-target-triple -lswift_Concurrency %target-swift-default-executor-opt %target-embedded-concurrency-threading-shim %target-embedded-posix-shim %if OS=macosx %{ -lc++ -Wl,-dead_strip %} %else %{ -lstdc++ -lpthread -Wl,--gc-sections %} -o %t/main
// RUN: %target-run %t/main | %FileCheck %s
// RUN: %llvm-nm %t/main | %FileCheck %s --check-prefix=SYMBOLS
// RUN: %llvm-nm %t/main | %FileCheck %s --check-prefix=ABSENT --allow-empty
// RUN: %target-swift-frontend -enable-experimental-feature Embedded -parse-as-library -c %t/main.swift -o %t/main.o
// RUN: %target-clang %target-clang-resource-dir-opt %t/main.o %t/clock.o -L%swift_obj_root/lib/swift/embedded/%module-target-triple -lswift_Concurrency %target-swift-default-executor-opt %target-embedded-concurrency-threading-shim %target-embedded-posix-shim %if OS=macosx %{ -lc++ -Wl,-dead_strip %} %else %{ -lstdc++ -lpthread -Wl,--gc-sections %} -o %t/main
// RUN: %target-run %t/main | %FileCheck %s
// RUN: %llvm-nm %t/main | %FileCheck %s --check-prefix=SYMBOLS
// RUN: %llvm-nm %t/main | %FileCheck %s --check-prefix=ABSENT --allow-empty

// REQUIRES: swift_embedded_platform
// REQUIRES: swift_feature_Embedded
// REQUIRES: executable_test
// REQUIRES: optimized_stdlib
// REQUIRES: OS=macosx || OS=linux-gnu

// The default executor must support suspending-clock deadlines without any
// continuous-clock definitions. Check symbols as well, so the POSIX archive
// cannot silently supply the missing clock hooks.
// CHECK: done
// SYMBOLS-DAG: _swift_clockSuspending_getTime
// SYMBOLS-DAG: _swift_clockSuspending_getResolution
// SYMBOLS-DAG: swift_task_enqueueGlobalWithSuspendingDeadlineImpl
// ABSENT-NOT: _swift_clockContinuous
// ABSENT-NOT: swift_get_time
// ABSENT-NOT: swift_get_clock_res
// ABSENT-NOT: swift_sleep
// ABSENT-NOT: swift_task_enqueueGlobalWithContinuousDeadline
// ABSENT-NOT: swift_task_enqueueGlobalWithDeadline

//--- main.swift
import _Concurrency

// User-defined clocks need not implement the runtime scheduling requirement.
struct CustomClock: Clock {
  var now: SuspendingClock.Instant { SuspendingClock.now }
  var minimumResolution: Duration { .nanoseconds(1) }

  func sleep(until deadline: SuspendingClock.Instant,
             tolerance: Duration?) async throws {
    try await SuspendingClock().sleep(until: deadline, tolerance: tolerance)
  }
}

@main
struct Main {
  static func main() async throws {
    let clock = SuspendingClock()
    precondition(clock.minimumResolution == .nanoseconds(3))
    let start = clock.now
    try await Task.sleep(for: .milliseconds(2), clock: clock)
    precondition(start.duration(to: clock.now) == .milliseconds(2))
    try await clock.sleep(until: clock.now + .milliseconds(3))
    precondition(start.duration(to: clock.now) == .milliseconds(5))
    try await Task.sleep(nanoseconds: 1_000_000)
    precondition(start.duration(to: clock.now) == .milliseconds(6))
    try await clock.sleep(until: start)
    precondition(start.duration(to: clock.now) == .milliseconds(6))
    try await Task.sleep(for: .milliseconds(1), tolerance: .milliseconds(1),
                         clock: CustomClock())
    precondition(start.duration(to: clock.now) == .milliseconds(7))

    let cancelled = Task {
      withUnsafeCurrentTask { $0!.cancel() }
      do {
        try await Task.sleep(for: .milliseconds(100), clock: clock)
        preconditionFailure("cancelled sleep returned normally")
      } catch is CancellationError {
      } catch {
        preconditionFailure("unexpected error")
      }
    }
    await cancelled.value
    precondition(start.duration(to: clock.now) == .milliseconds(7))
    print("done")
  }
}

//--- clock.c
#include <swift/EmbeddedPlatform.h>

static __swift_int64_t now = 200000000000;

void _swift_clockSuspending_getTime(__swift_int64_t *seconds,
                                   __swift_int64_t *nanoseconds) {
  *seconds = now / 1000000000;
  *nanoseconds = now % 1000000000;
}

void _swift_clock_sleep(__swift_int64_t seconds, __swift_int64_t nanoseconds) {
  now += seconds * 1000000000 + nanoseconds;
}

void _swift_clockSuspending_getResolution(__swift_int64_t *seconds,
                                         __swift_int64_t *nanoseconds) {
  *seconds = 0;
  *nanoseconds = 3;
}
