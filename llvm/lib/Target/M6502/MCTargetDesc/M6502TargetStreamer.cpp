// TODO: header stuff

#include "M6502TargetStreamer.h"

namespace llvm {

M6502TargetStreamer::M6502TargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

M6502TargetAsmStreamer::M6502TargetAsmStreamer(MCStreamer &S)
    : M6502TargetStreamer(S) {}

} // end namespace llvm
