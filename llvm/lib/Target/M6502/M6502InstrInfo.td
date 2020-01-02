// TODO: header stuff.

include "M6502InstrFormats.td"

//===----------------------------------------------------------------------===//
// M6502 Type Profiles
//===----------------------------------------------------------------------===//

def SDT_M6502Wrapper : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>, SDTCisPtrTy<0>]>;

//===----------------------------------------------------------------------===//
// M6502 Specific Node Definitions
//===----------------------------------------------------------------------===//

def M6502return : SDNode<"M6502ISD::RETURN", SDTNone,
                         [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def M6502Wrapper : SDNode<"M6502ISD::WRAPPER", SDT_M6502Wrapper>;

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
// Arithmetic instructions
//===----------------------------------------------------------------------===//

multiclass ArithInst<string AsmName, SDPatternOperator Oper> {
  let Constraints = "$dst = $src" in
  {
    // Immediate
    def imm : Pseudo<(outs GPR8:$dst),
                     (ins GPR8:$src, i8imm:$k),
                     AsmName # "imm\t$dst, $k",
                     [(set i8:$dst, (Oper GPR8:$src, imm:$k))]>;
    
    // Immediate 16-bit
    def imm16 : Pseudo<(outs DREGS:$dst),
                       (ins DREGS:$src, i16imm:$k),
                       AsmName # "imm16\t$dst, $k",
                       [(set i16:$dst, (Oper DREGS:$src, imm:$k))]>;

    // Register
    def reg : Pseudo<(outs GPR8:$dst),
                     (ins GPR8:$src, GPR8:$r),
                     AsmName # "reg\t$dst, $r",
                     [(set i8:$dst, (Oper GPR8:$src, GPR8:$r))]>;

    // Register 16-bit
    def reg16 : Pseudo<(outs DREGS:$dst),
                       (ins DREGS:$src, DREGS:$r),
                       AsmName # "reg16\t$dst, $r",
                       [(set i16:$dst, (Oper DREGS:$src, DREGS:$r))]>;

    // TODO: Absolute address, Indirect address, etc.
  }
}

let isCommutable = 1, Defs = [SREG] in
  defm AD0 : ArithInst<"_AD0", add>; // FIXME: implicit SREG?

let isCommutable = 1, Defs = [SREG], Uses = [SREG] in
  defm ADC : ArithInst<"_ADC", adde>; // FIXME: implicit SREG?

let Defs = [SREG] in
  defm SB1 : ArithInst<"_SB1", sub>; // FIXME: implicit SREG?

let Defs = [SREG], Uses = [SREG] in
  defm SBC : ArithInst<"_SBC", sube>; // FIXME: implicit SREG?

let isCommutable = 1 in
  defm AND : ArithInst<"_AND", and>;

let isCommutable = 1 in
  defm EOR : ArithInst<"_EOR", xor>;

let isCommutable = 1 in
  defm ORA : ArithInst<"_ORA", or>;

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
def LDstk : LoadPseudo<
  (outs GPR8:$dst),
  (ins memspi:$src),
  "_LDstk\t$dst, $src",
  [(set i8:$dst, (load addr:$src))]
>;

def LDstk16 : LoadPseudo<
  (outs DREGS:$dst),
  (ins memspi:$src),
  "_LDstk16\t$dst, $src",
  [(set i16:$dst, (load addr:$src))]
>;

// Store register to the stack.
def STstk : StorePseudo<
  (outs),
  (ins memspi:$dst, GPR8:$src),
  "_STstk\t$dst, $src",
  [(store i8:$src, addr:$dst)]
>;

def STstk16 : StorePseudo<
  (outs),
  (ins memspi:$dst, DREGS:$src),
  "_STstk16\t$dst, $src",
  [(store i16:$src, addr:$dst)]
>;

// Load from memory addressed by pointer register

def LDptr : LoadPseudo<
  (outs GPR8:$reg),
  (ins DREGS:$ptrreg),
  "_LDptr\t$reg, $ptrreg",
  [(set GPR8:$reg, (load DREGS:$ptrreg))]
>;

def LDptr16 : LoadPseudo<
  (outs DREGS:$reg),
  (ins DREGS:$ptrreg),
  "_LDptr16\t$reg, $ptrreg",
  [(set DREGS:$reg, (load DREGS:$ptrreg))]
>;

// Load from absolute memory address

def LDabs : LoadPseudo<
  (outs GPR8:$dst),
  (ins i16imm:$src),
  "_LDabs\t$dst, $src",
  [(set GPR8:$dst, (load imm:$src))]
>;

def LDabs16 : LoadPseudo<
  (outs DREGS:$dst),
  (ins i16imm:$src),
  "_LDabs16\t$dst, $src",
  [(set DREGS:$dst, (load imm:$src))]
>;

// Store register to memory addressed by pointer register

def STptr : StorePseudo<
  (outs),
  (ins DREGS:$ptrreg, GPR8:$reg),
  "_STptr\t$ptrreg, $reg",
  [(store GPR8:$reg, i16:$ptrreg)]
>;

def STptr16 : StorePseudo<
  (outs),
  (ins DREGS:$ptrreg, DREGS:$reg),
  "_STptr16\t$ptrreg, $reg",
  [(store DREGS:$reg, i16:$ptrreg)]
>;

// Store register to absolute memory address

def STabs : StorePseudo<
  (outs),
  (ins i16imm:$dst, GPR8:$src),
  "_STabs\t$dst, $src",
  [(store GPR8:$src, imm:$dst)]
>;

def STabs16 : StorePseudo<
  (outs),
  (ins i16imm:$dst, DREGS:$src),
  "_STabs16\t$dst, $src",
  [(store DREGS:$src, imm:$dst)]
>;

// Sign- and zero-extension

def SEXT : ExtensionPseudo<
  (outs DREGS:$dst),
  (ins GPR8:$src),
  "_SEXT\t$dst, $src",
  [(set i16:$dst, (sext i8:$src))]
>;

def ZEXT : ExtensionPseudo<
  (outs DREGS:$dst),
  (ins GPR8:$src),
  "_ZEXT\t$dst, $src",
  [(set i16:$dst, (zext i8:$src))]
>;

//===----------------------------------------------------------------------===//
// Non-Instruction Patterns
//===----------------------------------------------------------------------===//

// `anyext`
def : Pat<(i16 (anyext i8:$src)),
          (INSERT_SUBREG (i16 (IMPLICIT_DEF)), i8:$src, sub_lo)>;

// `trunc`
def : Pat<(i8 (trunc i16:$src)),
          (EXTRACT_SUBREG i16:$src, sub_lo)>;

// sext_inreg
def : Pat<(sext_inreg i16:$src, i8),
          (SEXT (i8 (EXTRACT_SUBREG i16:$src, sub_lo)))>;

// GlobalAddress
def : Pat<(i16 (M6502Wrapper tglobaladdr:$dst)),
          (LDimm16 tglobaladdr:$dst)>;
def : Pat<(i8 (load (M6502Wrapper tglobaladdr:$dst))),
          (LDabs tglobaladdr:$dst)>;
def : Pat<(i16 (load (M6502Wrapper tglobaladdr:$dst))),
          (LDabs16 tglobaladdr:$dst)>;
def : Pat<(store i8:$src, (i16 (M6502Wrapper tglobaladdr:$dst))),
          (STabs tglobaladdr:$dst, i8:$src)>;
def : Pat<(store i16:$src, (i16 (M6502Wrapper tglobaladdr:$dst))),
          (STabs16 tglobaladdr:$dst, i16:$src)>;

// the add instruction always reads and writes the carry flag. The carry flag
// must be explicitly cleared.
def : Pat<(addc i8:$src, i8:$src2),
          (AD0reg i8:$src, i8:$src2)>;
def : Pat<(addc DREGS:$src, DREGS:$src2),
          (AD0reg16 DREGS:$src, DREGS:$src2)>;
          
// all sub instruction variants always writes the carry flag. The carry flag
// must be explicitly set.
def : Pat<(subc i8:$src, i8:$src2),
          (SB1reg i8:$src, i8:$src2)>;
def : Pat<(subc DREGS:$src, DREGS:$src2),
          (SB1reg16 DREGS:$src, DREGS:$src2)>;
