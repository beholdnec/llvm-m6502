// TODO: header stuff.

#include "M6502.h"
#include "M6502InstrInfo.h"
#include "M6502TargetMachine.h"
#include "MCTargetDesc/M6502MCTargetDesc.h"

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

using namespace llvm;

#define M6502_EXPAND_16BIT_PSEUDO_NAME "M6502 16-bit pseudo instruction expansion pass"

namespace {
  
/// Expands "placeholder" instructions marked as pseudo into
/// 8-bit M6502 instructions.
class M6502Expand16BitPseudo : public MachineFunctionPass {
public:
  static char ID;
  
  M6502Expand16BitPseudo() : MachineFunctionPass(ID) {
    initializeM6502Expand16BitPseudoPass(*PassRegistry::getPassRegistry());
  }
  
  bool runOnMachineFunction(MachineFunction &MF) override;
  
  StringRef getPassName() const override { return M6502_EXPAND_16BIT_PSEUDO_NAME; }

private:
  typedef MachineBasicBlock Block;
  typedef Block::iterator BlockIt;
  
  const M6502RegisterInfo *TRI;
  const TargetInstrInfo *TII;

  bool expandMBB(Block &MBB);
  bool expandMI(Block &MBB, BlockIt MBBI);
  template <unsigned OP> bool expand(Block &MBB, BlockIt MBBI);
  
  MachineInstrBuilder buildMI(Block &MBB, BlockIt MBBI, unsigned Opcode) {
    return BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(Opcode));
  }
  
  bool expandArith(unsigned OpLo, unsigned OpHi, Block &MBB, BlockIt MBBI);
  bool expandLogic(unsigned Op, Block &MBB, BlockIt MBBI);
  bool expandLogicImm(unsigned Op, Block &MBB, BlockIt MBBI);
  bool isLogicImmOpRedundant(unsigned Op, unsigned ImmVal) const;
};

char M6502Expand16BitPseudo::ID = 0;

bool M6502Expand16BitPseudo::expandMBB(MachineBasicBlock &MBB) {
  bool Modified = false;

  BlockIt MBBI = MBB.begin(), E = MBB.end();
  while (MBBI != E) {
    BlockIt NMBBI = std::next(MBBI);
    Modified |= expandMI(MBB, MBBI);
    MBBI = NMBBI;
  }

  return Modified;
}

bool M6502Expand16BitPseudo::runOnMachineFunction(MachineFunction &MF) {
  bool Modified = false;

  const M6502Subtarget &STI = MF.getSubtarget<M6502Subtarget>();
  TRI = STI.getRegisterInfo();
  TII = STI.getInstrInfo();

  // We need to track liveness in order to use register scavenging.
  MF.getProperties().set(MachineFunctionProperties::Property::TracksLiveness);

  for (Block &MBB : MF) {
    bool ContinueExpanding = true;
    unsigned ExpandCount = 0;

    // Continue expanding the block until all pseudos are expanded.
    do {
      assert(ExpandCount < 10 && "pseudo expand limit reached");

      bool BlockModified = expandMBB(MBB);
      Modified |= BlockModified;
      ExpandCount++;

      ContinueExpanding = BlockModified;
    } while (ContinueExpanding);
  }

  return Modified;
}

template <>
bool M6502Expand16BitPseudo::expand<M6502::LDimm16>(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  unsigned OpLo, OpHi, DstLoReg, DstHiReg;
  unsigned DstReg = MI.getOperand(0).getReg();
  bool DstIsDead = MI.getOperand(0).isDead();
  OpLo = M6502::LDimm;
  OpHi = M6502::LDimm;
  TRI->splitReg(DstReg, DstLoReg, DstHiReg);

  auto MIBLO = buildMI(MBB, MBBI, OpLo)
    .addReg(DstLoReg, RegState::Define | getDeadRegState(DstIsDead));

  auto MIBHI = buildMI(MBB, MBBI, OpHi)
    .addReg(DstHiReg, RegState::Define | getDeadRegState(DstIsDead));

  switch (MI.getOperand(1).getType()) {
  case MachineOperand::MO_GlobalAddress: {
    const GlobalValue *GV = MI.getOperand(1).getGlobal();
    int64_t Offs = MI.getOperand(1).getOffset();
    unsigned TF = MI.getOperand(1).getTargetFlags();

    MIBLO.addGlobalAddress(GV, Offs, TF | M6502II::MO_LO);
    MIBHI.addGlobalAddress(GV, Offs, TF | M6502II::MO_HI);
    break;
  }
  case MachineOperand::MO_BlockAddress: {
    const BlockAddress *BA = MI.getOperand(1).getBlockAddress();
    unsigned TF = MI.getOperand(1).getTargetFlags();

    MIBLO.add(MachineOperand::CreateBA(BA, TF | M6502II::MO_LO));
    MIBHI.add(MachineOperand::CreateBA(BA, TF | M6502II::MO_HI));
    break;
  }
  case MachineOperand::MO_Immediate: {
    unsigned Imm = MI.getOperand(1).getImm();

    MIBLO.addImm(Imm & 0xff);
    MIBHI.addImm((Imm >> 8) & 0xff);
    break;
  }
  default:
    llvm_unreachable("Unknown operand type!");
  }

  MI.eraseFromParent();
  return true;
}

