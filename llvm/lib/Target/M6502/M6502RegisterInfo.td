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

def SPL : M6502Reg<"SPL">;
def SPH : M6502Reg<"SPH">;
def R0L : M6502Reg<"R0L">;
def R0H : M6502Reg<"R0H">;
def R1L : M6502Reg<"R1L">;
def R1H : M6502Reg<"R1H">;
def R2L : M6502Reg<"R2L">;
def R2H : M6502Reg<"R2H">;
def R3L : M6502Reg<"R3L">;
def R3H : M6502Reg<"R3H">;

let SubRegIndices = [sub_lo, sub_hi],
CoveredBySubRegs = 1 in
{
  def SP : M6502Reg<"SP", [SPL, SPH]>;
  def R0 : M6502Reg<"R0", [R0L, R0H]>;
  def R1 : M6502Reg<"R1", [R1L, R1H]>;
  def R2 : M6502Reg<"R2", [R2L, R2H]>;
  def R3 : M6502Reg<"R3", [R3L, R3H]>;
}

//===----------------------------------------------------------------------===//
// Register Classes
//===----------------------------------------------------------------------===//

// Main 8-bit register class.
def GPR8 : RegisterClass<"M6502", [i8], 8,
  (
    add SPL, SPH, R0L, R0H, R1L, R1H, R2L, R2H, R3L, R3H
  )>;

// Main 16-bit pair register class.
def DREGS : RegisterClass<"M6502", [i16], 8,
  (
    add SP, R0, R1, R2, R3
  )>;

// Register class used for the stack read pseudo instruction.
def GPRSP: RegisterClass<"M6502", [i16], 8, (add SP)>;

// Status register. Includes ZF, NF, VF and CF.
def SREG : M6502Reg<"FLAGS">;
def CCR : RegisterClass<"M6502", [i8], 8, (add SREG)>
{
  let CopyCost = -1;      // Don't allow copying of status registers
}
