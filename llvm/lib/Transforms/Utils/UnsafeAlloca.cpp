#include "llvm/Transforms/Utils/UnsafeAlloca.h"
using namespace llvm;

PreservedAnalyses UnsafeAllocaPass::run(Function &F,
                                        FunctionAnalysisManager &AM) {
  return PreservedAnalyses::all();
}