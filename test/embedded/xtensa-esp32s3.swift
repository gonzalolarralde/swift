// RUN: %target-swift-frontend %s -module-name S3 -target xtensa-esp32s3-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -emit-ir -o - | %FileCheck %s
// RUN: %target-swift-frontend %s -module-name S3 -target xtensa-esp32s3-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -O -emit-object -o %t.o
// RUN: %target-swift-frontend %s -module-name S3 -target xtensa-esp32s3-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -emit-assembly -o - | %FileCheck %s --check-prefix=SECTIONS
// RUN: not %target-swift-frontend %s -module-name S3 -target xtensa-esp32s3-none-elf -sdk "" -target-cpu esp32 -enable-experimental-feature Embedded -wmo -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-CPU
// RUN: not %target-swift-frontend %s -module-name S3 -target xtensa-esp32s3-none-elf -sdk "" -Xcc -mcpu=esp32 -enable-experimental-feature Embedded -wmo -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-XCC
// RUN: not %target-swift-frontend %s -target xtensa-esp32s3-none-elf -sdk "" -typecheck 2>&1 | %FileCheck %s --check-prefix=NEEDS-EMBEDDED
// RUN: %target-swift-frontend -target xtensa-esp32s3-none-elf -sdk "" -print-target-info | %FileCheck %s --check-prefix=INFO
// REQUIRES: swift_feature_Embedded
// REQUIRES: CODEGENERATOR=Xtensa

#if !arch(xtensa) || !os(none) || !_endian(little) || !_pointerBitWidth(_32)
#error("incorrect ESP32-S3 target configuration")
#endif

// CHECK: target triple = "xtensa-esp32s3-none-elf"
// CHECK: define {{.*}}i32 @s3_add
// CHECK: "target-cpu"="esp32s3"
// SECTIONS: .text.s3_add
// BAD-XCC: invalid value 'esp32' in 'CPU for xtensa-esp32s3-none-elf'
// BAD-CPU: invalid value 'esp32' in '-target-cpu for xtensa-esp32s3-none-elf'
// NEEDS-EMBEDDED: target 'xtensa-esp32s3-none-elf' requires '-enable-experimental-feature Embedded'
// INFO: "triple": "xtensa-esp32s3-none-elf"
@_cdecl("s3_add")
public func add(_ a: UInt32, _ b: UInt32) -> UInt32 {
  a &+ b
}

// Native aggregate lowering consults Clang's Swift ABI hooks even when the
// LLVM calling convention falls back to the platform C convention.
public struct FourWords {
  public var a, b, c, d: UInt32
}

@_silgen_name("s3_four_words")
public func fourWords(_ value: FourWords) -> FourWords {
  value
}

public struct ThreeDoubles {
  public var a, b, c: Double
}

// Floating-point values also use integer argument/result registers; three
// doubles cannot be returned in the four available 32-bit result registers.
@_silgen_name("s3_three_doubles")
public func threeDoubles(_ value: ThreeDoubles) -> ThreeDoubles {
  value
}
