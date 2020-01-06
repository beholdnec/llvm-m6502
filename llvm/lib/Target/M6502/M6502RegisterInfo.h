// TODO: header stuff.

#ifndef LLVM_M6502_REGISTER_INFO_H
#define LLVM_M6502_REGISTER_INFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "M6502GenRegisterInfo.inc"

namespace llvm {

/// Utilities relating to M6502 registers.
class M6502RegisterInfo : public M6502GenRegisterInfo {
public:
  M6502RegisterInfo();

  const uint16_t *
  getCalleeSavedRegs(const MachineFunction *MF = 0) const override;
  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;
  
  const TargetRegisterClass *
  getLargestLegalSuperClass(const TargetRegisterClass *RC,
                            const MachineFunction &MF) const override;
  
  /// Stack Frame Processing Methods
  void eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = NULL) const override;
                           
  Register getFrameRegister(const MachineFunction &MF) const override;
  
  /// Splits a 16-bit `DREGS` register into the lo/hi register pair.
  /// \param Reg A 16-bit register to split.
  void splitReg(unsigned Reg, unsigned &LoReg, unsigned &HiReg) const;
};

} // end of namespace llvm

#endif // LLVM_M6502_REGISTER_INFO_H
