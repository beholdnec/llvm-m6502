// TODO: header stuff

#include "M6502MCExpr.h"

#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCValue.h"

namespace llvm {

const M6502MCExpr *M6502MCExpr::create(VariantKind Kind, const MCExpr *Expr,
                                       bool Negated, MCContext &Ctx) {
  return new (Ctx) M6502MCExpr(Kind, Expr, Negated);
}

void M6502MCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  assert(Kind != VK_M6502_None);

  if (isNegated())
    OS << '-';

  if (Kind == VK_M6502_HI8)
    OS << '>';

  if (Kind == VK_M6502_LO8)
    OS << '<';

  getSubExpr()->print(OS, MAI);
}

bool M6502MCExpr::evaluateAsRelocatableImpl(MCValue &Result,
                                            const MCAsmLayout *Layout,
                                            const MCFixup *Fixup) const {
  MCValue Value;
  bool isRelocatable = SubExpr->evaluateAsRelocatable(Value, Layout, Fixup);

  if (!isRelocatable)
    return false;

  if (Value.isAbsolute()) {
    Result = MCValue::get(evaluateAsInt64(Value.getConstant()));
  } else {
    if (!Layout) return false;

    MCContext &Context = Layout->getAssembler().getContext();
    const MCSymbolRefExpr *Sym = Value.getSymA();
    MCSymbolRefExpr::VariantKind Modifier = Sym->getKind();
    if (Modifier != MCSymbolRefExpr::VK_None)
      return false;

    Sym = MCSymbolRefExpr::create(&Sym->getSymbol(), Modifier, Context);
    Result = MCValue::get(Sym, Value.getSymB(), Value.getConstant());
  }

  return true;
}

int64_t M6502MCExpr::evaluateAsInt64(int64_t Value) const {
  if (Negated)
    Value *= -1;

  switch (Kind) {
  case M6502MCExpr::VK_M6502_LO8:
    Value &= 0xff;
    break;
  case M6502MCExpr::VK_M6502_HI8:
    Value &= 0xff00;
    Value >>= 8;
    break;

  case M6502MCExpr::VK_M6502_None:
    llvm_unreachable("Uninitialized expression.");
  }
  return static_cast<uint64_t>(Value) & 0xff;
}

void M6502MCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*getSubExpr());
}

} // end of namespace llvm
