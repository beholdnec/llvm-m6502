// TODO: header stuff

#include "llvm/Support/TargetRegistry.h"

namespace llvm {
Target &getTheM6502Target() {
  static Target TheM6502Target;
  return TheM6502Target;
}
}

extern "C" void LLVMInitializeM6502TargetInfo() {
  llvm::RegisterTarget<llvm::Triple::m6502> X(llvm::getTheM6502Target(), "m6502",
                                              "MOS 6502", "M6502");
}
