// TODO: header stuff.

// A generic M6502 instruction.
class M6502Inst<dag outs, dag ins, string asmstr, list<dag> pattern> : Instruction
{
  let Namespace = "M6502";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;
}

// A class for pseudo instructions.
// Psuedo instructions are not real M6502 instructions. The DAG stores
// psuedo instructions which are replaced by real M6502 instructions by
// M6502ExpandPseudoInsts.cpp.
//
// For example, the ADDW (add wide, as in add 16 bit values) instruction
// is defined as a pseudo instruction. In M6502ExpandPseudoInsts.cpp,
// the instruction is then replaced by two add instructions - one for each byte.
class Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
  : M6502Inst<outs, ins, asmstr, pattern>
{
  let isPseudo = 1;
  let isCodeGenOnly = 1;
}

class LoadPseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
  : Pseudo<outs, ins, asmstr, pattern>
{
  let mayLoad = 1;
  //let Defs = [SP];
}

class StorePseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
  : Pseudo<outs, ins, asmstr, pattern>
{
  let mayStore = 1; // FIXME: is this needed?
  //let Defs = [SP];
}

class ExtensionPseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
  : Pseudo<outs, ins, asmstr, pattern>
{
  //let Defs = [SREG];
}
