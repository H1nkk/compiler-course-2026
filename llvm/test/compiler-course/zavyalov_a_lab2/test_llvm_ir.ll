; RUN: opt -load-pass-plugin %llvmshlibdir/zavyalov_a_lab2_LLVM_IR%pluginext\
; RUN: -passes=IcmpReplacer -S %s | FileCheck %s

define dso_local noundef zeroext i1 @_Z14SignedLessThanii(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z14SignedLessThanii(
; CHECK: %cmp.sge = icmp sge i32 %0, %1
; CHECK-NEXT: %cmp.sge.not = xor i1 %cmp.sge, true
; CHECK-NEXT: ret i1 %cmp.sge.not
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4

  %cmp = icmp slt i32 %0, %1
  ret i1 %cmp
}

define dso_local noundef zeroext i1 @_Z17SignedGreaterThanii(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z17SignedGreaterThanii(
; CHECK: %cmp.sle = icmp sle i32 %0, %1
; CHECK-NEXT: %cmp.sle.not = xor i1 %cmp.sle, true
; CHECK-NEXT: ret i1 %cmp.sle.not
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
  %cmp = icmp sgt i32 %0, %1
  ret i1 %cmp
}

define dso_local noundef zeroext i1 @_Z19SignedLessEqualThanii(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z19SignedLessEqualThanii(
; CHECK: %cmp.sgt = icmp sgt i32 %0, %1
; CHECK-NEXT: %cmp.sgt.not = xor i1 %cmp.sgt, true
; CHECK-NEXT: ret i1 %cmp.sgt.not
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
  %cmp = icmp sle i32 %0, %1
  ret i1 %cmp
}

define dso_local noundef zeroext i1 @_Z22SignedGreaterEqualThanii(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z22SignedGreaterEqualThanii(
; CHECK: %cmp.slt = icmp slt i32 %0, %1
; CHECK-NEXT: %cmp.slt.not = xor i1 %cmp.slt, true
; CHECK-NEXT: ret i1 %cmp.slt.not
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
  %cmp = icmp sge i32 %0, %1
  ret i1 %cmp
}

define dso_local noundef zeroext i1 @_Z16UnsignedLessThanjj(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z16UnsignedLessThanjj(
; CHECK: %cmp.uge = icmp uge i32 %0, %1
; CHECK-NEXT: %cmp.uge.not = xor i1 %cmp.uge, true
; CHECK-NEXT: ret i1 %cmp.uge.not
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
  %cmp = icmp ult i32 %0, %1
  ret i1 %cmp
}

define dso_local noundef zeroext i1 @_Z19UnsignedGreaterThanjj(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z19UnsignedGreaterThanjj(
; CHECK: %cmp.ule = icmp ule i32 %0, %1
; CHECK-NEXT: %cmp.ule.not = xor i1 %cmp.ule, true
; CHECK-NEXT: ret i1 %cmp.ule.not
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
  %cmp = icmp ugt i32 %0, %1
  ret i1 %cmp
}

define dso_local noundef zeroext i1 @_Z21UnsignedLessEqualThanjj(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z21UnsignedLessEqualThanjj(
; CHECK: %cmp.ugt = icmp ugt i32 %0, %1
; CHECK-NEXT: %cmp.ugt.not = xor i1 %cmp.ugt, true
; CHECK-NEXT: ret i1 %cmp.ugt.not
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
  %cmp = icmp ule i32 %0, %1
  ret i1 %cmp
}

define dso_local noundef zeroext i1 @_Z24UnsignedGreaterEqualThanjj(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z24UnsignedGreaterEqualThanjj(
; CHECK: %cmp.ult = icmp ult i32 %0, %1
; CHECK-NEXT: %cmp.ult.not = xor i1 %cmp.ult, true
; CHECK-NEXT: ret i1 %cmp.ult.not
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
  %cmp = icmp uge i32 %0, %1
  ret i1 %cmp
}