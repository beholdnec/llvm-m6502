// TODO: header stuff.

#ifndef LLVM_M6502_FRAME_LOWERING_H
#define LLVM_M6502_FRAME_LOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

/// Utilities for creating function call frames.
class M6502FrameLowering : public TargetFrameLowering {
public:
  explicit M6502FrameLowering();
  
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  bool hasFP(const MachineFunction &MF) const override;
  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override;
};

} // end of namespace llvm

#endif // LLVM_M6502_FRAME_LOWERING_H