// TODO: header stuff.

#ifndef LLVM_M6502_INSTR_INFO_H
#define LLVM_M6502_INSTR_INFO_H

#include "llvm/CodeGen/TargetInstrInfo.h"

#include "M6502RegisterInfo.h"

#define GET_INSTRINFO_HEADER
#include "M6502GenInstrInfo.inc"
#undef GET_INSTRINFO_HEADER

namespace llvm {

namespace M6502CC {

/// M6502 specific condition codes.
/// These correspond to `M6502_*_COND` in `M6502InstrInfo.td`.
/// They must be kept in synch.
enum CondCodes {
  COND_EQ, //!< Equal
  COND_NE, //!< Not equal
  COND_GE, //!< Greater than or equal
  COND_LT, //!< Less than
  COND_SH, //!< Unsigned same or higher
  COND_LO, //!< Unsigned lower
  COND_MI, //!< Minus
  COND_PL, //!< Plus
  COND_INVALID
};

} // end of namespace M6502CC

/// Utilities related to the M6502 instruction set.
class M6502InstrInfo : public M6502GenInstrInfo {
public:
  explicit M6502InstrInfo();
  
  const M6502RegisterInfo &getRegisterInfo() const { return RI; }
  
  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, unsigned DestReg, unsigned SrcReg,
                   bool KillSrc) const override;
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