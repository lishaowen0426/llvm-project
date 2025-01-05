#pragma once

#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class UnsafeAllocaPass : public PassInfoMixin<UnsafeAllocaPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

private:
  void replaceAllocaWithCustomAllocator(AllocaInst *AI, Module *M,
                                        LLVMContext &ctx,
                                        FunctionCallee &CustomAlloca);
};

} // namespace llvm