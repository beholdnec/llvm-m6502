// TODO: header stuff

#ifndef LLVM_M6502_ASM_BACKEND_H
#define LLVM_M6502_ASM_BACKEND_H

#include "llvm/ADT/Triple.h"
#include "llvm/MC/MCAsmBackend.h"

namespace llvm {

class MCObjectTargetWriter;

// Utilities for manipulating generated M6502 machine code.
class M6502AsmBackend : public MCAsmBackend {
public:
  M6502AsmBackend(Triple::OSType OSType)
      : MCAsmBackend(support::little), OSType(OSType) {}

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override;
  
  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override;
  
  unsigned getNumFixupKinds() const override {
    // TODO
    return 0; // M6502::NumTargetFixupKinds;
  }
  
  bool mayNeedRelaxation(const MCInst &Inst,
                         const MCSubtargetInfo &STI) const override {
    return false;
  }
  
  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const override {
    llvm_unreachable("RelaxInstruction() unimplemented");
    return false;
  }
  
  void relaxInstruction(const MCInst &Inst, const MCSubtargetInfo &STI,
                        MCInst &Res) const override {}
                        
  bool writeNopData(raw_ostream &OS, uint64_t Count) const override;

private:
  Triple::OSType OSType;
};

} // end namespace llvm

#endif // LLVM_M6502_ASM_BACKEND_H