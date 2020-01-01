// TODO: header stuff.

include "M6502InstrFormats.td"

//===----------------------------------------------------------------------===//
// M6502 Specific Node Definitions
//===----------------------------------------------------------------------===//

def M6502return : SDNode<"M6502ISD::RETURN", SDTNone,
                         [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

//===----------------------------------------------------------------------===//
// M6502 Operands, Complex Patterns and Transformations Definitions.
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
// M6502 Instruction list
// Most of the following instructions are pseudo-instructions intended to be
// lowered by a late-stage compilation pass.
// FIXME: Such a pass has not been implemented yet.
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Return instructions.
//===----------------------------------------------------------------------===//
let isTerminator = 1,
isReturn = 1,
isBarrier = 1 in 
{
  def RETURN : Pseudo<(outs),
                      (ins),
                      "_RETURN",
                      [(M6502return)]>;
}

//===----------------------------------------------------------------------===//
// Data transfer instructions
//===----------------------------------------------------------------------===//

// 8 and 16-bit register move instructions.
let hasSideEffects = 0 in
{
  def Treg : Pseudo<(outs GPR8:$rd),
                    (ins GPR8:$rr),
                    "_Treg\t$rd, $rr",
                    []>;
}

// Load immediate values into registers.
let isReMaterializable = 1 in
{
  def LDimm : Pseudo<(outs GPR8:$dst),
                     (ins i8imm:$k),
                     "_LDimm\t$dst, $k",
                     [(set i8:$dst, imm:$k)]>;

  def LDimm16: Pseudo<(outs DREGS:$dst),
                      (ins i16imm:$k),
                      "_LDimm16\t$dst, $k",
                      [(set i16:$dst, imm:$k)]>;
}

// Load register from the stack.
def LDstack : LoadPseudo<
  (outs GPR8:$dst),
  (ins memspi:$src),
  "_LDstack\t$dst, $src",
  [(set i8:$dst, (load addr:$src))]
>;

def LDstack16 : LoadPseudo<
  (outs DREGS:$dst),
  (ins memspi:$src),
  "_LDstack16\t$dst, $src",
  [(set i16:$dst, (load addr:$src))]
>;

// Store register to the stack.
def STstack : StorePseudo<
  (outs),
  (ins memspi:$dst, GPR8:$src),
  "_STstack\t$dst, $src",
  [(store i8:$src, addr:$dst)]
>;

def STstack16 : StorePseudo<
  (outs),
  (ins memspi:$dst, DREGS:$src),
  "_STstack16\t$dst, $src",
  [(store i16:$src, addr:$dst)]
>;
