// TODO: header stuff

// XXX: The M6502 backend doesn't currently use LLVM's Calling Convention machinery.
// Here, it is effectively disabled by assigning all return values to the stack.
// TODO: Handle Calling Conventions in a late compilation stage, perhaps as part
// of optimization. The idea is that this stage will use a heuristic to decide
// the best way to pass return values to a caller.
def RetCC_M6502 : CallingConv<[
  CCAssignToStack<1, 1>
]>;

// XXX: The M6502 backend doesn't currently use LLVM's Calling Convention machinery.
// Here, it is effectively disabled by assigning all arguments to the stack.
// TODO: Handle Calling Conventions in a late compilation stage, perhaps as part
// of optimization. The idea is that this stage will use a heuristic to decide
// the best way to pass arguments to a function.
def CC_M6502 : CallingConv<[
  CCAssignToStack<1, 1>
]>;
