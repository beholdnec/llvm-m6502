// TODO: header stuff

#ifndef LLVM_M6502_H
#define LLVM_M6502_H

#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class FunctionPass;
class M6502TargetMachine;

FunctionPass *createM6502ISelDag(M6502TargetMachine &TM,
                                 CodeGenOpt::Level OptLevel);
FunctionPass *createM6502Expand16BitPseudoPass();
                                 
void initializeM6502Expand16BitPseudoPass(PassRegistry&);

} // end namespace llvm

#endif // LLVM_M6502_H