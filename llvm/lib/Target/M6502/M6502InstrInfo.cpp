// TODO: header stuff.

#include "M6502InstrInfo.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "M6502GenInstrInfo.inc"

namespace llvm {
  
M6502InstrInfo::M6502InstrInfo()
    : M6502GenInstrInfo(/*M6502::ADJCALLSTACKDOWN, M6502::ADJCALLSTACKUP*/), RI() {}

} // end of namespace llvm
