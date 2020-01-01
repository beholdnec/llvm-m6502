// TODO: header stuff

#ifndef LLVM_M6502_ASM_INFO_H
#define LLVM_M6502_ASM_INFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {

class Triple;

/// Specifies the format of M6502 assembly files.
class M6502MCAsmInfo : public MCAsmInfo {
public:
  explicit M6502MCAsmInfo(const Triple &TT);
};

} // end namespace llvm

#endif // LLVM_M6502_ASM_INFO_H