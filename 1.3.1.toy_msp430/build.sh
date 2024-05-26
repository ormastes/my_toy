mkdir build
cd build
# ;libcxx;libcxxabi
cmake -G Ninja -DLLVM_DEFAULT_TARGET_TRIPLE=msp430-unknown-none -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/home/ormastes/dev/install/clang_17_xar_debug/lib/cmake/llvm -DLLVM_TARGETS_TO_BUILD="MSP430" -DLLVM_ENABLE_PROJECTS="clang" ../llvm-project/llvm 
ninja