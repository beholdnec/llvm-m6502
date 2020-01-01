// TODO: header stuff.

#ifndef LLVM_M6502_MCINST_LOWER_H
#define LLVM_M6502_MCINST_LOWER_H

namespace llvm {

class AsmPrinter;
class MachineInstr;
class MachineOperand;
class MCContext;
class MCInst;
class MCOperand;
class MCSymbol;

/// Lowers `MachineInstr` objects into `MCInst` objects.
class M6502MCInstLower {
public:
  M6502MCInstLower(MCContext &Ctx, AsmPrinter &Printer)
      : Ctx(Ctx), Printer(Printer) {}

  /// Lowers a `MachineInstr` into a `MCInst`.
  void lowerInstruction(const MachineInstr &MI, MCInst &OutMI) const;
  MCOperand lowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym) const;
  
private:
  MCContext &Ctx;
  AsmPrinter &Printer;
};

} // end of namespace llvm

#endif // LLVM_M6502_MCINST_LOWER_H
