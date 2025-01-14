#include "llvm/Transforms/Utils/UnsafeAlloca.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "unsafe-alloca"

using namespace llvm;

PreservedAnalyses UnsafeAllocaPass::run(Function &F,
                                        FunctionAnalysisManager &AM) {

  Module *M = F.getParent();
  LLVMContext &ctx = M->getContext();

  Type *ReturnType = PointerType::get(Type::getInt8Ty(ctx), 0);
  std::vector<Type *> ArgTypes = {Type::getInt64Ty(ctx), Type::getInt64Ty(ctx)};
  FunctionCallee CustomAlloc = M->getOrInsertFunction(
      "miri_unsafe_alloc", FunctionType::get(ReturnType, ArgTypes, false));
  std::vector<Instruction *> InstructionsToErase;

  for (auto &BB : F) {
    for (auto &I : BB) {
      if (auto *AI = dyn_cast<AllocaInst>(&I)) {
        if (MDNode *MD = AI->getMetadata("miri-target" /*kind*/)) {
          for (unsigned i = 0; i < MD->getNumOperands(); i++) {
            if (auto *MDStr = dyn_cast<MDString>(MD->getOperand(i))) {
              if (MDStr->getString() == "unsafe_alloca") {
                replaceAllocaWithCustomAllocator(AI, M, ctx, CustomAlloc);
                InstructionsToErase.push_back(AI);
              }
            }
          }
        }
      }
    }
  }
  for (Instruction *Inst : InstructionsToErase) {
    Inst->eraseFromParent();
  }

  // we set up the unsafe stack only if unsafe_alloca exists
  if (!InstructionsToErase.empty()) {
    /// insert miri_setup_unsafe_stack at entry
    FunctionCallee SetupFunction =
        M->getOrInsertFunction("miri_setup_unsafe_stack",
                               FunctionType::get(Type::getVoidTy(ctx), false));
    IRBuilder<> EntryBuilder(&*F.getEntryBlock().getFirstInsertionPt());
    EntryBuilder.CreateCall(SetupFunction);

    /// insert miri_reset_unsafe_alloc before return
    FunctionCallee ResetFunction =
        M->getOrInsertFunction("miri_reset_unsafe_alloc",
                               FunctionType::get(Type::getVoidTy(ctx), false));

    for (auto &BB : F) {
      Instruction *Terminator = BB.getTerminator();
      if (isa<ReturnInst>(Terminator)) {
        IRBuilder<> Builder(Terminator);
        Builder.CreateCall(ResetFunction);
      }
    }
  }

  // register this pass as early as possible since it invalidate all analysis
  return PreservedAnalyses::none();
}

void UnsafeAllocaPass::replaceAllocaWithCustomAllocator(
    AllocaInst *AI, Module *M, LLVMContext &ctx, FunctionCallee &CustomAlloca) {
  LLVM_DEBUG(dbgs() << "unsafe alloca: " << *AI << "\n");

  IRBuilder<> Builder(AI);

  Type *AllocatedType = AI->getAllocatedType();
  DataLayout DL(M);
  uint64_t TypeSize = DL.getTypeAllocSize(AllocatedType);
  unsigned Alignment = AI->getAlign().value();

  // Create the arguments for the custom allocator
  Value *Size = ConstantInt::get(Type::getInt64Ty(ctx), TypeSize);
  Value *Align = ConstantInt::get(Type::getInt64Ty(ctx), Alignment);

  // Call the custom allocator
  CallInst *Call = Builder.CreateCall(CustomAlloca, {Size, Align});

  // Cast the returned pointer to the allocated type
  Value *Cast = Builder.CreateBitCast(Call, AI->getType());

  AI->replaceAllUsesWith(Cast);
}