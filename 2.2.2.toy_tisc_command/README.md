# Apply new architecture
reference: https://github.com/ormastes/toy-riscv-backend/blob/main/README.md
reference: https://llvm.org/docs/WritingAnLLVMBackend.html
It is toy msp430.

## Steps
1. add project
update llvm/CMakeLists.txt
update llvm\cmake\config-ix.cmake
Copy the directory llvm\lib\Target\MSP430 to llvm\lib\Target\TOYMSP430
update llvm/lib/Target/TOYMSP430/CMakeLists.txt
```cmake
add_llvm_component_group(TOYMSP430)
```
2. apply 'toymsp430' to llvm\include\llvm\ADT\Triple.h and lib/TargetParser/Triple.cpp
change build script  LLVM_DEFAULT_TARGET_TRIPLE to toymsp430-unknown-none

change source files msp430 to toymsp430
```cmake
  \\wsl.localhost\Ubuntu-22.04\home\ormastes\dev\pri\my_toy\1.3.1.toy_msp430\llvm-project\llvm\lib\Target\TOYMSP430\Disassembler\MSP430Disassembler.cpp (1 일치)
	줄  27: #define DEBUG_TYPE "msp430-disassembler"
  \\wsl.localhost\Ubuntu-22.04\home\ormastes\dev\pri\my_toy\1.3.1.toy_msp430\llvm-project\llvm\lib\Target\TOYMSP430\AsmParser\MSP430AsmParser.cpp (1 일치)
	줄  31: #define DEBUG_TYPE "msp430-asm-parser"
  \\wsl.localhost\Ubuntu-22.04\home\ormastes\dev\pri\my_toy\1.3.1.toy_msp430\llvm-project\llvm\lib\Target\TOYMSP430\MSP430.td (2 일치)
	줄 43: def : Proc<"msp430",          []>;
	줄 44: def : Proc<"msp430x",         [FeatureX]>;
  \\wsl.localhost\Ubuntu-22.04\home\ormastes\dev\pri\my_toy\1.3.1.toy_msp430\llvm-project\llvm\lib\Target\TOYMSP430\TargetInfo\MSP430TargetInfo.cpp (2 일치)
	줄 19:   RegisterTarget<Triple::msp430> X(getTheMSP430Target(), "msp430",
  \\wsl.localhost\Ubuntu-22.04\home\ormastes\dev\pri\my_toy\1.3.1.toy_msp430\llvm-project\llvm\lib\Target\TOYMSP430\MSP430ISelDAGToDAG.cpp (1 일치)
	줄  32: #define DEBUG_TYPE "msp430-isel"
  \\wsl.localhost\Ubuntu-22.04\home\ormastes\dev\pri\my_toy\1.3.1.toy_msp430\llvm-project\llvm\lib\Target\TOYMSP430\MSP430RegisterInfo.cpp (1 일치)
	줄  28: #define DEBUG_TYPE "msp430-reg-info"
  \\wsl.localhost\Ubuntu-22.04\home\ormastes\dev\pri\my_toy\1.3.1.toy_msp430\llvm-project\llvm\lib\Target\TOYMSP430\MSP430Subtarget.cpp (2 일치)
	줄 19: #define DEBUG_TYPE "msp430-subtarget"
	줄 48:     CPUName = "msp430";
  \\wsl.localhost\Ubuntu-22.04\home\ormastes\dev\pri\my_toy\1.3.1.toy_msp430\llvm-project\llvm\lib\Target\TOYMSP430\MSP430BranchSelector.cpp (2 일치)
	줄  28: #define DEBUG_TYPE "msp430-branch-select"
	줄  31:     BranchSelectEnabled("msp430-branch-select", cl::Hidden, cl::init(true),
  \\wsl.localhost\Ubuntu-22.04\home\ormastes\dev\pri\my_toy\1.3.1.toy_msp430\llvm-project\llvm\lib\Target\TOYMSP430\MSP430ISelLowering.cpp (3 일치)
	줄   37: #define DEBUG_TYPE "msp430-lower"
	줄   40:   "msp430-no-legal-immediate", cl::Hidden,
	줄  368: // tests/codegen/msp430/shift-amount-threshold-b.ll
```
3. Change Target
build script LLVM_TARGETS_TO_BUILD to TOYMSP430
change component group to TOYMSP430 on llvm/lib/Target/TOYMSP430/CMakeLists.txt
```cmake
add_llvm_component_group(TOYMSP430)
...
add_llvm_target(TOYMSP430CodeGen
  LINK_COMPONENTS
  TOYMSP430Desc
  TOYMSP430Info

  ADD_TO_COMPONENT
  TOYMSP430
```
Change add_llvm_target to llvm/lib/Target/TOYMSP430/AsmParser/CMakeLists.txt
```cmake
add_llvm_component_library(LLVMTOYMSP430AsmParser
  LINK_COMPONENTS
  TOYMSP430Desc
  TOYMSP430Info

  ADD_TO_COMPONENT
  TOYMSP430
```
Change add_llvm_target to llvm/lib/Target/TOYMSP430/Disassembler/CMakeLists.txt
```cmake
add_llvm_component_library(LLVMTOYMSP430Disassembler
  LINK_COMPONENTS
  TOYMSP430Info

  ADD_TO_COMPONENT
  TOYMSP430  
```
Change add_llvm_target to llvm/lib/Target/TOYMSP430/MCTargetDesc/CMakeLists.txt
```cmake
add_llvm_component_library(LLVMTOYMSP430Desc
  LINK_COMPONENTS
  TOYMSP430Info

  ADD_TO_COMPONENT
  TOYMSP430
```
Change add_llvm_target to llvm/lib/Target/TOYMSP430/TargetInfo/CMakeLists.txt
```cmake
add_llvm_component_library(LLVMMSP430Info
  ADD_TO_COMPONENT
  TOYMSP430
```

