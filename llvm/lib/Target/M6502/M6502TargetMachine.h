// TODO: header stuff

#ifndef LLVM_M6502_TARGET_MACHINE_H
#define LLVM_M6502_TARGET_MACHINE_H

#include "llvm/Target/TargetMachine.h"

#include "M6502Subtarget.h"

namespace llvm {

class M6502TargetMachine : public LLVMTargetMachine {
public:
  M6502TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                     StringRef FS, const TargetOptions &Options,
                     Optional<Reloc::Model> RM,
                     Optional<CodeModel::Model> CM,
                     CodeGenOpt::Level OL, bool JIT);
  
  const M6502Subtarget *getSubtargetImpl(const Function &) const override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return this->TLOF.get();
  }

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

private:
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  M6502Subtarget SubTarget;
};

} // end namespace llvm

#endif // LLVM_M6502_TARGET_MACHINE_H
