// RUN: %target-swift-frontend %s -emit-ir -g -o - \
// RUN:    -module-name a -enable-experimental-concurrency \
// RUN:    | %FileCheck %s --check-prefix=CHECK
// REQUIRES: concurrency

// UNSUPPORTED: CPU=arm64e

// Test that lifetime extension preserves a dbg.declare for "n" in the resume
// funclet.

// CHECK-LABEL: define {{.*}} void @"$s1a4fiboyS2iYF.resume.0"
// CHECK-NEXT: entryresume.0:
// CHECK-NEXT: call void @llvm.dbg.declare(metadata {{.*}}%2, metadata ![[R:[0-9]+]], {{.*}}!DIExpression(DW_OP
// CHECK-NEXT: call void @llvm.dbg.declare(metadata {{.*}}%2, metadata ![[LHS:[0-9]+]], {{.*}}!DIExpression(DW_OP
// CHECK-NEXT: call void @llvm.dbg.declare(metadata {{.*}}%2, metadata ![[RHS:[0-9]+]], {{.*}}!DIExpression(DW_OP
// CHECK-NEXT: call void @llvm.dbg.declare(metadata {{.*}}%2, metadata ![[N:[0-9]+]], {{.*}}!DIExpression(DW_OP
// CHECK-NOT: {{ ret }}
// CHECK: call void asm sideeffect ""
// CHECK: ![[R]] = !DILocalVariable(name: "retval"
// CHECK: ![[LHS]] = !DILocalVariable(name: "lhs"
// CHECK: ![[RHS]] = !DILocalVariable(name: "rhs"
// CHECK: ![[N]] = !DILocalVariable(name: "n"
public func fibo(_ n: Int) async -> Int {
  var retval = n
  if retval < 2 { return 1 }
  retval = retval - 1
  let lhs = await fibo(retval - 1)
  let rhs = await fibo(retval - 2)
  return lhs + rhs + retval
}
