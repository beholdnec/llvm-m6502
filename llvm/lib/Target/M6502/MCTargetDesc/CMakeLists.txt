add_llvm_library(LLVMM6502Desc
  M6502AsmBackend.cpp
  M6502ELFObjectWriter.cpp
  M6502ELFStreamer.cpp
  M6502InstPrinter.cpp
  M6502MCAsmInfo.cpp
  M6502MCTargetDesc.cpp
  M6502TargetStreamer.cpp
)

add_dependencies(LLVMM6502Desc M6502CommonTableGen)
