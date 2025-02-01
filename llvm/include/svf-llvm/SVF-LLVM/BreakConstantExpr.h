//===- BreakConstantGEPs.h - Change constant GEPs into GEP instructions --- --//
//
//                          The SAFECode Compiler
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass changes all GEP constant expressions into GEP instructions.  This
// permits the rest of SAFECode to put run-time checks on them if necessary.
//
//===----------------------------------------------------------------------===//

#ifndef BREAKCONSTANTGEPS_H
#define BREAKCONSTANTGEPS_H

#include "BasicTypes.h"
#include "SVFIR/SVFValue.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"

namespace SVF
{

//
// Pass: BreakConstantGEPs
//
// Description:
//  This pass modifies a function so that it uses GEP instructions instead of
//  GEP constant expressions.
//
class BreakConstantGEPs : public ModulePass
{
private:
    // Private methods

    // Private variables

public:
    static char ID;
    BreakConstantGEPs() : ModulePass(ID) {}
    llvm::StringRef getPassName() const
    {
        return "Remove Constant GEP Expressions";
    }
    virtual bool runOnModule(Module& M);
};

//
// Pass: MergeFunctionRets
//
// Description:
//  This pass modifies a function so that each function only have one unified
//  exit basic block
//
/*
class MergeFunctionRets : public ModulePass
{
private:
    // Private methods

    // Private variables

public:
    static char ID;
    MergeFunctionRets() : ModulePass(ID) {}
    llvm::StringRef getPassName() const
    {
        return "unify function exit into one dummy exit basic block";
    }
    virtual bool runOnModule(Module& M)
    {
        UnifyFunctionExit(M);
        return true;
    }
    void getAnalysisUsage(llvm::AnalysisUsage& AU) const override
    {
        AU.addRequired<UnifyFunctionExitNodes>();
    }

    inline void UnifyFunctionExit(Module& module)
    {

        // Set up a FunctionAnalysisManager for running the new pass
        llvm::FunctionAnalysisManager FAM;

        // Register required analyses for the FunctionAnalysisManager
        llvm::PassBuilder PB;
        PB.registerFunctionAnalyses(FAM);
        for (Function& F : module)
        {
            if (F.isDeclaration())
                continue;
            UnifyFunctionExitNodes& AnalysisPass =
                getAnalysis<UnifyFunctionExitNodes>();
            AnalysisPass.run(F, FAM);
        }
    }
};
*/

} // End namespace SVF

#endif
