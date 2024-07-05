# Apply new architecture
reference: https://github.com/ormastes/toy-riscv-backend/blob/main/README.md
reference: https://llvm.org/docs/WritingAnLLVMBackend.html
reference: https://github.com/PacktPublishing/Learn-LLVM-17/tree/main/Chapter11



## Initial file changes
add 
TISCAtttribute.h and TISCAtttributeParser.h
add
ELFReloc/TISC.def

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


### Optional
MCELFObjectWriter.h 
file need to changed for OS abi
```
  static uint8_t getOSABI(Triple::OSType OSType) {
```
IR/CallingConv.h
```
namespace CallingConv {

  /// LLVM IR allows to use arbitrary numbers as calling convention identifiers.
  using ID = unsigned;

  /// A set of enums which specify the assigned numeric values for known llvm
  /// calling conventions.
  /// LLVM Calling Convention Representation
  enum {
    TISC_BUILTIN =111,
```
ELFObjectFile.cpp
```
template <class ELFT>
StringRef ELFObjectFile<ELFT>::getFileFormatName() const {
      case ELF::EM_TISC:
      return "elf32-tisc";

template <class ELFT> Triple::ArchType ELFObjectFile<ELFT>::getArch() const {
  case ELF::EM_TISC:
    return Triple::tisc;
```

Object/ELF.cpp
```
StringRef llvm::object::getELFRelocationTypeName()
case ELF::EM_TISC:
    switch (Type) {
#include "llvm/BinaryFormat/ELFRelocs/TISC.def"
    default:
      break;
    }
    break;
```
Object/ELFObjectFile.cpp
```
std::optional<StringRef> ELFObjectFileBase::tryGetCPUName() const {
  switch (getEMachine()) {

  case ELF::EM_TISC:
    return StringRef("tisc");
```
ELFYAML.cpp
```
void ScalarEnumerationTraits<ELFYAML::ELF_REL>::enumeration(
      case ELF::EM_TISC:
#include "llvm/BinaryFormat/ELFRelocs/TISC.def"
    break;
```