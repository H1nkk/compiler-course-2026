; RUN: opt -load-pass-plugin %llvmshlibdir/zavyalov_a_lab2_LLVM_IR%pluginext\
; RUN: -passes=IcmpReplacer -S %s | FileCheck %s

define dso_local noundef zeroext i1 @_Z14SignedLessThanii(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z14SignedLessThanii(
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
; CHECK: %cmp.sge = icmp sge i32 %0, %1
; CHECK: %cmp.sge.not = xor i1 %cmp.sge, true
; CHECK: ret i1 %cmp.sge.not
  %cmp = icmp slt i32 %0, %1
  ret i1 %cmp
}

define dso_local noundef zeroext i1 @_Z17SignedGreaterThanii(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z17SignedGreaterThanii(
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
; CHECK: %cmp.sle = icmp sle i32 %0, %1
; CHECK: %cmp.sle.not = xor i1 %cmp.sle, true
; CHECK: ret i1 %cmp.sle.not
  %cmp = icmp sgt i32 %0, %1
  ret i1 %cmp
}

define dso_local noundef zeroext i1 @_Z19SignedLessEqualThanii(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z19SignedLessEqualThanii(
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
; CHECK: %cmp.sgt = icmp sgt i32 %0, %1
; CHECK: %cmp.sgt.not = xor i1 %cmp.sgt, true
; CHECK: ret i1 %cmp.sgt.not
  %cmp = icmp sle i32 %0, %1
  ret i1 %cmp
}

define dso_local noundef zeroext i1 @_Z22SignedGreaterEqualThanii(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z22SignedGreaterEqualThanii(
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
; CHECK: %cmp.slt = icmp slt i32 %0, %1
; CHECK: %cmp.slt.not = xor i1 %cmp.slt, true
; CHECK: ret i1 %cmp.slt.not
  %cmp = icmp sge i32 %0, %1
  ret i1 %cmp
}

define dso_local noundef zeroext i1 @_Z16UnsignedLessThanjj(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z16UnsignedLessThanjj(
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
; CHECK: %cmp.uge = icmp uge i32 %0, %1
; CHECK: %cmp.uge.not = xor i1 %cmp.uge, true
; CHECK: ret i1 %cmp.uge.not
  %cmp = icmp ult i32 %0, %1
  ret i1 %cmp
}

define dso_local noundef zeroext i1 @_Z19UnsignedGreaterThanjj(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z19UnsignedGreaterThanjj(
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
; CHECK: %cmp.ule = icmp ule i32 %0, %1
; CHECK: %cmp.ule.not = xor i1 %cmp.ule, true
; CHECK: ret i1 %cmp.ule.not
  %cmp = icmp ugt i32 %0, %1
  ret i1 %cmp
}

define dso_local noundef zeroext i1 @_Z21UnsignedLessEqualThanjj(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z21UnsignedLessEqualThanjj(
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
; CHECK: %cmp.ugt = icmp ugt i32 %0, %1
; CHECK: %cmp.ugt.not = xor i1 %cmp.ugt, true
; CHECK: ret i1 %cmp.ugt.not
  %cmp = icmp ule i32 %0, %1
  ret i1 %cmp
}

define dso_local noundef zeroext i1 @_Z24UnsignedGreaterEqualThanjj(i32 noundef %a, i32 noundef %b) #0 {
; CHECK-LABEL: @_Z24UnsignedGreaterEqualThanjj(
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
; CHECK: %cmp.ult = icmp ult i32 %0, %1
; CHECK: %cmp.ult.not = xor i1 %cmp.ult, true
; CHECK: ret i1 %cmp.ult.not
  %cmp = icmp uge i32 %0, %1
  ret i1 %cmp
}