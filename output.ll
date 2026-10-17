; ModuleID = 'main_module'
source_filename = "main_module"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@0 = private unnamed_addr constant [9 x i8] c"methinks\00", align 1

define dso_local i32 @main() {
entry:
  %myVar = alloca i32, align 4
  store i32 579, ptr %myVar, align 4
  %0 = call i32 (ptr, i64, ...) @delight_print(ptr @0, i64 8)
  ret i32 0
}

declare i32 @delight_print(ptr, i64, ...)
