// TODO: header stuff

#ifndef LLVM_M6502_TARGET_STREAMER_H
#define LLVM_M6502_TARGET_STREAMER_H

#include "llvm/MC/MCStreamer.h"

namespace llvm {

// A generic M6502 target output stream.
class M6502TargetStreamer : public MCTargetStreamer {
public:
  explicit M6502TargetStreamer(MCStreamer &S);
};

// A target streamer for textual M6502 assembly code.
class M6502TargetAsmStreamer : public M6502TargetStreamer {
public:
  explicit M6502TargetAsmStreamer(MCStreamer &S);
};

} // end namespace llvm

#endif // LLVM_AVR_TARGET_STREAMER_H