// RUN: %target-swift-frontend %s -module-name NoStdlib -target xtensa-esp32-none-elf -sdk "" -parse-stdlib -parse-as-library -emit-ir -o - | %FileCheck %s
// RUN: %target-swift-frontend %s -module-name NoStdlib -target xtensa-esp32s2-none-elf -sdk "" -parse-stdlib -parse-as-library -emit-empty-object-file -c -o %t.s2.o
// RUN: %target-swift-frontend %s -module-name NoStdlib -target xtensa-esp32s3-none-elf -sdk "" -parse-stdlib -parse-as-library -emit-ir -o - | %FileCheck %s
// RUN: %target-swift-frontend %s -module-name NoStdlib -target xtensa-esp32s3-none-elf -sdk "" -parse-stdlib -parse-as-library -emit-empty-object-file -c -o %t.o
// REQUIRES: CODEGENERATOR=Xtensa

// Operations that do not consume the standard library do not need Embedded
// mode. In particular, compiler tests use -parse-stdlib to define their own
// types and compare Embedded and non-Embedded diagnostics.
// CHECK: define {{.*}}void @"$s{{.*}}4noopyyF"
public func noop() {}
