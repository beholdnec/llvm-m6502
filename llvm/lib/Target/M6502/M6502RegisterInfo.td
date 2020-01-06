// TODO: header stuff

//===----------------------------------------------------------------------===//
//  Declarations that describe the M6502 register file
//===----------------------------------------------------------------------===//

// 8-bit General purpose register definition.
class M6502Reg<string name,
               list<Register> subregs = []>
  : RegisterWithSubRegs<name, subregs>
{
  let Namespace = "M6502";
  let SubRegs = subregs;
}

// Subregister indices.
let Namespace = "M6502" in
{
  def sub_lo : SubRegIndex<8>;
  def sub_hi : SubRegIndex<8, 8>;
}

//===----------------------------------------------------------------------===//
//  8-bit general purpose pseudo-registers in zero-page memory
//===----------------------------------------------------------------------===//

def R0 : M6502Reg<"r0">;
def R1 : M6502Reg<"r1">;
def R2 : M6502Reg<"r2">;
def R3 : M6502Reg<"r3">;
def R4 : M6502Reg<"r4">;
def R5 : M6502Reg<"r5">;
def R6 : M6502Reg<"r6">;
def R7 : M6502Reg<"r7">;
def SPL : M6502Reg<"SPL">;
def SPH : M6502Reg<"SPH">;

let SubRegIndices = [sub_lo, sub_hi],
CoveredBySubRegs = 1 in
{
  def SP     : M6502Reg<"SP",      [SPL, SPH]>;
  def R7R6   : M6502Reg<"r7:r6",   [R6, R7]>;
  def R5R4   : M6502Reg<"r5:r4",   [R4, R5]>;
  def R3R2   : M6502Reg<"r3:r2",   [R2, R3]>;
  def R1R0   : M6502Reg<"r1:r0",   [R0, R1]>;
}

//===----------------------------------------------------------------------===//
// Register Classes
//===----------------------------------------------------------------------===//

// Main 8-bit register class.
def GPR8 : RegisterClass<"M6502", [i8], 8,
  (
    add R0, R1, R2, R3, R4, R5, R6, R7, SPL, SPH
  )>;

// Main 16-bit pair register class.
def DREGS : RegisterClass<"M6502", [i16], 8,
  (
    add R1R0, R3R2, R5R4, R7R6, SP
  )>;

// Register class used for the stack read pseudo instruction.
def GPRSP: RegisterClass<"M6502", [i16], 8, (add SP)>;

// Status register. Includes ZF, NF, VF and CF.
def SREG : M6502Reg<"FLAGS">;
def CCR : RegisterClass<"M6502", [i8], 8, (add SREG)>
{
  let CopyCost = -1;      // Don't allow copying of status registers
}
