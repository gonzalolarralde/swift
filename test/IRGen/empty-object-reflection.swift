// RUN: %target-swift-frontend %s -module-name Empty -target riscv32-none-none-eabi -sdk "" -parse-stdlib -parse-as-library -emit-empty-object-file -c -o %t.o
// RUN: %llvm-nm --defined-only %t.o | count 0
// REQUIRES: CODEGENERATOR=RISCV

// Reflection's version marker must not add a global to an explicitly empty
// object. This also works without an installed target standard library.
public func unused() {}
