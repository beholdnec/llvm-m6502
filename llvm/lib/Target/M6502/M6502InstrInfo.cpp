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
    : M6502GenInstrInfo(M6502::ADJCALLSTACKDOWN, M6502::ADJCALLSTACKUP), RI() {}

void M6502InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 const DebugLoc &DL, unsigned DestReg,
                                 unsigned SrcReg, bool KillSrc) const {
  const M6502Subtarget &STI = MBB.getParent()->getSubtarget<M6502Subtarget>();
  const M6502RegisterInfo &TRI = *STI.getRegisterInfo();
  unsigned Opc;

  if (M6502::DREGSRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MI, DL, get(M6502::Treg16), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else {
    if (M6502::GPR8RegClass.contains(DestReg, SrcReg)) {
      Opc = M6502::Treg;
    } else {
      llvm_unreachable("Impossible reg-to-reg copy");
    }

    BuildMI(MBB, MI, DL, get(Opc), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  }
}

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
    Opcode = M6502::STstk;
  } else if (TRI->isTypeLegalForClass(*RC, MVT::i16)) {
    Opcode = M6502::STstk16;
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
    Opcode = M6502::LDstk;
  } else if (TRI->isTypeLegalForClass(*RC, MVT::i16)) {
    Opcode = M6502::LDstk16;
  } else {
    llvm_unreachable("Cannot load this register from a stack slot!");
  }

  BuildMI(MBB, MI, DL, get(Opcode), DestReg)
      .addFrameIndex(FrameIndex)
      .addImm(0)
      .addMemOperand(MMO);
}

} // end of namespace llvm
