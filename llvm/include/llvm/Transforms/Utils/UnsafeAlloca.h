#pragma once

#include "llvm/IR/PassManager.h"

namespace llvm {

class UnsafeAllocaPass : public PassInfoMixin<UnsafeAllocaPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace llvm