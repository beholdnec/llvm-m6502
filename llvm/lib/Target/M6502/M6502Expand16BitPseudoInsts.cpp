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

bool M6502Expand16BitPseudo::expandMI(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  int Opcode = MBBI->getOpcode();

#define EXPAND(Op)               \
  case Op:                       \
    return expand<Op>(MBB, MI)

  switch (Opcode) {
    EXPAND(M6502::LDimm16);
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
