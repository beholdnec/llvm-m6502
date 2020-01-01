// TODO: header stuff

#include "M6502AsmBackend.h"

#include "MCTargetDesc/M6502MCTargetDesc.h"

#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"

namespace llvm {

std::unique_ptr<MCObjectTargetWriter>
M6502AsmBackend::createObjectTargetWriter() const {
  return createM6502ELFObjectWriter(MCELFObjectTargetWriter::getOSABI(OSType));
}

void M6502AsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                                 const MCValue &Target,
                                 MutableArrayRef<char> Data, uint64_t Value,
                                 bool IsResolved,
                                 const MCSubtargetInfo *STI) const {
  // TODO
}

bool M6502AsmBackend::writeNopData(raw_ostream &OS, uint64_t Count) const {
  // TODO
  return true;
}

MCAsmBackend *createM6502AsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                    const MCRegisterInfo &MRI,
                                    const llvm::MCTargetOptions &TO) {
  return new M6502AsmBackend(STI.getTargetTriple().getOS());
}

} // end of namespace llvm
