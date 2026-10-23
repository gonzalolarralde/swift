// RUN: %target-swift-frontend %s -module-name Family -target xtensa-esp32-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -import-objc-header %S/Inputs/xtensa-esp32-family.h -Xcc -DEXPECT_FP=1 -Xcc -DEXPECT_CAS=1 -emit-ir -o - | %FileCheck %s --check-prefixes=COMMON,ESP32
// RUN: %target-swift-frontend %s -module-name Family -target xtensa-esp32-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -O -emit-object -o %t.esp32.o
// RUN: %target-swift-frontend %s -module-name Family -target xtensa-esp32-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -import-objc-header %S/Inputs/xtensa-esp32-family.h -Xcc -DEXPECT_FP=1 -Xcc -DEXPECT_CAS=1 -Xcc -Xclang -Xcc -target-feature -Xcc -Xclang -Xcc -windowed -Xcc -Xclang -Xcc -target-feature -Xcc -Xclang -Xcc +windowed -typecheck
// RUN: %target-swift-frontend %s -module-name Family -target xtensa-esp32-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -target-cpu esp8266 -target-cpu esp32 -typecheck
// RUN: %target-swift-frontend %s -module-name Family -target xtensa-esp32-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -Xcc -mcpu=esp8266 -Xcc -mcpu=esp32 -typecheck
// RUN: %target-swift-frontend %s -module-name Family -target xtensa-esp32-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -clang-target xtensa-esp32-none-elf -typecheck
// RUN: not %target-swift-frontend %s -module-name Family -target xtensa-esp32-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -target-cpu esp8266 -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-ESP32-CPU
// RUN: not %target-swift-frontend %s -module-name Family -target xtensa-esp32-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -Xcc -mcpu=esp32s3 -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-ESP32-XCC
// RUN: not %target-swift-frontend %s -module-name Family -target xtensa-esp32-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -direct-clang-cc1-module-build -Xcc -triple -Xcc xtensa-esp32-none-elf -Xcc -target-cpu -Xcc esp32s3 -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-ESP32-XCC
// RUN: not %target-swift-frontend %s -module-name Family -target xtensa-esp32-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -Xcc -target -Xcc xtensa-unknown-none-elf -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-ESP32-TRIPLE
// RUN: not %target-swift-frontend %s -module-name Family -target xtensa-esp32-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -clang-target xtensa-esp32s3-none-elf -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-ESP32-CLANG-TARGET
// RUN: not %target-swift-frontend %s -module-name Family -target xtensa-esp32-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -Xcc -Xclang -Xcc -target-feature -Xcc -Xclang -Xcc +windowed -Xcc -Xclang -Xcc -target-feature -Xcc -Xclang -Xcc -windowed -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-ESP32-ABI
// RUN: not %target-swift-frontend %s -target xtensa-esp32-none-elf -sdk "" -typecheck 2>&1 | %FileCheck %s --check-prefix=NEEDS-ESP32-EMBEDDED
// RUN: %target-swift-frontend -target xtensa-esp32-none-elf -sdk "" -print-target-info | %FileCheck %s --check-prefix=INFO-ESP32
// RUN: %target-swift-frontend %s -module-name Family -target xtensa-esp32s2-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -import-objc-header %S/Inputs/xtensa-esp32-family.h -Xcc -DEXPECT_FP=0 -Xcc -DEXPECT_CAS=0 -emit-ir -o - | %FileCheck %s --check-prefixes=COMMON,S2
// RUN: %target-swift-frontend %s -module-name Family -target xtensa-esp32s2-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -O -emit-object -o %t.esp32s2.o
// RUN: %target-swift-frontend %s -module-name Family -target xtensa-esp32s2-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -import-objc-header %S/Inputs/xtensa-esp32-family.h -Xcc -DEXPECT_FP=0 -Xcc -DEXPECT_CAS=0 -Xcc -Xclang -Xcc -target-feature -Xcc -Xclang -Xcc -windowed -Xcc -Xclang -Xcc -target-feature -Xcc -Xclang -Xcc +windowed -typecheck
// RUN: %target-swift-frontend %s -module-name Family -target xtensa-esp32s2-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -target-cpu esp8266 -target-cpu esp32s2 -typecheck
// RUN: %target-swift-frontend %s -module-name Family -target xtensa-esp32s2-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -Xcc -mcpu=esp8266 -Xcc -mcpu=esp32s2 -typecheck
// RUN: %target-swift-frontend %s -module-name Family -target xtensa-esp32s2-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -clang-target xtensa-esp32s2-none-elf -typecheck
// RUN: not %target-swift-frontend %s -module-name Family -target xtensa-esp32s2-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -target-cpu esp8266 -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-S2-CPU
// RUN: not %target-swift-frontend %s -module-name Family -target xtensa-esp32s2-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -Xcc -mcpu=esp32s3 -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-S2-XCC
// RUN: not %target-swift-frontend %s -module-name Family -target xtensa-esp32s2-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -direct-clang-cc1-module-build -Xcc -triple -Xcc xtensa-esp32s2-none-elf -Xcc -target-cpu -Xcc esp32s3 -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-S2-XCC
// RUN: not %target-swift-frontend %s -module-name Family -target xtensa-esp32s2-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -Xcc -target -Xcc xtensa-unknown-none-elf -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-S2-TRIPLE
// RUN: not %target-swift-frontend %s -module-name Family -target xtensa-esp32s2-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -clang-target xtensa-esp32s3-none-elf -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-S2-CLANG-TARGET
// RUN: not %target-swift-frontend %s -module-name Family -target xtensa-esp32s2-none-elf -sdk "" -enable-experimental-feature Embedded -wmo -parse-as-library -Xcc -Xclang -Xcc -target-feature -Xcc -Xclang -Xcc +windowed -Xcc -Xclang -Xcc -target-feature -Xcc -Xclang -Xcc -windowed -typecheck 2>&1 | %FileCheck %s --check-prefix=BAD-S2-ABI
// RUN: not %target-swift-frontend %s -target xtensa-esp32s2-none-elf -sdk "" -typecheck 2>&1 | %FileCheck %s --check-prefix=NEEDS-S2-EMBEDDED
// RUN: %target-swift-frontend -target xtensa-esp32s2-none-elf -sdk "" -print-target-info | %FileCheck %s --check-prefix=INFO-S2
// REQUIRES: swift_feature_Embedded
// REQUIRES: CODEGENERATOR=Xtensa

