// RUN: %empty-directory(%t)
// RUN: split-file %s %t
// RUN: %target-swift-frontend -enable-experimental-feature Embedded -O -parse-as-library -c %t/clock.swift -o %t/continuous.o
// RUN: %target-clang %target-clang-resource-dir-opt -I %swift_obj_root/include %t/clock.c %t/continuous.o -L%swift_obj_root/lib/swift/embedded/%module-target-triple -lswift_Concurrency %target-embedded-concurrency-threading-shim %target-embedded-posix-shim %if OS=macosx %{ -lc++ -Wl,-dead_strip %} %else %{ -lstdc++ -lpthread -Wl,--gc-sections %} -o %t/continuous
// RUN: %target-run %t/continuous
// RUN: %llvm-nm %t/continuous > %t/continuous.symbols
// RUN: %FileCheck %s --check-prefix=CONTINUOUS < %t/continuous.symbols
// RUN: %FileCheck %s --check-prefix=NO-SUSPENDING < %t/continuous.symbols
// RUN: %target-swift-frontend -enable-experimental-feature Embedded -O -D SUSPENDING -parse-as-library -c %t/clock.swift -o %t/suspending.o
// RUN: %target-clang %target-clang-resource-dir-opt -D SUSPENDING -I %swift_obj_root/include %t/clock.c %t/suspending.o -L%swift_obj_root/lib/swift/embedded/%module-target-triple -lswift_Concurrency %target-embedded-concurrency-threading-shim %target-embedded-posix-shim %if OS=macosx %{ -lc++ -Wl,-dead_strip %} %else %{ -lstdc++ -lpthread -Wl,--gc-sections %} -o %t/suspending
// RUN: %target-run %t/suspending
// RUN: %llvm-nm %t/suspending > %t/suspending.symbols
// RUN: %FileCheck %s --check-prefix=SUSPENDING < %t/suspending.symbols
// RUN: %FileCheck %s --check-prefix=NO-CONTINUOUS < %t/suspending.symbols

// REQUIRES: swift_embedded_platform
// REQUIRES: swift_feature_Embedded
// REQUIRES: executable_test
// REQUIRES: optimized_stdlib
// REQUIRES: OS=macosx || OS=linux-gnu

// Reading a clock must only require that clock's two platform hooks.
// The POSIX archive supplies non-clock support, but its clock object must
// not be extracted. The default executor is deliberately not linked.
// CONTINUOUS-DAG: _swift_clockContinuous_getTime
// CONTINUOUS-DAG: _swift_clockContinuous_getResolution
// SUSPENDING-DAG: _swift_clockSuspending_getTime
// SUSPENDING-DAG: _swift_clockSuspending_getResolution
// NO-SUSPENDING-NOT: _swift_clockSuspending
// NO-SUSPENDING-NOT: _swift_clock_sleep
// NO-CONTINUOUS-NOT: _swift_clockContinuous
// NO-CONTINUOUS-NOT: _swift_clock_sleep

//--- clock.swift
import _Concurrency

@_cdecl("checkClock")
public func checkClock() -> Int32 {
#if SUSPENDING
  let clock = SuspendingClock()
#else
  let clock = ContinuousClock()
#endif
  let elapsed = clock.systemEpoch.duration(to: clock.now)
  return elapsed == .seconds(11) + .nanoseconds(22)
    && clock.minimumResolution == .nanoseconds(33) ? 0 : 1
}

//--- clock.c
#include <swift/EmbeddedPlatform.h>
#include <swift/ExecutorImpl.h>

#if defined(SUSPENDING)
#define GET_TIME _swift_clockSuspending_getTime
#define GET_RESOLUTION _swift_clockSuspending_getResolution
#else
#define GET_TIME _swift_clockContinuous_getTime
#define GET_RESOLUTION _swift_clockContinuous_getResolution
#endif

void GET_TIME(__swift_int64_t *seconds, __swift_int64_t *nanoseconds) {
  *seconds = 11;
  *nanoseconds = 22;
}

void GET_RESOLUTION(__swift_int64_t *seconds, __swift_int64_t *nanoseconds) {
  *seconds = 0;
  *nanoseconds = 33;
}

extern int checkClock(void);
// Importing Concurrency also emits support for deleted async methods.
// Satisfy its scheduler dependency without introducing any timer hooks.
SWIFT_CC(swift) void swift_task_enqueueGlobalImpl(SwiftJob *job) {
  __builtin_trap();
}

int main(void) {
  return checkClock();
}
