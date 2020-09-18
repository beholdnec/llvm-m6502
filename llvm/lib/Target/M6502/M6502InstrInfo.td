// TODO: header stuff.

include "M6502InstrFormats.td"

//===----------------------------------------------------------------------===//
// M6502 Type Profiles
//===----------------------------------------------------------------------===//

def SDT_M6502CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;
def SDT_M6502CallSeqEnd : SDCallSeqEnd<[SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;
def SDT_M6502Call : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_M6502Wrapper : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>, SDTCisPtrTy<0>]>;
def SDT_M6502Brcond : SDTypeProfile<0, 2,
                                    [SDTCisVT<0, OtherVT>, SDTCisVT<1, i8>]>;
def SDT_M6502Cmp : SDTypeProfile<0, 2, [SDTCisSameAs<0, 1>]>;

//===----------------------------------------------------------------------===//
// M6502 Specific Node Definitions
//===----------------------------------------------------------------------===//

def M6502return : SDNode<"M6502ISD::RETURN", SDTNone,
                         [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def M6502callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_M6502CallSeqStart,
                                [SDNPHasChain, SDNPOutGlue]>;
def M6502callseq_end : SDNode<"ISD::CALLSEQ_END", SDT_M6502CallSeqEnd,
                              [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;
                              
def M6502call : SDNode<"M6502ISD::CALL", SDT_M6502Call,
                       [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;

def M6502Wrapper : SDNode<"M6502ISD::WRAPPER", SDT_M6502Wrapper>;

def M6502brcond : SDNode<"M6502ISD::BRCOND", SDT_M6502Brcond,
                       [SDNPHasChain, SDNPInGlue]>;
def M6502cmp : SDNode<"M6502ISD::CMP", SDT_M6502Cmp, [SDNPOutGlue]>;
def M6502cmpc : SDNode<"M6502ISD::CMPC", SDT_M6502Cmp, [SDNPInGlue, SDNPOutGlue]>;

// Shift nodes.
def M6502lsl : SDNode<"M6502ISD::LSL", SDTIntUnaryOp>;
def M6502lsr : SDNode<"M6502ISD::LSR", SDTIntUnaryOp>;
def M6502asr : SDNode<"M6502ISD::ASR", SDTIntUnaryOp>;

//===----------------------------------------------------------------------===//
// M6502 Operands, Complex Patterns and Transformations Definitions.
//===----------------------------------------------------------------------===//

// Address operand for `SP+imm` used by LDstk and STstk
def memspi : Operand<iPTR>
{
  let MIOperandInfo = (ops GPRSP, i16imm);
}

// FIXME: M6502 doesn't work this way.
// Addressing mode pattern reg+imm6
def addr : ComplexPattern<iPTR, 2, "SelectAddr", [], [SDNPWantRoot]>;

def brtarget : Operand<OtherVT>;
def call_target : Operand<iPTR>;

// M6502 specific condition code. These correspond to M6502_*_COND in
// M6502InstrInfo.td. They must be kept in synch.
def M6502_COND_EQ : PatLeaf<(i8 0)>;
def M6502_COND_NE : PatLeaf<(i8 1)>;
def M6502_COND_GE : PatLeaf<(i8 2)>;
def M6502_COND_LT : PatLeaf<(i8 3)>;
def M6502_COND_SH : PatLeaf<(i8 4)>;
def M6502_COND_LO : PatLeaf<(i8 5)>;
def M6502_COND_MI : PatLeaf<(i8 6)>;
def M6502_COND_PL : PatLeaf<(i8 7)>;

//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//
// M6502 Instruction list
// Most of the following instructions are pseudo-instructions intended to be
// lowered by a late-stage compilation pass.
// FIXME: Such a pass has not been implemented yet.
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//

// ADJCALLSTACKDOWN/UP implicitly use/def SP because they may be expanded into
// a stack adjustment and the codegen must know that they may modify the stack
// pointer before prolog-epilog rewriting occurs.
// Pessimistically assume ADJCALLSTACKDOWN / ADJCALLSTACKUP will become
// sub / add which can clobber SREG.
let Defs = [SP, SREG],
Uses = [SP] in
{
  def ADJCALLSTACKDOWN : Pseudo<(outs),
                                (ins i16imm:$amt, i16imm:$amt2),
                                "#ADJCALLSTACKDOWN",
                                [(M6502callseq_start timm:$amt, timm:$amt2)]>;

  def ADJCALLSTACKUP : Pseudo<(outs),
                              (ins i16imm:$amt1, i16imm:$amt2),
                              "#ADJCALLSTACKUP",
                              [(M6502callseq_end timm:$amt1, timm:$amt2)]>;
}

//===----------------------------------------------------------------------===//
// Call instructions
//===----------------------------------------------------------------------===//

let isCall = 1 in
{
  let Uses = [SP] in
  def CALLk : Pseudo<(outs),
                     (ins call_target:$k),
                     "_CALL\t$k",
                     [(M6502call imm:$k)]>;
}

//===----------------------------------------------------------------------===//
// Jump instructions
//===----------------------------------------------------------------------===//

let isBarrier = 1,
isBranch = 1,
isTerminator = 1 in
{
  let isIndirectBranch = 1 in
  def IJMP : Pseudo<(outs),
                    (ins DREGS:$target),
                    "_IJMP\t$target",
                    [(brind i16:$target)]>;

  def JMPk : Pseudo<(outs),
                    (ins brtarget:$k),
                    "_JMP\t$k",
                    [(br bb:$k)]>;
}

//===----------------------------------------------------------------------===//
// Arithmetic instructions
//===----------------------------------------------------------------------===//

multiclass ArithInst<string AsmName, SDPatternOperator Oper> {
  let Constraints = "$dst = $src",
  Defs = [SREG] in
  {
    // Immediate
    def imm : Pseudo<(outs GPR8:$dst),
                     (ins GPR8:$src, i8imm:$k),
                     AsmName # "imm\t$dst, $k",
                     [(set i8:$dst, (Oper GPR8:$src, imm:$k)), (implicit SREG)]>;
    
    // Immediate 16-bit
    def imm16 : Pseudo<(outs DREGS:$dst),
                       (ins DREGS:$src, i16imm:$k),
                       AsmName # "imm16\t$dst, $k",
                       [(set i16:$dst, (Oper DREGS:$src, imm:$k)), (implicit SREG)]>;

    // Register
    def reg : Pseudo<(outs GPR8:$dst),
                     (ins GPR8:$src, GPR8:$r),
                     AsmName # "reg\t$dst, $r",
                     [(set i8:$dst, (Oper GPR8:$src, GPR8:$r)), (implicit SREG)]>;

    // Register 16-bit
    def reg16 : Pseudo<(outs DREGS:$dst),
                       (ins DREGS:$src, DREGS:$r),
                       AsmName # "reg16\t$dst, $r",
                       [(set i16:$dst, (Oper DREGS:$src, DREGS:$r)), (implicit SREG)]>;

    // Absolute address
    // FIXME: codegen never emits this instruction. Fix load-folding.
    let mayLoad = 1 in
    def abs : Pseudo<(outs GPR8:$dst),
                     (ins GPR8:$src, i16imm:$mem),
                     AsmName # "abs\t$dst, $mem",
                     [(set i8:$dst, (Oper GPR8:$src, (load imm:$mem))), (implicit SREG)]>;

    // Absolute address 16-bit
    // FIXME: codegen never emits this instruction. Fix load-folding.
    let mayLoad = 1 in
    def abs16 : Pseudo<(outs DREGS:$dst),
                       (ins DREGS:$src, i16imm:$mem),
                       AsmName # "abs16\t$dst, $mem",
                       [(set i16:$dst, (Oper DREGS:$src, (load imm:$mem))), (implicit SREG)]>;

    // TODO: Indirect address, etc.
  }
}

let isCommutable = 1, Defs = [SREG] in
  defm AD0 : ArithInst<"_AD0", add>;

let isCommutable = 1, Defs = [SREG], Uses = [SREG] in
  defm ADC : ArithInst<"_ADC", adde>;

let Defs = [SREG] in
  defm SB1 : ArithInst<"_SB1", sub>;

let Defs = [SREG], Uses = [SREG] in
  defm SBC : ArithInst<"_SBC", sube>;

let isCommutable = 1 in
  defm AND : ArithInst<"_AND", and>;

let isCommutable = 1 in
  defm EOR : ArithInst<"_EOR", xor>;

let isCommutable = 1 in
  defm OR : ArithInst<"_OR", or>;
  
//===----------------------------------------------------------------------===//
// Increment and Decrement
//===----------------------------------------------------------------------===//

let Constraints = "$dst = $src",
Defs = [SREG] in
{
  // These instructions do not modify the carry flag, and therefore
  // cannot be used for 16-bit arithmetic.

  def INreg : Pseudo<(outs GPR8:$dst),
                     (ins GPR8:$src),
                     "_INreg\t$dst",
                     [(set i8:$dst, (add i8:$src, 1)), (implicit SREG)]>;
                     
  def DEreg : Pseudo<(outs GPR8:$dst),
                     (ins GPR8:$src),
                     "_DEreg\t$dst",
                     [(set i8:$dst, (add i8:$src, -1)), (implicit SREG)]>;
}

let Defs = [SREG],
mayLoad = 1,
mayStore = 1 in
{
  def INabs : Pseudo<(outs),
                     (ins i16imm:$mem),
                     "_INabs\t$mem",
                     [(store (add (i8 (load imm:$mem)), 1), imm:$mem), (implicit SREG)]>;

  def DEabs : Pseudo<(outs),
                     (ins i16imm:$mem),
                     "_DEabs\t$mem",
                     [(store (add (i8 (load imm:$mem)), -1), imm:$mem), (implicit SREG)]>;
}

def : Pat<(store (add (i8 (load (i16 (M6502Wrapper tglobaladdr:$mem)))), 1), (i16 (M6502Wrapper tglobaladdr:$mem))),
          (INabs tglobaladdr:$mem)>;
          
def : Pat<(store (add (i8 (load (i16 (M6502Wrapper tglobaladdr:$mem)))), -1), (i16 (M6502Wrapper tglobaladdr:$mem))),
          (DEabs tglobaladdr:$mem)>;
  
//===----------------------------------------------------------------------===//
// Bit and bit-test instructions
//===----------------------------------------------------------------------===//

// Bit shift/rotate operations.
let Constraints = "$dst = $src",
Defs = [SREG] in
{
  def ASLreg : Pseudo<(outs GPR8:$dst),
                      (ins GPR8:$src),
                      "_ASLreg\t$dst",
                      [(set i8:$dst, (M6502lsl i8:$src)), (implicit SREG)]>;
                      
  def ASLreg16 : Pseudo<(outs DREGS:$dst),
                        (ins DREGS:$src),
                        "_ASLreg16\t$dst",
                        [(set i16:$dst, (M6502lsl i16:$src)), (implicit SREG)]>;

  def LSRreg : Pseudo<(outs GPR8:$dst),
                      (ins GPR8:$src),
                      "_LSRreg\t$dst",
                      [(set i8:$dst, (M6502lsr i8:$src)), (implicit SREG)]>;
                      
  def LSRreg16 : Pseudo<(outs DREGS:$dst),
                      (ins DREGS:$src),
                      "_LSRreg16\t$dst",
                      [(set i16:$dst, (M6502lsr i16:$src)), (implicit SREG)]>;

  def ASRreg16 : Pseudo<(outs DREGS:$dst),
                        (ins DREGS:$src),
                        "_ASRreg16\t$dst",
                        [(set i16:$dst, (M6502asr i16:$src)), (implicit SREG)]>;
}

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
// Compare operations.
//===----------------------------------------------------------------------===//

let Defs = [SREG] in
{
  def CPimm : Pseudo<(outs),
                     (ins GPR8:$src, i8imm:$k),
                     "_CPimm\t$src, $k",
                     [(M6502cmp i8:$src, imm:$k), (implicit SREG)]>;

  def CPreg : Pseudo<(outs),
                     (ins GPR8:$src1, GPR8:$src2),
                     "_CPreg\t$src1, $src2",
                     [(M6502cmp i8:$src1, i8:$src2), (implicit SREG)]>;
}

// FIXME: This is adapted from AVR, but M6502 has no equivalent to cpc.

let Defs = [SREG], Uses = [SREG] in
{
  def CPCimm : Pseudo<(outs),
                      (ins GPR8:$src, i8imm:$k),
                      "_CPCimm\t$src, $k",
                      [(M6502cmpc i8:$src, imm:$k), (implicit SREG)]>;

  def CPCreg : Pseudo<(outs),
                      (ins GPR8:$src1, GPR8:$src2),
                      "_CPCreg\t$src1, $src2",
                      [(M6502cmpc i8:$src1, i8:$src2), (implicit SREG)]>;
}

//===----------------------------------------------------------------------===//
// Conditional branches
//===----------------------------------------------------------------------===//

// Based on status register. We cannot simplify these into instruction aliases
// because we also need to be able to specify a pattern to match for ISel.
let isBranch = 1,
isTerminator = 1,
Uses = [SREG] in
{
  def BREQk : Pseudo<(outs),
                     (ins brtarget:$target),
                     "_BREQ\t$target",
                     [(M6502brcond bb:$target, M6502_COND_EQ)]>;
                     
  def BRNEk : Pseudo<(outs),
                     (ins brtarget:$target),
                     "_BRNE\t$target",
                     [(M6502brcond bb:$target, M6502_COND_NE)]>;

  def BRSHk : Pseudo<(outs),
                     (ins brtarget:$target),
                     "_BRSH\t$target",
                     [(M6502brcond bb:$target, M6502_COND_SH)]>;
                     
  def BRLOk : Pseudo<(outs),
                     (ins brtarget:$target),
                     "_BRLO\t$target",
                     [(M6502brcond bb:$target, M6502_COND_LO)]>;
                     
  def BRMIk : Pseudo<(outs),
                     (ins brtarget:$target),
                     "_BRMI\t$target",
                     [(M6502brcond bb:$target, M6502_COND_MI)]>;
                     
  def BRPLk : Pseudo<(outs),
                     (ins brtarget:$target),
                     "_BRPL\t$target",
                     [(M6502brcond bb:$target, M6502_COND_PL)]>;
                     
  def BRGEk : Pseudo<(outs),
                     (ins brtarget:$target),
                     "_BRGE\t$target",
                     [(M6502brcond bb:$target, M6502_COND_GE)]>;
                     
  def BRLTk : Pseudo<(outs),
                     (ins brtarget:$target),
                     "_BRLT\t$target",
                     [(M6502brcond bb:$target, M6502_COND_LT)]>;
}

//===----------------------------------------------------------------------===//
// Data transfer instructions
//===----------------------------------------------------------------------===//

// 8 and 16-bit register move instructions.
let hasSideEffects = 0 in
{
  def Treg : Pseudo<(outs GPR8:$dst),
                    (ins GPR8:$src),
                    "_Treg\t$dst, $src",
                    []>;

  def Treg16 : Pseudo<(outs DREGS:$dst),
                      (ins DREGS:$src),
                      "_Treg16\t$dst, $src",
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

// Load from absolute memory address

let canFoldAsLoad = 1,
isReMaterializable = 1 in
{
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
}

// Load from memory addressed by pointer register

let canFoldAsLoad = 1,
isReMaterializable = 1 in
{
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
}

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
  [(set i16:$dst, (sext i8:$src)), (implicit SREG)]
>;

def ZEXT : ExtensionPseudo<
  (outs DREGS:$dst),
  (ins GPR8:$src),
  "_ZEXT\t$dst, $src",
  [(set i16:$dst, (zext i8:$src)), (implicit SREG)]
>;

//===----------------------------------------------------------------------===//
// Non-Instruction Patterns
//===----------------------------------------------------------------------===//

// Calls.
def : Pat<(M6502call (i16 tglobaladdr:$dst)),
          (CALLk tglobaladdr:$dst)>;
def : Pat<(M6502call (i16 texternalsym:$dst)),
          (CALLk texternalsym:$dst)>;

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
