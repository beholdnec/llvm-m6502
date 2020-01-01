// TODO: header stuff

#ifndef LLVM_M6502_H
#define LLVM_M6502_H

#include "llvm/CodeGen/SelectionDAGNodes.h"

namespace llvm {

class FunctionPass;
class M6502TargetMachine;

FunctionPass *createM6502ISelDag(M6502TargetMachine &TM,
                                 CodeGenOpt::Level OptLevel);

} // end namespace llvm

#endif // LLVM_M6502_H