// TODO: header stuff.

#ifndef LLVM_M6502_ELF_STREAMER_H
#define LLVM_M6502_ELF_STREAMER_H

#include "M6502TargetStreamer.h"

namespace llvm {

/// A target streamer for an M6502 ELF object file.
class M6502ELFStreamer : public M6502TargetStreamer {
public:
  M6502ELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);
};

} // end namespace llvm

#endif
