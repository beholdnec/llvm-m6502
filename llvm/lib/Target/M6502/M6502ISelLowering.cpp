// TODO: header stuff.

#include "M6502ISelLowering.h"

#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/SelectionDAG.h"

#include "M6502TargetMachine.h"

namespace llvm {

M6502TargetLowering::M6502TargetLowering(const M6502TargetMachine &TM,
                                         const M6502Subtarget &STI)
    : TargetLowering(TM), Subtarget(STI) {
  // Set up the register classes.
  addRegisterClass(MVT::i8, &M6502::GPR8RegClass);
  addRegisterClass(MVT::i16, &M6502::DREGSRegClass);

  // Compute derived properties from the register classes.
  computeRegisterProperties(Subtarget.getRegisterInfo());
  
  setSchedulingPreference(Sched::RegPressure);
  
  for (MVT VT : MVT::integer_valuetypes()) {
    for (auto N : {ISD::EXTLOAD, ISD::SEXTLOAD, ISD::ZEXTLOAD}) {
      setLoadExtAction(N, VT, MVT::i1, Promote);
      setLoadExtAction(N, VT, MVT::i8, Expand);
    }
  }

  setTruncStoreAction(MVT::i16, MVT::i8, Expand);
  
  for (MVT VT : MVT::integer_valuetypes()) {
    setOperationAction(ISD::ADDC, VT, Legal);
    setOperationAction(ISD::SUBC, VT, Legal);
    setOperationAction(ISD::ADDE, VT, Legal);
    setOperationAction(ISD::SUBE, VT, Legal);
  }
  
  setMinFunctionAlignment(1);
}

const char *M6502TargetLowering::getTargetNodeName(unsigned Opcode) const {
#define NODE(name)       \
  case M6502ISD::name:     \
    return #name

  switch (Opcode) {
  default:
    return nullptr;
    NODE(RETURN);
#undef NODE
  }
}

//===----------------------------------------------------------------------===//
//             Formal Arguments Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "M6502GenCallingConv.inc"

bool
M6502TargetLowering::CanLowerReturn(CallingConv::ID CallConv,
                                    MachineFunction &MF, bool isVarArg,
                                    const SmallVectorImpl<ISD::OutputArg> &Outs,
                                    LLVMContext &Context) const
{
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, MF, RVLocs, Context);

  return CCInfo.CheckReturn(Outs, RetCC_M6502);
}

SDValue
M6502TargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                 bool isVarArg,
                                 const SmallVectorImpl<ISD::OutputArg> &Outs,
                                 const SmallVectorImpl<SDValue> &OutVals,
                                 const SDLoc &dl, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  auto DL = DAG.getDataLayout();

  // CCValAssign - represent the assignment of the return value to locations.
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  // Analyze return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_M6502);
  
  SmallVector<SDValue, 4> RetOps(1, Chain);

  for (unsigned i = 0; i < RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];

    if (VA.isMemLoc()) {
      EVT LocVT = VA.getLocVT();

      // Create the frame index object for this outgoing value.
      // FIXME: is this the correct thing to do?
      int FI = MFI.CreateFixedObject(LocVT.getSizeInBits() / 8,
                                     VA.getLocMemOffset(), true);

      // Create the SelectionDAG nodes corresponding to a store
      // to this location.
      SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DL));
      SDValue RetOp = DAG.getStore(Chain, dl, OutVals[i], FIN,
                                   MachinePointerInfo::getFixedStack(MF, FI),
                                   0);
      RetOps.push_back(RetOp);
    } else {
      llvm_unreachable("Return value must be located in memory");
    }
  }

  if (!RetOps.empty()) {
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, RetOps);
  }

  // FIXME: is glue needed?
  return DAG.getNode(M6502ISD::RETURN, dl, MVT::Other, Chain);
}

SDValue M6502TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl, SelectionDAG &DAG,
    SmallVectorImpl<SDValue> &InVals) const {
  // TODO
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  auto DL = DAG.getDataLayout();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeFormalArguments(Ins, CC_M6502);

  for (unsigned i = 0; i < ArgLocs.size(); ++i) {
    CCValAssign &VA = ArgLocs[i];

    if (VA.isMemLoc()) {
      EVT LocVT = VA.getLocVT();

      // Create the frame index object for this incoming parameter.
      int FI = MFI.CreateFixedObject(LocVT.getSizeInBits() / 8,
                                     VA.getLocMemOffset(), true);

      // Create the SelectionDAG nodes corresponding to a load
      // from this parameter.
      SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DL));
      InVals.push_back(DAG.getLoad(LocVT, dl, Chain, FIN,
                                   MachinePointerInfo::getFixedStack(MF, FI),
                                   0));
    } else {
      llvm_unreachable("Argument must be located in memory");
    }
  }

  return Chain;
}

} // end of namespace llvm
