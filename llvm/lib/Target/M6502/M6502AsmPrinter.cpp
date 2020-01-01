// TODO: header stuff

#include "M6502MCInstLower.h"
#include "TargetInfo/M6502TargetInfo.h"

#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/TargetRegistry.h"

#define DEBUG_TYPE "m6502-asm-printer"

namespace llvm {

/// An M6502 assembly code printer.
class M6502AsmPrinter : public AsmPrinter {
public:
  M6502AsmPrinter(TargetMachine &TM,
                  std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) { }

  StringRef getPassName() const override { return "M6502 Assembly Printer"; }

  void EmitInstruction(const MachineInstr *MI) override;
};

void M6502AsmPrinter::EmitInstruction(const MachineInstr *MI) {
  M6502MCInstLower MCInstLowering(OutContext, *this);

  MCInst I;
  MCInstLowering.lowerInstruction(*MI, I);
  EmitToStreamer(*OutStreamer, I);
}

} // end of namespace llvm

extern "C" void LLVMInitializeM6502AsmPrinter() {
  llvm::RegisterAsmPrinter<llvm::M6502AsmPrinter> X(llvm::getTheM6502Target());
}
