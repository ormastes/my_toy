#pragma once 


class Interpreter {
private:
  std::unique_ptr<llvm::orc::ExecutionSession> ES;

  llvm::DataLayout DL;
  llvm::orc:: MangleAndInterner Mangle;

  llvm::orc::RTDyldObjectLinkingLayer ObjectLayer;
  llvm::orc::IRCompileLayer CompileLayer;

  llvm::orc::JITDylib &MainJD;

  // JTMB is the JITTargetMachineBuilder: ->TargetMachine, TargetTriple
  // EPC is the ExecutorProcessControl: ->JITLinkMemoryManager, JITLinkAsyncLookupService, JITLinkExecutorProcessControl
  // MainJD is the JITDylib that will hold all modules
  // CompileLayer is the IR compiler layer: ->ES, ObjectLayer, ConcurrentIRCompiler(JTMB:JITTargetMachineBuilder)
  // ObjectLayer is the RTDyldObjectLinkingLayer: ->ES, SectionMemoryManager
  // Mangle is the MangleAndInterner: ->ES, DL
  // DL is the DataLayout, Here data layout is the ABI: ->ES, TargetTriple
  // ES is the ExecutionSession: ->EPC(SelfExecutorProcessControl)

public:
  Interpreter(std::unique_ptr<llvm::orc::ExecutionSession> ES, llvm::orc::JITTargetMachineBuilder JTMB, llvm::DataLayout DL)
      : ES(std::move(ES)), 
        DL(std::move(DL)), 
        Mangle(*this->ES, this->DL),
        ObjectLayer(*this->ES,[]() { return std::make_unique<llvm::SectionMemoryManager>(); }),
        CompileLayer(*this->ES, ObjectLayer, std::make_unique<llvm::orc::ConcurrentIRCompiler>(std::move(JTMB))),
        MainJD(this->ES->createBareJITDylib("<main>")) {
    
    // It find current binary interfaces, but currently not working why??
    auto searchGen = cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(DL.getGlobalPrefix()));
    MainJD.addGenerator(std::move(searchGen));
    if (JTMB.getTargetTriple().isOSBinFormatCOFF()) {
      ObjectLayer.setOverrideObjectFlagsWithResponsibilityFlags(true);
      ObjectLayer.setAutoClaimResponsibilityForObjectSymbols(true);
    }
  }

  ~Interpreter() {
    if (auto Err = ES->endSession()) ES->reportError(std::move(Err));
  }

  static llvm::Expected<std::unique_ptr<Interpreter>> Create() {
    auto EPC = llvm::orc::SelfExecutorProcessControl::Create();
    if (!EPC) return EPC.takeError();

    auto ES = std::make_unique<llvm::orc::ExecutionSession>(std::move(*EPC));

    llvm::orc::JITTargetMachineBuilder JTMB(ES->getExecutorProcessControl().getTargetTriple());

    auto DL = JTMB.getDefaultDataLayoutForTarget();
    if (!DL) return DL.takeError();

    return std::make_unique<Interpreter>(std::move(ES), std::move(JTMB), std::move(*DL));
  }

  const llvm::DataLayout &getDataLayout() const { return DL; }

  llvm::orc::JITDylib &getMainJITDylib() { return MainJD; }

  llvm::Error addModule(llvm::orc::ThreadSafeModule TSM, llvm::orc::ResourceTrackerSP RT = nullptr) {
    if (!RT) RT = MainJD.getDefaultResourceTracker();
    return CompileLayer.add(RT, std::move(TSM));
  }

  llvm::Expected<llvm::orc::ExecutorSymbolDef> lookup(llvm::StringRef Name) {
    return ES->lookup({&MainJD}, Mangle(Name.str()));
  }
};

