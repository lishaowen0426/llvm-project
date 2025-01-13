
#include "llvm/Transforms/Utils/ReplaceInsertValueUndef.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

Value *ReplaceInsertValueUndef::createNewValue(Instruction *I) {
  IRBuilder<> builder(I);
  Type *valueType = I->getType();

  Value *allocaInst =
      builder.CreateAlloca(valueType, nullptr, I->getName() + "_alloca");
  // Initialize the allocated memory to zero
  builder.CreateStore(Constant::getNullValue(valueType), allocaInst);

  // Load the new aggregate value
  Value *newValue =
      builder.CreateLoad(valueType, allocaInst, I->getName() + "_oldundef");

  return newValue;
}

void ReplaceInsertValueUndef::copyMetada(Instruction *from, Instruction *to) {
  SmallVector<std::pair<unsigned, MDNode *>, 4> metadata;
  from->getAllMetadata(metadata);

  for (const auto &md : metadata) {
    unsigned kind = md.first;
    MDNode *node = md.second;
    to->setMetadata(kind, node);
  }
}

PreservedAnalyses ReplaceInsertValueUndef::run(Function &F,
                                               FunctionAnalysisManager &AM) {
  bool modified = false;
  std::vector<Instruction *> toErase;

  for (auto &BB : F) {
    for (auto &I : BB) {
      if (auto *insertValueInst = dyn_cast<InsertValueInst>(&I)) {
        Value *aggregate = insertValueInst->getAggregateOperand();
        if (isa<PoisonValue>(aggregate) || isa<UndefValue>(aggregate)) {
          modified = true;

          Value *newAggregate = createNewValue(insertValueInst);
          auto *newInsertValue = InsertValueInst::Create(
              newAggregate, insertValueInst->getInsertedValueOperand(),
              insertValueInst->getIndices(),
              insertValueInst->getName() + "_newinsert", insertValueInst);

          insertValueInst->replaceAllUsesWith(newInsertValue);
          copyMetada(insertValueInst, newInsertValue);

          toErase.push_back(insertValueInst);
        }
      } else if (auto *insertElementInst = dyn_cast<InsertElementInst>(&I)) {
        Value *vector = insertElementInst->getOperand(0);
        if (isa<PoisonValue>(vector) || isa<UndefValue>(vector)) {
          modified = true;

          // Load the new vector value
          Value *newVector = createNewValue(insertElementInst);

          // Create a new InsertElementInst
          auto *newInst = InsertElementInst::Create(
              newVector, insertElementInst->getOperand(1),
              insertElementInst->getOperand(2),
              insertElementInst->getName() + "_new_insert", insertElementInst);

          insertElementInst->replaceAllUsesWith(newInst);
          copyMetada(insertElementInst, newInst);
          toErase.push_back(insertElementInst);
        }
      }
    }
  }

  // Erase the old instructions after the loop
  for (Instruction *inst : toErase) {
    inst->eraseFromParent();
  }
  return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}