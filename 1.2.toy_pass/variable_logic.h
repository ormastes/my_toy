#pragma once
// pass support
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"


//using namespace llvm;
//using namespace llvm::orc;

void variable_bootup_init();

void variable_InitializeModule();
void variable_functionast_codegen();