#if !arch(xtensa) || !os(none) || !_endian(little) || !_pointerBitWidth(_32)
#error("incorrect ESP32 family target configuration")
#endif

// COMMON: target datalayout = "e-m:e-p:32:32-i8:8:32-i16:16:32-i64:64-n32"
// ESP32: target triple = "xtensa-esp32-none-elf"
// S2: target triple = "xtensa-esp32s2-none-elf"
// COMMON: define {{.*}}i32 @family_add
// ESP32: "target-cpu"="esp32"
// S2: "target-cpu"="esp32s2"
// COMMON-SAME: "target-features"="{{[^"]*}}+windowed
// BAD-ESP32-CPU: invalid value 'esp8266' in '-target-cpu for xtensa-esp32-none-elf'
// BAD-ESP32-XCC: invalid value 'esp32s3' in 'CPU for xtensa-esp32-none-elf'
// BAD-ESP32-TRIPLE: invalid value 'xtensa-unknown-none-elf' in 'Clang target for xtensa-esp32-none-elf'
// BAD-ESP32-CLANG-TARGET: invalid value 'xtensa-esp32s3-none-elf' in 'Clang target for xtensa-esp32-none-elf'
// BAD-ESP32-ABI: error: ABI 'call0' is not supported for 'xtensa-esp32-none-elf'
// NEEDS-ESP32-EMBEDDED: target 'xtensa-esp32-none-elf' requires '-enable-experimental-feature Embedded'
// INFO-ESP32: "triple": "xtensa-esp32-none-elf"
// BAD-S2-CPU: invalid value 'esp8266' in '-target-cpu for xtensa-esp32s2-none-elf'
// BAD-S2-XCC: invalid value 'esp32s3' in 'CPU for xtensa-esp32s2-none-elf'
// BAD-S2-TRIPLE: invalid value 'xtensa-unknown-none-elf' in 'Clang target for xtensa-esp32s2-none-elf'
// BAD-S2-CLANG-TARGET: invalid value 'xtensa-esp32s3-none-elf' in 'Clang target for xtensa-esp32s2-none-elf'
// BAD-S2-ABI: error: ABI 'call0' is not supported for 'xtensa-esp32s2-none-elf'
// NEEDS-S2-EMBEDDED: target 'xtensa-esp32s2-none-elf' requires '-enable-experimental-feature Embedded'
// INFO-S2: "triple": "xtensa-esp32s2-none-elf"

@_cdecl("family_add")
public func add(_ a: UInt32, _ b: UInt32) -> UInt32 { a &+ b }
