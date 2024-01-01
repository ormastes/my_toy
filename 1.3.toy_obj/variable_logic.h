#pragma once

#include "llvm/IR/LegacyPassManager.h"

#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"



//using namespace llvm;
//using namespace llvm::sys;

void variable_bootup_init();

void variable_InitializeModule();
void variable_functionast_codegen();
void variable_post_main();
