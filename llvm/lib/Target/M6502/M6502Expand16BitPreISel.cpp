#include "M6502.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "m6502-expand-16bit-pre-isel"

namespace {

class M6502Expand16BitPreISel : public FunctionPass {
public:
  static char ID;

  M6502Expand16BitPreISel() : FunctionPass(ID) {
    initializeM6502Expand16BitPreISelPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
};

} // end anonymous namespace

char M6502Expand16BitPreISel::ID = 0;
static const char *name = "Expand 16-bit operations before instruction selection";
INITIALIZE_PASS(M6502Expand16BitPreISel, DEBUG_TYPE, name, false, false)

FunctionPass *llvm::createM6502Expand16BitPreISelPass() {
  return new M6502Expand16BitPreISel();
}

bool M6502Expand16BitPreISel::runOnFunction(Function &F) {
  bool Modified = false;

  for (BasicBlock &BB : F) {
    for (auto &BI : BB) {
      if (BinaryOperator *BO = dyn_cast<BinaryOperator>(&BI)) {
        if (BO->getOpcode() == BinaryOperator::And && BO->getType()->isIntegerTy(16)) {
          // LLVM_DEBUG(dbgs() << "Converting And instruction: " << *BO << "\n");
          // TODO
        }
      }
    }
  }

  return Modified;
}
