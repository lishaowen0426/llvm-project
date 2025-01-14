#include "llvm/SVF/SVFAnalysis.h"
#include "Graphs/SVFG.h"
#include "SVF-LLVM/LLVMUtil.h"
#include "SVF-LLVM/SVFIRBuilder.h"
#include "Util/SVFUtil.h"
#include "WPA/Andersen.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include <cstdlib>
#include <deque>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace llvm;
using namespace std;
using namespace SVF;

AnalysisKey SVFAnalysis::Key;

typedef struct {
  Value *dest;
  std::vector<Value *> val;
} TaintSource;

class SvfTainter {
private:
  std::vector<TaintSource> taintSources;
  std::set<const AllocaInst *> alloca_to_replace;
  std::set<const SVFStmt *> visited;
  Module *M;
  LLVMModuleSet *mSet;
  SVFIR *pag;
  Andersen *ander;
  VFG *vfg;

  void processIRForTaintSource(Instruction &I);

  void processDestVal(const Value *dest);
  void processEdge(const SVFStmt *e, bool forward);
  void collectTaintSources();

  void propagateTaint();

public:
  SvfTainter(Module *M) : M(M) {
    SVFModule *svfModule = LLVMModuleSet::buildSVFModule(*M);
    SVFIRBuilder svfIRbuilder(svfModule);
    mSet = LLVMModuleSet::getLLVMModuleSet();
    pag = svfIRbuilder.build();
    {
      std::streambuf *originalBuffer = std::cout.rdbuf();
      std::ostringstream nullStream;
      std::cout.rdbuf(nullStream.rdbuf());
      ander = AndersenWaveDiff::createAndersenWaveDiff(pag);
      vfg = new VFG(ander->getCallGraph());

      std::cout.rdbuf(originalBuffer);
    }
  }
  void process() {
    collectTaintSources();
    propagateTaint();
  }

  set<const AllocaInst *> result() { return alloca_to_replace; }
};

void SvfTainter::collectTaintSources() {
  for (auto &F : *M) {
    for (auto &BB : F) {
      for (auto &I : BB) {
        if (MDNode *MD = I.getMetadata("svf")) {
          // outs() << "Found instruction: " << I << "\n";
          processIRForTaintSource(I);
        }
      }
    }
  }
}

/// The store instruction in LLVM IR is indeed the most common instruction
/// for modifying the value at a specific memory location,
/// but it is not the only instruction that can change memory.
/// There are other instructions that can also modify memory content.
void SvfTainter::processIRForTaintSource(Instruction &I) {
  if (auto *allocaInst = dyn_cast<AllocaInst>(&I)) {
    taintSources.push_back(TaintSource{allocaInst, {}});
  } else if (auto *storeInst = dyn_cast<StoreInst>(&I)) {
    taintSources.push_back(TaintSource{storeInst->getPointerOperand(), {}});

  } else if (auto *atomicrmwInst = dyn_cast<AtomicRMWInst>(&I)) {
    taintSources.push_back(TaintSource{atomicrmwInst->getPointerOperand(), {}});
  } else if (auto *cmpxchgInst = dyn_cast<AtomicCmpXchgInst>(&I)) {
    taintSources.push_back(TaintSource{cmpxchgInst->getPointerOperand(), {}});
  } else if (auto *insertValueInst = dyn_cast<InsertValueInst>(&I)) {
    taintSources.push_back(
        TaintSource{insertValueInst->getAggregateOperand(), {}});
  } else if (auto *insertElemInst = dyn_cast<InsertElementInst>(&I)) {
    taintSources.push_back(TaintSource{insertElemInst->getOperand(0), {}});
  } else if (auto *callInst = dyn_cast<CallInst>(&I)) {
    std::vector<Value *> args;
    for (unsigned i = 0; i < callInst->arg_size(); ++i) {
      llvm::Value *arg = callInst->getArgOperand(i); // Get the argument
      args.push_back(arg);
    }
    if (callInst->getType()->isVoidTy()) {
      taintSources.push_back(TaintSource{nullptr, args});
    } else {
      taintSources.push_back(TaintSource{callInst, args});
    }
  }
}

void SvfTainter::processEdge(const SVFStmt *e, bool forward) {
  if (visited.find(e) == visited.end()) {
    visited.insert(e);
    auto *node = forward ? e->getDstNode() : e->getSrcNode();
    if (node->hasValue()) {
      processDestVal(mSet->getLLVMValue(node->getValue()));
    }
  } else {
    // std::cout << "edge: " << *e << " has been visited\n";
  }
}

void SvfTainter::processDestVal(const Value *dest) {
  // outs() << "dest llvm value: " << *dest << "\n";
  SVFValue *svfval = mSet->getSVFValue(dest);
  if (pag->hasValueNode(svfval)) {
    PAGNode *current = pag->getGNode(pag->getValueNode(svfval));
    if (current->hasValue()) {
      if (auto *allocaInst =
              dyn_cast<AllocaInst>(mSet->getLLVMValue(current->getValue()))) {
        alloca_to_replace.insert(allocaInst);

        for (auto e : current->getOutEdges()) {
          processEdge(e, true);
        }

        return;
      } else if (auto extracInst = dyn_cast<ExtractValueInst>(
                     mSet->getLLVMValue(current->getValue()))) {
        return processDestVal(extracInst->getAggregateOperand());
      } else if (auto insertInst = dyn_cast<InsertValueInst>(
                     mSet->getLLVMValue(current->getValue()))) {

        return processDestVal(insertInst->getAggregateOperand());
      } else if (auto insertElemInst = dyn_cast<InsertElementInst>(
                     mSet->getLLVMValue(current->getValue()))) {
        return processDestVal(insertElemInst->getOperand(0));
      } else if (auto extractElemInst = dyn_cast<ExtractElementInst>(
                     mSet->getLLVMValue(current->getValue()))) {
        return processDestVal(extractElemInst->getOperand(0));
      }
    }

    for (auto e : current->getInEdges()) {
      processEdge(e, false);
    }

  } else {
    std::cerr << "SVF value: " << *svfval << " has no pag node\n";
    abort();
  }
}

void SvfTainter::propagateTaint() {

  for (auto src : taintSources) {

    const Value *dest = src.dest;
    if (dest != nullptr) {
      processDestVal(dest);
    }

    for (auto arg : src.val) {
      processDestVal(arg);
    }
  }
}

SVFAnalysis::Result SVFAnalysis::run(Module &M, ModuleAnalysisManager &MAM) {
  SvfTainter tainter(&M);
  std::cout << "before analysis"
            << "\n";
  tainter.process();
  std::cout << "after analysis"
            << "\n";

  return tainter.result();
}