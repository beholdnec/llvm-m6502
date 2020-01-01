// TODO: header stuff.

#include "M6502InstPrinter.h"

#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/Support/FormattedStream.h"

#define DEBUG_TYPE "asm-printer"

namespace llvm {
  
// Include the auto-generated portion of the assembly writer.
#define PRINT_ALIAS_INSTR
#include "M6502GenAsmWriter.inc"
  
void M6502InstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                                 StringRef Annot, const MCSubtargetInfo &STI) {
  unsigned Opcode = MI->getOpcode();

  switch (Opcode) {
  default:
    if (!printAliasInstr(MI, O))
      printInstruction(MI, O);

    printAnnotation(O, Annot);
    break;
  }
}

const char *M6502InstPrinter::getPrettyRegisterName(unsigned RegNum,
                                                    MCRegisterInfo const &MRI) {
  // GCC prints register pairs by just printing the lower register
  // If the register contains a subregister, print it instead
  // if (MRI.getNumSubRegIndices() > 0) {
  //   unsigned RegLoNum = MRI.getSubReg(RegNum, AVR::sub_lo);
  //   RegNum = (RegLoNum != M6502::NoRegister) ? RegLoNum : RegNum;
  // }

  return getRegisterName(RegNum);
}

void M6502InstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                    raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  const MCOperandInfo &MOI = this->MII.get(MI->getOpcode()).OpInfo[OpNo];

  if (Op.isReg()) {
    O << getPrettyRegisterName(Op.getReg(), MRI);
  } else if (Op.isImm()) {
    O << Op.getImm();
  } else {
    assert(Op.isExpr() && "Unknown operand kind in printOperand");
    O << *Op.getExpr();
  }
}

} // end of namespace llvm
