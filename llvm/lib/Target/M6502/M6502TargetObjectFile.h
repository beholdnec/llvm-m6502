// TODO: header stuff.

#ifndef LLVM_M6502_TARGET_OBJECT_FILE_H
#define LLVM_M6502_TARGET_OBJECT_FILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {

/// Lowering for an M6502 ELF object file.
class M6502TargetObjectFile : public TargetLoweringObjectFileELF {

};

} // end of namespace llvm

#endif // LLVM_M6502_TARGET_OBJECT_FILE_H
