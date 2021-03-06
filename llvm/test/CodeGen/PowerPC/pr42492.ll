; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mtriple=powerpc64-unknown-linux-gnu -mcpu=pwr9 < %s | FileCheck %s

define void @f(i8*, i8*, i64*) {
; Check we don't assert and this is not a Hardware Loop
; CHECK-LABEL: f:
; CHECK:  .LBB0_2: #
; CHECK-NEXT:    cmplwi
; CHECK-NEXT:    cmpd
; CHECK-NEXT:    sldi
; CHECK-NEXT:    cror
; CHECK-NEXT:    addi
; CHECK-NEXT:    bc

  %4 = icmp eq i8* %0, %1
  br i1 %4, label %9, label %5

5:                                                ; preds = %3
  %6 = getelementptr inbounds i64, i64* %2, i64 1
  %7 = load i64, i64* %6, align 8
  br label %10

8:                                                ; preds = %10
  store i64 %14, i64* %6, align 8
  br label %9

9:                                                ; preds = %8, %3
  ret void

10:                                               ; preds = %5, %10
  %11 = phi i64 [ %7, %5 ], [ %14, %10 ]
  %12 = phi i32 [ 0, %5 ], [ %15, %10 ]
  %13 = phi i8* [ %0, %5 ], [ %16, %10 ]
  %14 = shl nsw i64 %11, 4
  %15 = add nuw nsw i32 %12, 1
  %16 = getelementptr inbounds i8, i8* %13, i64 1
  %17 = icmp ugt i32 %12, 14
  %18 = icmp eq i8* %16, %1
  %19 = or i1 %18, %17
  br i1 %19, label %8, label %10
}
