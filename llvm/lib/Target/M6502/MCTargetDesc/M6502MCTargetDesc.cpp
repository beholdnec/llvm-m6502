// TODO: header stuff

#include "M6502MCTargetDesc.h"

#include "M6502ELFStreamer.h"
#include "M6502InstPrinter.h"
#include "M6502MCAsmInfo.h"
#include "M6502TargetStreamer.h"
#include "TargetInfo/M6502TargetInfo.h"

#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "M6502GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "M6502GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "M6502GenRegisterInfo.inc"

using namespace llvm;

MCInstrInfo *llvm::createM6502MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitM6502MCInstrInfo(X);

  return X;
}

static MCRegisterInfo *createM6502MCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitM6502MCRegisterInfo(X, 0);

  return X;
}

static MCSubtargetInfo *createM6502MCSubtargetInfo(const Triple &TT,
                                                 StringRef CPU, StringRef FS) {
  return createM6502MCSubtargetInfoImpl(TT, CPU, FS);
}

static MCInstPrinter *createM6502MCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0) {
    return new M6502InstPrinter(MAI, MII, MRI);
  }

  return nullptr;
}

static MCTargetStreamer *
createM6502ObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  return new M6502ELFStreamer(S, STI);
}

static MCTargetStreamer *createMCAsmTargetStreamer(MCStreamer &S,
                                                   formatted_raw_ostream &OS,
                                                   MCInstPrinter *InstPrint,
                                                   bool isVerboseAsm) {
  return new M6502TargetAsmStreamer(S);
}

extern "C" void LLVMInitializeM6502TargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfo<M6502MCAsmInfo> X(getTheM6502Target());

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(getTheM6502Target(), createM6502MCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(getTheM6502Target(), createM6502MCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(getTheM6502Target(),
                                          createM6502MCSubtargetInfo);
  
  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(getTheM6502Target(),
                                        createM6502MCInstPrinter);

  // Register the obj target streamer.
  TargetRegistry::RegisterObjectTargetStreamer(getTheM6502Target(),
                                               createM6502ObjectTargetStreamer);

  // Register the asm target streamer.
  TargetRegistry::RegisterAsmTargetStreamer(getTheM6502Target(),
                                            createMCAsmTargetStreamer);

  // Register the asm backend (as little endian).
  TargetRegistry::RegisterMCAsmBackend(getTheM6502Target(), createM6502AsmBackend);
}
