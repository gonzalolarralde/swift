; RUN: %swift-llvm-opt -passes=swift-llvm-arc-optimize %s | %FileCheck %s --check-prefix=MOTION
; RUN: %swift-llvm-opt -passes=gvn %s | %FileCheck %s --check-prefix=AA

; Embedded Swift defines the reference-counting entry points in the client
; module. Their bodies and inlined uniqueness checks access ordinary IR memory.
; Moving this retain below the load makes a shared Array appear unique.

define weak_odr ptr @swift_retain(ptr returned %object) noinline nounwind {
entry:
  %refcount = getelementptr i8, ptr %object, i32 4
  %old = atomicrmw add ptr %refcount, i32 1 monotonic
  ret ptr %object
}

; MOTION-LABEL: define i32 @visible_uniqueness(
; MOTION: call ptr @swift_retain
; MOTION-NEXT: %count = load atomic i32
define i32 @visible_uniqueness(ptr %object) {
entry:
  %refcount = getelementptr i8, ptr %object, i32 4
  call ptr @swift_retain(ptr %object)
  %count = load atomic i32, ptr %refcount acquire, align 4
  ret i32 %count
}

; AA-LABEL: define i32 @visible_refcount_change(
; AA: %before = load i32
; AA: call ptr @swift_retain
; AA: %after = load i32
; MOTION-LABEL: define i32 @visible_refcount_change(
; MOTION: %before = load i32
; MOTION: call ptr @swift_retain
; MOTION-NEXT: %after = load i32
define i32 @visible_refcount_change(ptr %object) {
entry:
  %refcount = getelementptr i8, ptr %object, i32 4
  %before = load i32, ptr %refcount, align 4
  call ptr @swift_retain(ptr %object)
  %after = load i32, ptr %refcount, align 4
  %difference = sub i32 %after, %before
  ret i32 %difference
}
