set(LLVM_TARGET_DEFINITIONS M6502.td)

tablegen(LLVM M6502GenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM M6502GenCallingConv.inc -gen-callingconv)
tablegen(LLVM M6502GenDAGISel.inc -gen-dag-isel)
tablegen(LLVM M6502GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM M6502GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM M6502GenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(M6502CommonTableGen)

add_llvm_target(M6502CodeGen
  M6502AsmPrinter.cpp
  M6502Expand16BitPreISel.cpp
  M6502Expand16BitPseudoInsts.cpp
  M6502FrameLowering.cpp
  M6502InstrInfo.cpp
  M6502ISelDAGToDAG.cpp
  M6502ISelLowering.cpp
  M6502MCInstLower.cpp
  M6502RegisterInfo.cpp
  M6502Subtarget.cpp
  M6502TargetMachine.cpp
  
  DEPENDS
  intrinsics_gen
)

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
