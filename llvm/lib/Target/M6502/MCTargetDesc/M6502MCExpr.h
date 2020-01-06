// TODO: header stuff.

#ifndef LLVM_M6502_MCEXPR_H
#define LLVM_M6502_MCEXPR_H

#include "llvm/MC/MCExpr.h"

namespace llvm {

/// A expression in M6502 machine code.
class M6502MCExpr : public MCTargetExpr {
public:
  /// Specifies the type of an expression.
  enum VariantKind {
    VK_M6502_None,
    
    VK_M6502_HI8,  ///< Corresponds to `hi8()`.
    VK_M6502_LO8,  ///< Corresponds to `lo8()`.
  };
  
  /// Creates an M6502 machine code expression.
  static const M6502MCExpr *create(VariantKind Kind, const MCExpr *Expr,
                                   bool isNegated, MCContext &Ctx);

  const MCExpr *getSubExpr() const { return SubExpr; }

  bool isNegated() const { return Negated; }

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;
                                 
  void visitUsedExpr(MCStreamer &streamer) const override;
  
  MCFragment *findAssociatedFragment() const override {
    return getSubExpr()->findAssociatedFragment();
  }
  
  void fixELFSymbolsInTLSFixups(MCAssembler &Asm) const override {}

private:
  int64_t evaluateAsInt64(int64_t Value) const;
  
  const VariantKind Kind;
  const MCExpr *SubExpr;
  bool Negated;

  explicit M6502MCExpr(VariantKind Kind, const MCExpr *Expr, bool Negated)
      : Kind(Kind), SubExpr(Expr), Negated(Negated) {}
  ~M6502MCExpr() {}
};

} // end of namespace llvm

#endif // LLVM_M6502_MCEXPR_H
