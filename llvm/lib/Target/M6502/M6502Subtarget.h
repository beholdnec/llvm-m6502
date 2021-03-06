// TODO: header stuff

#ifndef LLVM_M6502_SUBTARGET_H
#define LLVM_M6502_SUBTARGET_H

#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"

#include "M6502FrameLowering.h"
#include "M6502ISelLowering.h"
#include "M6502InstrInfo.h"

#define GET_SUBTARGETINFO_HEADER
#include "M6502GenSubtargetInfo.inc"

namespace llvm {

class M6502TargetMachine;

/// A specific M6502 target device.
class M6502Subtarget : public M6502GenSubtargetInfo {
public:
  //! Creates an AVR subtarget.
  //! \param TT  The target triple.
  //! \param CPU The CPU to target.
  //! \param FS  The feature string.
  //! \param TM  The target machine.
  M6502Subtarget(const Triple &TT, const std::string &CPU, const std::string &FS,
                 const M6502TargetMachine &TM);
                 
  const M6502InstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const TargetFrameLowering *getFrameLowering() const override { return &FrameLowering; }
  const M6502TargetLowering *getTargetLowering() const override { return &TLInfo; }
  const M6502RegisterInfo *getRegisterInfo() const override { return &InstrInfo.getRegisterInfo(); }

  /// Parses a subtarget feature string, setting appropriate options.
  /// \note Definition of function is auto generated by `tblgen`.
  void ParseSubtargetFeatures(StringRef CPU, StringRef FS);
  
  M6502Subtarget &initializeSubtargetDependencies(StringRef CPU, StringRef FS,
                                                  const TargetMachine &TM);

private:
  M6502InstrInfo InstrInfo;
  M6502FrameLowering FrameLowering;
  M6502TargetLowering TLInfo;

  // Subtarget feature settings
  // See M6502.td for details.
  bool m_hasNoDecimal;

  // Dummy member, used by FeatureSet's. We cannot have a SubtargetFeature with
  // no variable, so we instead bind pseudo features to this variable.
  bool m_FeatureSetDummy;
};

} // end namespace llvm

#endif // LLVM_M6502_SUBTARGET_H
