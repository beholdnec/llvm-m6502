// TODO: header stuff.

#include "M6502ELFStreamer.h"

namespace llvm {

M6502ELFStreamer::M6502ELFStreamer(MCStreamer &S,
                                   const MCSubtargetInfo &STI)
    : M6502TargetStreamer(S) {
  // TODO
}

} // end namespace llvm
