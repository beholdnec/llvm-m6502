// TODO: header stuff.

include "M6502InstrFormats.td"

//===----------------------------------------------------------------------===//
// AVR Specific Node Definitions
//===----------------------------------------------------------------------===//

def M6502retflag : SDNode<"M6502ISD::RET_FLAG", SDTNone,
                          [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

//===----------------------------------------------------------------------===//
// AVR Operands, Complex Patterns and Transformations Definitions.
//===----------------------------------------------------------------------===//

// FIXME: M6502 doesn't work this way. We might need a different solution...
// Address operand for `SP+imm` used by STD{W}SPQRr
def memspi : Operand<iPTR>
{
  let MIOperandInfo = (ops GPRSP, i16imm);
}

// FIXME: M6502 doesn't work this way.
// Addressing mode pattern reg+imm6
def addr : ComplexPattern<iPTR, 2, "SelectAddr", [], [SDNPWantRoot]>;

//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//
// AVR Instruction list
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Return instructions.
//===----------------------------------------------------------------------===//
let isTerminator = 1,
isReturn = 1,
isBarrier = 1 in 
{
  def RET : Pseudo<(outs),
                   (ins),
                   "ret",
                   [(M6502retflag)]>;
}

//===----------------------------------------------------------------------===//
// Data transfer instructions
//===----------------------------------------------------------------------===//

// Load immediate values into registers.
let isReMaterializable = 1 in
{
  def LDIRdK : Pseudo<(outs GPR8:$rd),
                      (ins i8imm:$k),
                      "ldi\t$rd, $k",
                      [(set i8:$rd, imm:$k)]>;
}

// 8 and 16-bit register move instructions.
let hasSideEffects = 0 in
{
  def MOVRdRr : Pseudo<(outs GPR8:$rd),
                       (ins GPR8:$rr),
                       "mov\t$rd, $rr",
                       []>;
}

//===----------------------------------------------------------------------===//
// Pseudo instructions for later expansion
//===----------------------------------------------------------------------===//

// This pseudo is either converted to a regular store or a push which clobbers
// SP.
def STDSPQRr : StorePseudo<
  (outs),
  (ins memspi:$dst, GPR8:$src),
  "stdstk\t$dst, $src",
  [(store i8:$src, addr:$dst)]
>;
