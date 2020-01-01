// TODO: header stuff

#include "M6502Subtarget.h"

#include "M6502TargetMachine.h"
#include "MCTargetDesc/M6502MCTargetDesc.h"

#define DEBUG_TYPE "m6502-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "M6502GenSubtargetInfo.inc"

namespace llvm {
  
M6502Subtarget::M6502Subtarget(const Triple &TT, const std::string &CPU,
                               const std::string &FS, const M6502TargetMachine &TM)
    : M6502GenSubtargetInfo(TT, CPU, FS), InstrInfo(), FrameLowering(),
      TLInfo(TM, initializeSubtargetDependencies(CPU, FS, TM)),

      // Subtarget features
      m_hasNoDecimal(false), m_FeatureSetDummy(false) {
  // Parse features string.
  ParseSubtargetFeatures(CPU, FS);
}

M6502Subtarget &
M6502Subtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS,
                                                const TargetMachine &TM) {
  // Parse features string.
  ParseSubtargetFeatures(CPU, FS);
  return *this;
}

} // end of namespace llvm
