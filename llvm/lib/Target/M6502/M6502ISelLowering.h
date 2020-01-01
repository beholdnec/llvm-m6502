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
  RET_FLAG,
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

private:
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
                      
protected:
  const M6502Subtarget &Subtarget;
};

} // end of namespace llvm

#endif // LLVM_M6502_ISEL_LOWERING_H
