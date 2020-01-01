// TODO: header stuff

#include "M6502FrameLowering.h"

namespace llvm {
  
M6502FrameLowering::M6502FrameLowering()
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 1, -2) {}
    
void M6502FrameLowering::emitPrologue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  // TODO
}

void M6502FrameLowering::emitEpilogue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  // TODO
}

bool M6502FrameLowering::hasFP(const MachineFunction &MF) const {
  // TODO
  return false;
}

} // end of namespace llvm
