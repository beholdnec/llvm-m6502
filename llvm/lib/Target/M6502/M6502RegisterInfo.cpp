// TODO: header stuff.

#include "M6502RegisterInfo.h"

#include "llvm/ADT/BitVector.h"

#include "M6502.h"
#include "M6502InstrInfo.h"
#include "M6502TargetMachine.h"
#include "MCTargetDesc/M6502MCTargetDesc.h"

#define GET_REGINFO_TARGET_DESC
#include "M6502GenRegisterInfo.inc"

namespace llvm {

M6502RegisterInfo::M6502RegisterInfo() : M6502GenRegisterInfo(0) {}

const uint16_t *
M6502RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_Normal_SaveList;
}

const uint32_t *
M6502RegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                        CallingConv::ID CC) const {
  return CSR_Normal_RegMask;
}

BitVector M6502RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  //  Reserve the stack pointer.
  Reserved.set(M6502::SPL);
  Reserved.set(M6502::SPH);
  Reserved.set(M6502::SP);

  return Reserved;
}

const TargetRegisterClass *
M6502RegisterInfo::getLargestLegalSuperClass(const TargetRegisterClass *RC,
                                             const MachineFunction &MF) const {
  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();
  if (TRI->isTypeLegalForClass(*RC, MVT::i16)) {
    return &M6502::DREGSRegClass;
  }

  if (TRI->isTypeLegalForClass(*RC, MVT::i8)) {
    return &M6502::GPR8RegClass;
  }

  llvm_unreachable("Invalid register size");
}

void M6502RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                            int SPAdj, unsigned FIOperandNum,
                                            RegScavenger *RS) const {
  // FIXME: this is utterly borked
  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &FrameInfo = MF.getFrameInfo();
  const M6502FrameLowering *TFI = getFrameLowering(MF);

  int Index = MI.getOperand(FIOperandNum).getIndex();
  MI.getOperand(FIOperandNum).ChangeToImmediate(Index); // ????
  //MI.getOperand(FIOperandNum).ChangeToImmediate(FrameInfo.getObjectOffset(Index));
}

Register M6502RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return M6502::SP;
}

} // end of namespace llvm
