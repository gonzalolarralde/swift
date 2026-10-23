// RUN: %target-swift-frontend %s -module-name S3Contract -target xtensa-esp32s3-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -import-objc-header %S/Inputs/xtensa-target-contract.h -emit-ir -o - | %FileCheck %s --check-prefix=S3
// RUN: %target-swift-frontend %s -module-name S3Contract -target xtensa-esp32s3-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -import-objc-header %S/Inputs/xtensa-target-contract.h -Xcc -Xclang -Xcc -target-feature -Xcc -Xclang -Xcc +windowed -Xcc -Xclang -Xcc -target-feature -Xcc -Xclang -Xcc -s32c1i -Xcc -DEXPECT_S32C1I=0 -emit-ir -o - | %FileCheck %s --check-prefixes=S3,NO-CAS
// RUN: %target-swift-frontend %s -module-name S3Contract -target xtensa-esp32s3-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -import-objc-header %S/Inputs/xtensa-target-contract.h -Xcc -Xclang -Xcc -target-feature -Xcc -Xclang -Xcc -windowed -Xcc -Xclang -Xcc -target-feature -Xcc -Xclang -Xcc +windowed -typecheck
// RUN: %target-swift-frontend %s -module-name S3Contract -target xtensa-esp32s3-none-elf -clang-target xtensa-esp32s3-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -emit-ir -o - | %FileCheck %s --check-prefix=S3
// RUN: not %target-swift-frontend %s -module-name S3Contract -target xtensa-esp32s3-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -Xcc -Xclang -Xcc -target-feature -Xcc -Xclang -Xcc -windowed -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-ABI
// RUN: not %target-swift-frontend %s -module-name S3Contract -target xtensa-esp32s3-none-elf -clang-target xtensa-esp32s3-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -Xcc -Xclang -Xcc -target-feature -Xcc -Xclang -Xcc +windowed -Xcc -Xclang -Xcc -target-feature -Xcc -Xclang -Xcc -windowed -emit-ir -o /dev/null 2>&1 | %FileCheck %s --check-prefix=BAD-ABI
// RUN: not %target-swift-frontend %s -module-name S3Contract -target xtensa-esp32s3-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -Xcc -target -Xcc xtensa-unknown-none-elf -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-TRIPLE
// RUN: not %target-swift-frontend %s -module-name S3Contract -target xtensa-esp32s3-none-elf -clang-target xtensa-unknown-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-TRIPLE
// REQUIRES: swift_feature_Embedded
// REQUIRES: CODEGENERATOR=Xtensa

// S3: target datalayout = "e-m:e-p:32:32-i8:8:32-i16:16:32-i64:64-n32"
// S3: target triple = "xtensa-esp32s3-none-elf"
// S3: "target-cpu"="esp32s3"
// S3-SAME: "target-features"="{{[^"]*}}+windowed
// NO-CAS-SAME: ,-s32c1i"
// BAD-ABI: error: ABI 'call0' is not supported for 'xtensa-esp32s3-none-elf'
// BAD-ABI-NOT: Stack dump
// BAD-TRIPLE: error: invalid value 'xtensa-unknown-none-elf' in 'Clang target for xtensa-esp32s3-none-elf'

@_cdecl("s3_identity")
public func identity(_ x: UInt32) -> UInt32 { x }

