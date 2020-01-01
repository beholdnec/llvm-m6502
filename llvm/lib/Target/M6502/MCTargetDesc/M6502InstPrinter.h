// TODO: header stuff

#ifndef LLVM_M6502_INST_PRINTER_H
#define LLVM_M6502_INST_PRINTER_H

#include "llvm/MC/MCInstPrinter.h"

namespace llvm {

/// Prints M6502 instructions to a textual stream.
class M6502InstPrinter : public MCInstPrinter {
public:
  M6502InstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                   const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}
      
  void printInst(const MCInst *MI, raw_ostream &O, StringRef Annot,
                 const MCSubtargetInfo &STI) override;
};
  
} // end namespace llvm

#endif // LLVM_M6502_INST_PRINTER_H
