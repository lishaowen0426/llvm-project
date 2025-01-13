#pragma once

#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class ReplaceInsertValueUndef : public PassInfoMixin<ReplaceInsertValueUndef> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

private:
  Value *createNewValue(Instruction *I);
  void copyMetada(Instruction *from, Instruction *to);
};

} // namespace llvm