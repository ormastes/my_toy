# Apply new architecture
It is base on https://github.com/ormastes/toy-riscv-backend/blob/main/README.md
Actually it does not work on 18.0 now.

## Changes on llvm/CMakeLists.txt
```
## List of all targets to be built by default:
set(LLVM_ALL_TARGETS
```
add
```
TOYRISCV
```

## Add architecture in cmake config. llvm\cmake\config-ix.cmake
```
elseif (LLVM_NATIVE_ARCH MATCHES "toyriscv32")
  set(LLVM_NATIVE_ARCH TOYRISCV)
elseif (LLVM_NATIVE_ARCH MATCHES "toyriscv64")
  set(LLVM_NATIVE_ARCH TOYRISCV)
else ()
  message(FATAL_ERROR "Unknown architecture ${LLVM_NATIVE_ARCH}")
endif ()
```

## Add to triple.h on include/llvm/TargetParser/Triple.h
(old include/llvm/ADT/Triple.h)
```
public:
  enum ArchType {
	    toyriscv32,     //
		toyriscv64,     //
```

## Add to triple.cpp on lib/Support/Triple.cpp
```
StringRef Triple::getArchTypeName(ArchType Kind) {
  switch (Kind) {
    case toyriscv32:     return "toyriscv32";
	case toyriscv64:     return "toyriscv64";
```
```
StringRef Triple::getArchTypePrefix(ArchType Kind) {
  switch (Kind) {
  default:
    return StringRef();
  case toyriscv32:
  case toyriscv64:  return "toyriscv";
```
```
Triple::ArchType Triple::getArchTypeForLLVMName(StringRef Name) {
  Triple::ArchType BPFArch(parseBPFArch(Name));
  return StringSwitch<Triple::ArchType>(Name)
    .Case("toyriscv32", toyriscv32)
    .Case("toyriscv64", toyriscv64)
```
```
static Triple::ArchType parseArch(StringRef ArchName) {
  auto AT = StringSwitch<Triple::ArchType>(ArchName)
    .Case("toyriscv32", Triple::toyriscv32)
    .Case("toyriscv64", Triple::toyriscv64)
```
static Triple::ObjectFormatType getDefaultFormat(const Triple &T) {
  switch (T.getArch()) {
    case Triple::toyriscv32:
    case Triple::toyriscv64:
	    return Triple::ELF;
```
```
static unsigned getArchPointerBitWidth(llvm::Triple::ArchType Arch) {
  switch (Arch) {
    case llvm::Triple::toyriscv32:
	  return 32;
	case llvm::Triple::toyriscv64:	
      return 64;
```
```
Triple Triple::get32BitArchVariant() const {
  Triple T(*this);
  switch (getArch()) {
    case Triple::toyriscv32:
    // Already 32-bit.
	break;
	case Triple::toyriscv64:     T.setArch(Triple::toyriscv32); break;
```
Triple Triple::get64BitArchVariant() const {
  Triple T(*this);
  switch (getArch()) {
    case Triple::toyriscv64:
	  // Already 64-bit.
      break;
	case Triple::toyriscv32:      T.setArch(Triple::toyriscv64); break;
```
```
Triple Triple::getBigEndianArchVariant() const {
  Triple T(*this);
  // Already big endian.
  if (!isLittleEndian())
    return T;
  switch (getArch()) {
    case Triple::toyriscv32:
    case Triple::toyriscv64:
	    T.setArch(UnknownArch);
    break;
```
```
bool Triple::isLittleEndian() const {
  switch (getArch()) {
    case Triple::toyriscv32:
	case Triple::toyriscv64:
		return true;
```

## lib/Target/TOYRISCV copy
lib/Target/TOYRISCV need to be copied to llvm/lib/Target/TOYRISCV