template <>
bool M6502Expand16BitPseudo::expand<M6502::STabs16>(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  unsigned OpLo, OpHi, SrcLoReg, SrcHiReg;
  unsigned SrcReg = MI.getOperand(1).getReg();
  bool SrcIsKill = MI.getOperand(1).isKill();
  OpLo = M6502::STabs;
  OpHi = M6502::STabs;
  TRI->splitReg(SrcReg, SrcLoReg, SrcHiReg);

  // Write the high byte first in case this address belongs to a special
  // I/O address with a special temporary register.
  // FIXME: is this needed for M6502?
  auto MIBHI = buildMI(MBB, MBBI, OpHi);
  auto MIBLO = buildMI(MBB, MBBI, OpLo);

  switch (MI.getOperand(0).getType()) {
  case MachineOperand::MO_GlobalAddress: {
    const GlobalValue *GV = MI.getOperand(0).getGlobal();
    int64_t Offs = MI.getOperand(0).getOffset();
    unsigned TF = MI.getOperand(0).getTargetFlags();

    MIBLO.addGlobalAddress(GV, Offs, TF);
    MIBHI.addGlobalAddress(GV, Offs + 1, TF);
    break;
  }
  case MachineOperand::MO_Immediate: {
    unsigned Imm = MI.getOperand(0).getImm();

    MIBLO.addImm(Imm);
    MIBHI.addImm(Imm + 1);
    break;
  }
  default:
    llvm_unreachable("Unknown operand type!");
  }

  MIBLO.addReg(SrcLoReg, getKillRegState(SrcIsKill));
  MIBHI.addReg(SrcHiReg, getKillRegState(SrcIsKill));

  MIBLO.setMemRefs(MI.memoperands());
  MIBHI.setMemRefs(MI.memoperands());

  MI.eraseFromParent();
  return true;
}

bool M6502Expand16BitPseudo::
expandLogic(unsigned Op, Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  unsigned SrcLoReg, SrcHiReg, DstLoReg, DstHiReg;
  unsigned DstReg = MI.getOperand(0).getReg();
  unsigned SrcReg = MI.getOperand(2).getReg();
  bool DstIsDead = MI.getOperand(0).isDead();
  bool DstIsKill = MI.getOperand(1).isKill();
  bool SrcIsKill = MI.getOperand(2).isKill();
  bool ImpIsDead = MI.getOperand(3).isDead();
  TRI->splitReg(SrcReg, SrcLoReg, SrcHiReg);
  TRI->splitReg(DstReg, DstLoReg, DstHiReg);

  auto MIBLO = buildMI(MBB, MBBI, Op)
    .addReg(DstLoReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstLoReg, getKillRegState(DstIsKill))
    .addReg(SrcLoReg, getKillRegState(SrcIsKill));

  // SREG is always implicitly dead
  MIBLO->getOperand(3).setIsDead();

  auto MIBHI = buildMI(MBB, MBBI, Op)
    .addReg(DstHiReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstHiReg, getKillRegState(DstIsKill))
    .addReg(SrcHiReg, getKillRegState(SrcIsKill));

  if (ImpIsDead)
    MIBHI->getOperand(3).setIsDead();

  MI.eraseFromParent();
  return true;
}

bool M6502Expand16BitPseudo::
  isLogicImmOpRedundant(unsigned Op, unsigned ImmVal) const {

  // ANDI Rd, 0xff is redundant.
  if (Op == M6502::ANDimm && ImmVal == 0xff)
    return true;

  // ORI Rd, 0x0 is redundant.
  if (Op == M6502::ORAimm && ImmVal == 0x0)
    return true;

  return false;
}

