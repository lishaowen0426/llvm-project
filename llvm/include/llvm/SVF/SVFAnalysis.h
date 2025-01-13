#pragma once

#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include <set>

namespace llvm {

class SVFAnalysis : public AnalysisInfoMixin<SVFAnalysis> {
public:
  using Result = std::set<const AllocaInst *>;
  Result run(Module &M, ModuleAnalysisManager &AM);

private:
  friend AnalysisInfoMixin<SVFAnalysis>;

  static AnalysisKey Key;
};

} // namespace llvm