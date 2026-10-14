; ModuleID = 'main_module'
source_filename = "main_module"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

define i32 @main() {
entry:
  %myVar = alloca i32, align 4
  store i32 0, ptr %myVar, align 4
}
