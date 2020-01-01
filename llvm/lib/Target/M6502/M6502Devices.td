// TODO: header stuff

// A feature set aggregates features, grouping them. We don't want to create a
// new member in M6502Subtarget (to store a value) for each set because we do not
// care if the set is supported, only the subfeatures inside the set. We fix
// this by simply setting the same dummy member for all feature sets, which is
// then ignored.
class FeatureSet<string name, string desc, list<SubtargetFeature> i>
  : SubtargetFeature<name, "m_FeatureSetDummy", "true", desc, i>;

// A family of microcontrollers, defining a set of supported features.
class Family<string name, list<SubtargetFeature> i>
  : FeatureSet<name, !strconcat("The device is a part of the ",
               name, " family"), i>;

// The device does not support decimal mode.
def FeatureNoDecimal : SubtargetFeature<"nodecimal", "m_hasNoDecimal", "true", "The device does not support decimal mode">;

//===---------------------------------------------------------------------===//
// M6502 Families
//===---------------------------------------------------------------------===//

// The device has at least the bare minimum that **every** single M6502
// device should have.
def FamilyM6502           : Family<"m6502", []>;
def FamilyM6502Nd         : Family<"m6502nd", [FeatureNoDecimal]>;

//===---------------------------------------------------------------------===//
// M6502 families supported.
//===---------------------------------------------------------------------===//

class Device<string Name, Family Fam,
             list<SubtargetFeature> ExtraFeatures = []>
  : Processor<Name, NoItineraries, !listconcat([Fam],ExtraFeatures)>;
  
def : Device<"m6502",      FamilyM6502>;
def : Device<"m6502nd",   FamilyM6502Nd>;
