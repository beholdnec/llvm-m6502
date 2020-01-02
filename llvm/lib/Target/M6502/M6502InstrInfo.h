// TODO: header stuff.

#ifndef LLVM_M6502_INSTR_INFO_H
#define LLVM_M6502_INSTR_INFO_H

#include "llvm/CodeGen/TargetInstrInfo.h"

#include "M6502RegisterInfo.h"

#define GET_INSTRINFO_HEADER
#include "M6502GenInstrInfo.inc"
#undef GET_INSTRINFO_HEADER

namespace llvm {

/// Utilities related to the M6502 instruction set.
class M6502InstrInfo : public M6502GenInstrInfo {
public:
  explicit M6502InstrInfo();
  
  const M6502RegisterInfo &getRegisterInfo() const { return RI; }
  
  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI, unsigned SrcReg,
                           bool isKill, int FrameIndex,
                           const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI) const override;
  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI, unsigned DestReg,
                            int FrameIndex, const TargetRegisterClass *RC,
                            const TargetRegisterInfo *TRI) const override;

private:
  const M6502RegisterInfo RI;
};

} // end of namespace llvm

#endif // LLVM_M6502_INSTR_INFO_H