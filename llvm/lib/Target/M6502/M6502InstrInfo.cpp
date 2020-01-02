// TODO: header stuff.

#include "M6502InstrInfo.h"

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

#include "M6502.h"
#include "M6502TargetMachine.h"
#include "MCTargetDesc/M6502MCTargetDesc.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "M6502GenInstrInfo.inc"

namespace llvm {
  
M6502InstrInfo::M6502InstrInfo()
    : M6502GenInstrInfo(/*M6502::ADJCALLSTACKDOWN, M6502::ADJCALLSTACKUP*/), RI() {}

void M6502InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator MI,
                                         unsigned SrcReg, bool isKill,
                                         int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         const TargetRegisterInfo *TRI) const {
  MachineFunction &MF = *MBB.getParent();
  // AVRMachineFunctionInfo *AFI = MF.getInfo<AVRMachineFunctionInfo>();

  // AFI->setHasSpills(true);

  DebugLoc DL;
  if (MI != MBB.end()) {
    DL = MI->getDebugLoc();
  }

  const MachineFrameInfo &MFI = MF.getFrameInfo();

  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FrameIndex),
      MachineMemOperand::MOStore, MFI.getObjectSize(FrameIndex),
      MFI.getObjectAlignment(FrameIndex));

  unsigned Opcode = 0;
  if (TRI->isTypeLegalForClass(*RC, MVT::i8)) {
    //Opcode = M6502::STDPtrQRr;
    llvm_unreachable("TODO: implement i8 store to stack slot");
  } else if (TRI->isTypeLegalForClass(*RC, MVT::i16)) {
    Opcode = M6502::STstack16;
  } else {
    llvm_unreachable("Cannot store this register into a stack slot!");
  }

  BuildMI(MBB, MI, DL, get(Opcode))
      .addFrameIndex(FrameIndex)
      .addImm(0)
      .addReg(SrcReg, getKillRegState(isKill))
      .addMemOperand(MMO);
}

void M6502InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator MI,
                                          unsigned DestReg, int FrameIndex,
                                          const TargetRegisterClass *RC,
                                          const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (MI != MBB.end()) {
    DL = MI->getDebugLoc();
  }

  MachineFunction &MF = *MBB.getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();

  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FrameIndex),
      MachineMemOperand::MOLoad, MFI.getObjectSize(FrameIndex),
      MFI.getObjectAlignment(FrameIndex));

  unsigned Opcode = 0;
  if (TRI->isTypeLegalForClass(*RC, MVT::i8)) {
    //Opcode = AVR::LDDRdPtrQ;
    llvm_unreachable("TODO: implement LDstack for 8-bit regs");
  } else if (TRI->isTypeLegalForClass(*RC, MVT::i16)) {
    Opcode = M6502::LDstack16;
  } else {
    llvm_unreachable("Cannot load this register from a stack slot!");
  }

  BuildMI(MBB, MI, DL, get(Opcode), DestReg)
      .addFrameIndex(FrameIndex)
      .addImm(0)
      .addMemOperand(MMO);
}

} // end of namespace llvm
