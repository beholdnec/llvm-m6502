// TODO: header stuff.

#ifndef LLVM_M6502_ISEL_LOWERING_H
#define LLVM_M6502_ISEL_LOWERING_H

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

namespace M6502ISD {

/// M6502 Specific DAG Nodes
enum NodeType {
  /// Start the numbering where the builtin ops leave off.
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  /// Return from subroutine.
  RETURN,
  /// Represents an abstract call instruction,
  /// which includes a bunch of information.
  CALL,
  /// A wrapper node for TargetConstantPool,
  /// TargetExternalSymbol, and TargetGlobalAddress.
  WRAPPER,
  LSL,     ///< Logical shift left.
  LSR,     ///< Logical shift right.
  ASR,     ///< Arithmetic shift right.
  ROR,     ///< Bit rotate right.
  ROL,     ///< Bit rotate left.
  LSLLOOP, ///< A loop of single logical shift left instructions.
  LSRLOOP, ///< A loop of single logical shift right instructions.
  ROLLOOP, ///< A loop of single left bit rotate instructions.
  RORLOOP, ///< A loop of single right bit rotate instructions.
  ASRLOOP, ///< A loop of single arithmetic shift right instructions.
  /// M6502 conditional branches. Operand 0 is the chain operand, operand 1
  /// is the block to branch if condition is true, operand 2 is the
  /// condition code, and operand 3 is the flag operand produced by a CMP
  /// or TEST instruction.
  BRCOND,
  /// Compare instruction.
  CMP,
  /// Compare with carry instruction.
  CMPC,
  /// Test for zero or minus instruction.
  // TST,
  /// Operand 0 and operand 1 are selection variable, operand 2
  /// is condition code and operand 3 is flag operand.
  SELECT_CC,
  /// Extract high byte of 16-bit value
  EXTRACTHI,
};

} // end of namespace M6502ISD

class M6502Subtarget;
class M6502TargetMachine;

/// Performs target lowering for the M6502.
class M6502TargetLowering : public TargetLowering {
public:
  explicit M6502TargetLowering(const M6502TargetMachine &TM,
                               const M6502Subtarget &STI);
                               
  const char *getTargetNodeName(unsigned Opcode) const override;
  
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

private:
  SDValue getM6502Cmp(SDValue LHS, SDValue RHS, ISD::CondCode CC, SDValue &AVRcc,
                      SelectionDAG &DAG, SDLoc dl) const;
  SDValue LowerAnd(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerExtractElement(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerShifts(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBlockAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;

  bool CanLowerReturn(CallingConv::ID CallConv,
                      MachineFunction &MF, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      LLVMContext &Context) const override;
                      
  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &dl,
                      SelectionDAG &DAG) const override;
  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;
  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;
  SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
                          CallingConv::ID CallConv, bool isVarArg,
                          const SmallVectorImpl<ISD::InputArg> &Ins,
                          const SDLoc &dl, SelectionDAG &DAG,
                          SmallVectorImpl<SDValue> &InVals) const;
                      
protected:
  const M6502Subtarget &Subtarget;
};

} // end of namespace llvm

#endif // LLVM_M6502_ISEL_LOWERING_H