bool M6502Expand16BitPseudo::
expandLogicImm(unsigned Op, Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  unsigned DstLoReg, DstHiReg;
  unsigned DstReg = MI.getOperand(0).getReg();
  bool DstIsDead = MI.getOperand(0).isDead();
  bool SrcIsKill = MI.getOperand(1).isKill();
  bool ImpIsDead = MI.getOperand(3).isDead();
  unsigned Imm = MI.getOperand(2).getImm();
  unsigned Lo8 = Imm & 0xff;
  unsigned Hi8 = (Imm >> 8) & 0xff;
  TRI->splitReg(DstReg, DstLoReg, DstHiReg);

  if (!isLogicImmOpRedundant(Op, Lo8)) {
    auto MIBLO = buildMI(MBB, MBBI, Op)
      .addReg(DstLoReg, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(DstLoReg, getKillRegState(SrcIsKill))
      .addImm(Lo8);

    // SREG is always implicitly dead
    MIBLO->getOperand(3).setIsDead();
  }

  if (!isLogicImmOpRedundant(Op, Hi8)) {
    auto MIBHI = buildMI(MBB, MBBI, Op)
      .addReg(DstHiReg, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(DstHiReg, getKillRegState(SrcIsKill))
      .addImm(Hi8);

    if (ImpIsDead)
      MIBHI->getOperand(3).setIsDead();
  }

  MI.eraseFromParent();
  return true;
}

template <>
bool M6502Expand16BitPseudo::expand<M6502::ANDreg16>(Block &MBB, BlockIt MBBI) {
  return expandLogic(M6502::ANDreg, MBB, MBBI);
}

template <>
bool M6502Expand16BitPseudo::expand<M6502::ANDimm16>(Block &MBB, BlockIt MBBI) {
  return expandLogicImm(M6502::ANDimm, MBB, MBBI);
}

template <>
bool M6502Expand16BitPseudo::expand<M6502::ORAreg16>(Block &MBB, BlockIt MBBI) {
  return expandLogic(M6502::ORAreg, MBB, MBBI);
}

template <>
bool M6502Expand16BitPseudo::expand<M6502::ORAimm16>(Block &MBB, BlockIt MBBI) {
  return expandLogicImm(M6502::ORAimm, MBB, MBBI);
}

template <>
bool M6502Expand16BitPseudo::expand<M6502::EORreg16>(Block &MBB, BlockIt MBBI) {
  return expandLogic(M6502::EORreg, MBB, MBBI);
}

bool M6502Expand16BitPseudo::
expandArith(unsigned OpLo, unsigned OpHi, Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  unsigned SrcLoReg, SrcHiReg, DstLoReg, DstHiReg;
  unsigned DstReg = MI.getOperand(0).getReg();
  unsigned SrcReg = MI.getOperand(2).getReg();
  bool DstIsDead = MI.getOperand(0).isDead();
  bool DstIsKill = MI.getOperand(1).isKill();
  bool SrcIsKill = MI.getOperand(2).isKill();
  bool ImpIsDead = MI.getOperand(3).isDead();
  TRI->splitReg(SrcReg, SrcLoReg, SrcHiReg);
  TRI->splitReg(DstReg, DstLoReg, DstHiReg);

  buildMI(MBB, MBBI, OpLo)
    .addReg(DstLoReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstLoReg, getKillRegState(DstIsKill))
    .addReg(SrcLoReg, getKillRegState(SrcIsKill));

  auto MIBHI = buildMI(MBB, MBBI, OpHi)
    .addReg(DstHiReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstHiReg, getKillRegState(DstIsKill))
    .addReg(SrcHiReg, getKillRegState(SrcIsKill));

  if (ImpIsDead)
    MIBHI->getOperand(3).setIsDead();

  // SREG is always implicitly killed
  MIBHI->getOperand(4).setIsKill();

  MI.eraseFromParent();
  return true;
}

template <>
bool M6502Expand16BitPseudo::expand<M6502::AD0reg16>(Block &MBB, BlockIt MBBI) {
  return expandArith(M6502::AD0reg, M6502::ADCreg, MBB, MBBI);
}

template <>
bool M6502Expand16BitPseudo::expand<M6502::ADCreg16>(Block &MBB, BlockIt MBBI) {
  return expandArith(M6502::ADCreg, M6502::ADCreg, MBB, MBBI);
}

template <>
bool M6502Expand16BitPseudo::expand<M6502::SB1reg16>(Block &MBB, BlockIt MBBI) {
  return expandArith(M6502::SB1reg, M6502::SBCreg, MBB, MBBI);
}

template <>
bool M6502Expand16BitPseudo::expand<M6502::SBCreg16>(Block &MBB, BlockIt MBBI) {
  return expandArith(M6502::SBCreg, M6502::SBCreg, MBB, MBBI);
}

template <> bool M6502Expand16BitPseudo::expand<M6502::SEXT>(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  unsigned DstLoReg, DstHiReg;
  // sext R17:R16, R17
  // mov     r16, r17
  // lsl     r17
  // sbc     r17, r17
  // sext R17:R16, R13
  // mov     r16, r13
  // mov     r17, r13
  // lsl     r17
  // sbc     r17, r17
  // sext R17:R16, R16
  // mov     r17, r16
  // lsl     r17
  // sbc     r17, r17
  unsigned DstReg = MI.getOperand(0).getReg();
  unsigned SrcReg = MI.getOperand(1).getReg();
  bool DstIsDead = MI.getOperand(0).isDead();
  bool SrcIsKill = MI.getOperand(1).isKill();
  bool ImpIsDead = MI.getOperand(2).isDead();
  TRI->splitReg(DstReg, DstLoReg, DstHiReg);

  if (SrcReg != DstLoReg) {
    auto MOV = buildMI(MBB, MBBI, M6502::Treg)
      .addReg(DstLoReg, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(SrcReg);

    if (SrcReg == DstHiReg) {
      MOV->getOperand(1).setIsKill();
    }
  }

  if (SrcReg != DstHiReg) {
    buildMI(MBB, MBBI, M6502::Treg)
      .addReg(DstHiReg, RegState::Define)
      .addReg(SrcReg, getKillRegState(SrcIsKill));
  }

  buildMI(MBB, MBBI, M6502::AD0reg) // LSL Rd <==> ADD Rd, Rr
    .addReg(DstHiReg, RegState::Define)
    .addReg(DstHiReg)
    .addReg(DstHiReg, RegState::Kill);

  auto SBC = buildMI(MBB, MBBI, M6502::SBCreg)
    .addReg(DstHiReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstHiReg, RegState::Kill)
    .addReg(DstHiReg, RegState::Kill);

  if (ImpIsDead)
    SBC->getOperand(3).setIsDead();

  // SREG is always implicitly killed
  SBC->getOperand(4).setIsKill();

  MI.eraseFromParent();
  return true;
}

template <> bool M6502Expand16BitPseudo::expand<M6502::ZEXT>(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  unsigned DstLoReg, DstHiReg;
  // zext R25:R24, R20
  // mov      R24, R20
  // eor      R25, R25
  // zext R25:R24, R24
  // eor      R25, R25
  // zext R25:R24, R25
  // mov      R24, R25
  // eor      R25, R25
  unsigned DstReg = MI.getOperand(0).getReg();
  unsigned SrcReg = MI.getOperand(1).getReg();
  bool DstIsDead = MI.getOperand(0).isDead();
  bool SrcIsKill = MI.getOperand(1).isKill();
  bool ImpIsDead = MI.getOperand(2).isDead();
  TRI->splitReg(DstReg, DstLoReg, DstHiReg);

  if (SrcReg != DstLoReg) {
    buildMI(MBB, MBBI, M6502::Treg)
      .addReg(DstLoReg, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(SrcReg, getKillRegState(SrcIsKill));
  }

  auto EOR = buildMI(MBB, MBBI, M6502::EORreg)
    .addReg(DstHiReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstHiReg, RegState::Kill)
    .addReg(DstHiReg, RegState::Kill);

  if (ImpIsDead)
    EOR->getOperand(3).setIsDead();

  MI.eraseFromParent();
  return true;
}

bool M6502Expand16BitPseudo::expandMI(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  int Opcode = MBBI->getOpcode();

#define EXPAND(Op)               \
  case Op:                       \
    return expand<Op>(MBB, MI)

  switch (Opcode) {
    EXPAND(M6502::LDimm16);
    EXPAND(M6502::STabs16);
    EXPAND(M6502::AD0reg16);
    EXPAND(M6502::ADCreg16);
    EXPAND(M6502::SB1reg16);
    EXPAND(M6502::SBCreg16);
    EXPAND(M6502::ANDreg16);
    EXPAND(M6502::ANDimm16);
    EXPAND(M6502::ORAreg16);
    EXPAND(M6502::ORAimm16);
    EXPAND(M6502::EORreg16);
    EXPAND(M6502::SEXT);
    EXPAND(M6502::ZEXT);
  }
#undef EXPAND
  return false;
}

} // end of anonymous namespace

INITIALIZE_PASS(M6502Expand16BitPseudo, "m6502-expand-16bit-pseudo",
                M6502_EXPAND_16BIT_PSEUDO_NAME, false, false)
namespace llvm {

FunctionPass *createM6502Expand16BitPseudoPass() { return new M6502Expand16BitPseudo(); }

} // end of namespace llvm
