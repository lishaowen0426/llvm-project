#include "llvm/SVF/SVFTransform.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/SVF/SVFAnalysis.h"

using namespace llvm;
using namespace std;

PreservedAnalyses SVFTransform::run(Module &M, ModuleAnalysisManager &MAM) {
  auto &Result = MAM.getResult<SVFAnalysis>(M);

  LLVMContext &ctx = M.getContext();
  unsigned mkind = ctx.getMDKindID("svf-target");
  MDNode *mnode = MDNode::get(ctx, MDString::get(ctx, "unsafe_alloca"));

  for (auto &F : M) {
    for (auto &B : F) {
      for (auto &I : B) {
        if (auto *allocaInst = dyn_cast<AllocaInst>(&I)) {
          if (Result.find(allocaInst) != Result.end()) {
            //           outs() << "find: " << *allocaInst << "\n";
            allocaInst->setMetadata(mkind, mnode);
          }
        }
      }
    }
  }
  return PreservedAnalyses::all();
}