Fix file which config-ix.cmake point
LLVMInitialize${LLVM_NATIVE_ARCH}Target in MSP430TargetMachine.cpp
LLVMInitialize${LLVM_NATIVE_ARCH}TargetInfo in TargetInfo\MSP430TargetInfo.cpp
LLVMInitialize${LLVM_NATIVE_ARCH}TargetMC in CTargetDesc\MSP430MCTargetDesc.cpp
LLVMInitialize${LLVM_NATIVE_ARCH}AsmPrinter in MSP430AsmPrinter.cpp
Next is optional
LLVMInitialize${LLVM_NATIVE_ARCH}AsmParser in AsmParser\MSP430AsmParser.cp
LLVMInitialize${LLVM_NATIVE_ARCH}Disassembler in Disassembler\MSP430Disassembler.cpp
```cmake
if (NOT ${LLVM_NATIVE_ARCH} IN_LIST LLVM_TARGETS_TO_BUILD)
  message(STATUS
    "Native target ${LLVM_NATIVE_ARCH} is not selected; lli will not JIT code")
else ()
  message(STATUS "Native target architecture is ${LLVM_NATIVE_ARCH}")
  set(LLVM_NATIVE_TARGET LLVMInitialize${LLVM_NATIVE_ARCH}Target)
  set(LLVM_NATIVE_TARGETINFO LLVMInitialize${LLVM_NATIVE_ARCH}TargetInfo)
  set(LLVM_NATIVE_TARGETMC LLVMInitialize${LLVM_NATIVE_ARCH}TargetMC)
  set(LLVM_NATIVE_ASMPRINTER LLVMInitialize${LLVM_NATIVE_ARCH}AsmPrinter)

  # We don't have an ASM parser for all architectures yet.
  if (EXISTS ${PROJECT_SOURCE_DIR}/lib/Target/${LLVM_NATIVE_ARCH}/AsmParser/CMakeLists.txt)
    set(LLVM_NATIVE_ASMPARSER LLVMInitialize${LLVM_NATIVE_ARCH}AsmParser)
  endif ()

  # We don't have an disassembler for all architectures yet.
  if (EXISTS ${PROJECT_SOURCE_DIR}/lib/Target/${LLVM_NATIVE_ARCH}/Disassembler/CMakeLists.txt)
    set(LLVM_NATIVE_DISASSEMBLER LLVMInitialize${LLVM_NATIVE_ARCH}Disassembler)
  endif ()
endif ()
```
## Initial file changes

### Changes on llvm/CMakeLists.txt
```cmake
### List of all targets to be built by default:
set(LLVM_ALL_TARGETS
```
add
```
TOYMSP430
```

### Add architecture in cmake config. llvm\cmake\config-ix.cmake
```
elseif (LLVM_NATIVE_ARCH MATCHES "toymsp430")
  set(LLVM_NATIVE_ARCH TOYMSP430)
else ()
  message(FATAL_ERROR "Unknown architecture ${LLVM_NATIVE_ARCH}")
endif ()
```

### Add to triple.h on include/llvm/TargetParser/Triple.h
(old include/llvm/ADT/Triple.h)
```
public:
  enum ArchType {
	  toymsp430,     
```

### Add to triple.cpp on lib/TargetParser/Triple.cpp
```
StringRef Triple::getArchTypeName(ArchType Kind) {
  switch (Kind) {
    case toymsp430:     return "toymsp430";
```
```
StringRef Triple::getArchTypePrefix(ArchType Kind) {
  switch (Kind) {
  default:
    return StringRef();
  case toymsp430:  return "toymsp430";
```
```
Triple::ArchType Triple::getArchTypeForLLVMName(StringRef Name) {
  Triple::ArchType BPFArch(parseBPFArch(Name));
  return StringSwitch<Triple::ArchType>(Name)
    .Case("toymsp430", toymsp430)
```
```
static Triple::ArchType parseArch(StringRef ArchName) {
  auto AT = StringSwitch<Triple::ArchType>(ArchName)
    .Case("toymsp430", Triple::toymsp430)
```
static Triple::ObjectFormatType getDefaultFormat(const Triple &T) {
  switch (T.getArch()) {
    case Triple::toymsp430:
	    return Triple::ELF;
```
```
static unsigned getArchPointerBitWidth(llvm::Triple::ArchType Arch) {
  switch (Arch) {
  case llvm::Triple::toymsp430:
	  return 16;
```
```
Triple Triple::get32BitArchVariant() const {
  Triple T(*this);
  switch (getArch()) {
	case Triple::toymsp430:
     T.setArch(Triple::UnknownArch); break;
```
Triple Triple::get64BitArchVariant() const {
  Triple T(*this);
  switch (getArch()) {
	case Triple::toymsp430:
     T.setArch(Triple::UnknownArch); break;
```
```
Triple Triple::getBigEndianArchVariant() const {
  Triple T(*this);
  // Already big endian.
  if (!isLittleEndian())
    return T;
  switch (getArch()) {
    ///case Triple::toymsp430:
	    T.setArch(UnknownArch);
    break;
```
```
bool Triple::isLittleEndian() const {
  switch (getArch()) {
    case Triple::toymsp430:

		return true;
```

### lib/Target/Toy_msp430 
write the new architecture