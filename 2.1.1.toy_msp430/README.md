# Apply new architecture
reference: https://github.com/ormastes/toy-riscv-backend/blob/main/README.md
reference: https://llvm.org/docs/WritingAnLLVMBackend.html

tisc : count 58
TOYMSP43_ : count 35
total 93

## Steps
1. add project
update llvm/CMakeLists.txt
```cmake
# List of all targets to be built by default:
set(LLVM_ALL_TARGETS
...
  TISC
```
update llvm\cmake\config-ix.cmake
```cmake
# Determine the native architecture.
...
elseif (LLVM_NATIVE_ARCH MATCHES "tisc")
  set(LLVM_NATIVE_ARCH TISC)
```
Copy the directory llvm\lib\Target\MSP430 to llvm\lib\Target\TISC
update llvm/lib/Target/TISC/CMakeLists.txt
```cmake
add_llvm_component_group(TISC)
```
2. apply 'toymsp430' to llvm\include\llvm\TargetParser\Triple.h and lib/TargetParser/Triple.cpp
```cpp
# llvm\include\llvm\TargetParser\Triple.h 
class Triple {
public:
  enum ArchType {
...
    tisc,
```

```cpp
# llvm/lib/TargetParser/Triple.cpp
StringRef Triple::getArchTypeName(ArchType Kind) {
  switch (Kind) {
...
  case tisc:     return "tisc";

StringRef Triple::getArchTypePrefix(ArchType Kind) {
  switch (Kind) {
...
  case tisc:  return "tisc";

Triple::ArchType Triple::getArchTypeForLLVMName(StringRef Name) {
  Triple::ArchType BPFArch(parseBPFArch(Name));
  return StringSwitch<Triple::ArchType>(Name)
...
    .Case("tisc", tisc)

static Triple::ArchType parseArch(StringRef ArchName) {
  auto AT = StringSwitch<Triple::ArchType>(ArchName)
...
    .Case("tisc", Triple::tisc)

static Triple::ObjectFormatType getDefaultFormat(const Triple &T) {
  case Triple::tisc:


## CHANGE IT for target ###########################################
static unsigned getArchPointerBitWidth(llvm::Triple::ArchType Arch) {
  switch (Arch) {
... // choose 16/32/64...
  case llvm::Triple::tisc:

Triple Triple::get32BitArchVariant() const {
  Triple T(*this);
  switch (getArch()) {
...
  case Triple::tisc:
    T.setArch(UnknownArch);
    break;

Triple Triple::get64BitArchVariant() const {
  Triple T(*this);
  switch (getArch()) {
...
    T.setArch(UnknownArch);
    break;

## CHANGE IT for target ###########################################
Triple Triple::getBigEndianArchVariant() const {
...
  case Triple::tisc:
    T.setArch(UnknownArch);
    break;

bool Triple::isLittleEndian() const {
...
  case Triple::tisc:
    return true;


```

## to rename files

```
sudo apt-get install rename
find . -type f -name 'MSP430*' -print0 | xargs -0 rename 's/MSP430/TISC/'
```

## Add aditional files
ELF.h
```cpp

// Machine architectures
// See current registered ELF machine architectures at:
//    http://www.uxsglobal.com/developers/gabi/latest/ch4.eheader.html
enum {
  EM_TISC = 259,       // TISC


// TISC specific e_flags
enum : unsigned {
  EF_TISC_MACH_TISCx11 = 11,
  EF_TISC_MACH_TISCx11x1 = 110,
  EF_TISC_MACH_TISCx12 = 12,
  EF_TISC_MACH_TISCx13 = 13,
  EF_TISC_MACH_TISCx14 = 14,
  EF_TISC_MACH_TISCx15 = 15,
  EF_TISC_MACH_TISCx16 = 16,
  EF_TISC_MACH_TISCx20 = 20,
  EF_TISC_MACH_TISCx22 = 22,
  EF_TISC_MACH_TISCx23 = 23,
  EF_TISC_MACH_TISCx24 = 24,
  EF_TISC_MACH_TISCx26 = 26,
  EF_TISC_MACH_TISCx31 = 31,
  EF_TISC_MACH_TISCx32 = 32,
  EF_TISC_MACH_TISCx33 = 33,
  EF_TISC_MACH_TISCx41 = 41,
  EF_TISC_MACH_TISCx42 = 42,
  EF_TISC_MACH_TISCx43 = 43,
  EF_TISC_MACH_TISCx44 = 44,
  EF_TISC_MACH_TISCX = 45,
  EF_TISC_MACH_TISCx46 = 46,
  EF_TISC_MACH_TISCx47 = 47,
  EF_TISC_MACH_TISCx54 = 54,
};

// ELF Relocation types for TISC
enum {
#include "ELFRelocs/TISC.def"
};

// Section types.
enum : unsigned {
  SHT_TISC_ATTRIBUTES = 0x70000003U,
```
Add file llvm/include/llvm/BinaryFormat/ELFRelocs/TISC.def


CodeView.h
```cpp
enum class CallingConvention : uint8_t {