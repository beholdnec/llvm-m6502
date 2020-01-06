// TODO: header stuff

#include "M6502TargetMachine.h"

#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/Support/TargetRegistry.h"

#include "M6502.h"
#include "M6502TargetObjectFile.h"
#include "TargetInfo/M6502TargetInfo.h"

namespace llvm {

static const char *M6502DataLayout = "e-P1-p:16:8-i8:8-i16:8-i32:8-i64:8-f32:8-f64:8-n8-a:8";

/// Processes a CPU name.
static StringRef getCPU(StringRef CPU) {
  if (CPU.empty() || CPU == "generic") {
    return "m6502";
  }

  return CPU;
}

static Reloc::Model getEffectiveRelocModel(Optional<Reloc::Model> RM) {
  return RM.hasValue() ? *RM : Reloc::Static;
}

M6502TargetMachine::M6502TargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       Optional<Reloc::Model> RM,
                                       Optional<CodeModel::Model> CM,
                                       CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, M6502DataLayout, TT, getCPU(CPU), FS, Options,
                        getEffectiveRelocModel(RM),
                        getEffectiveCodeModel(CM, CodeModel::Small), OL),
      SubTarget(TT, getCPU(CPU), FS, *this) {
  this->TLOF = make_unique<M6502TargetObjectFile>();
  initAsmInfo();
}

namespace {
// M6502 Code Generator Pass Configuration Options.
class M6502PassConfig : public TargetPassConfig {
public:
  M6502PassConfig(M6502TargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  M6502TargetMachine &getM6502TargetMachine() const {
    return getTM<M6502TargetMachine>();
  }

  bool addInstSelector() override;
  void addPreSched2() override;
};
} // namespace

TargetPassConfig *M6502TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new M6502PassConfig(*this, PM);
}

extern "C" void LLVMInitializeM6502Target() {
  // Register the target.
  RegisterTargetMachine<M6502TargetMachine> X(getTheM6502Target());

  auto &PR = *PassRegistry::getPassRegistry();
  initializeM6502Expand16BitPseudoPass(PR);
}

const M6502Subtarget *M6502TargetMachine::getSubtargetImpl(const Function &) const {
  return &SubTarget;
}

//===----------------------------------------------------------------------===//
// Pass Pipeline Configuration
//===----------------------------------------------------------------------===//

bool M6502PassConfig::addInstSelector() {
  // Install an instruction selector.
  addPass(createM6502ISelDag(getM6502TargetMachine(), getOptLevel()));

  return false;
}

void M6502PassConfig::addPreSched2() {
  addPass(createM6502Expand16BitPseudoPass());
}

} // end of namespace llvm
