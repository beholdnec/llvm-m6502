// TODO: header stuff

#ifndef LLVM_M6502_MCTARGET_DESC_H
#define LLVM_M6502_MCTARGET_DESC_H

#include "llvm/Support/DataTypes.h"

#include <memory>

namespace llvm {

class MCAsmBackend;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;

MCInstrInfo *createM6502MCInstrInfo();

/// Creates an assembly backend for M6502.
MCAsmBackend *createM6502AsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                    const MCRegisterInfo &MRI,
                                    const llvm::MCTargetOptions &TO);
                                    
/// Creates an ELF object writer for AVR.
std::unique_ptr<MCObjectTargetWriter> createM6502ELFObjectWriter(uint8_t OSABI);

} // end namespace llvm

#define GET_REGINFO_ENUM
#include "M6502GenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "M6502GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "M6502GenSubtargetInfo.inc"

#endif // LLVM_M6502_MCTARGET_DESC_H
