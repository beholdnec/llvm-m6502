// TODO: header stuff.

#include "llvm/MC/MCELFObjectWriter.h"

namespace llvm {

/// Writes M6502 machine code into an ELF object file.
class M6502ELFObjectWriter : public MCELFObjectTargetWriter {
public:
  M6502ELFObjectWriter(uint8_t OSABI);
  
  unsigned getRelocType(MCContext &Ctx,
                        const MCValue &Target,
                        const MCFixup &Fixup,
                        bool IsPCRel) const override;
};

M6502ELFObjectWriter::M6502ELFObjectWriter(uint8_t OSABI)
// TODO: assign a proper ELF ID to M6502
    : MCELFObjectTargetWriter(false, OSABI, ELF::EM_NONE, true) {}

unsigned M6502ELFObjectWriter::getRelocType(MCContext &Ctx,
                                            const MCValue &Target,
                                            const MCFixup &Fixup,
                                            bool IsPCRel) const {
  switch ((unsigned) Fixup.getKind()) {
  // TODO
  default:
    llvm_unreachable("invalid fixup kind!");
  }
}

std::unique_ptr<MCObjectTargetWriter> createM6502ELFObjectWriter(uint8_t OSABI) {
  return make_unique<M6502ELFObjectWriter>(OSABI);
}

} // end of namespace llvm
