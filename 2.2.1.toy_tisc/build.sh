cd ../2.0.0.llvm
cd llvm-project
git switch toy_tisc
cd ..
mkdir build
cd build
# ;libcxx;libcxxabi
#  -DCMAKE_MODULE_LINKER_FLAGS="-fuse-ld=lld" -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld" 
# -DCMAKE_STATIC_LINKER_FLAGS="-fuse-ld=lld" -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" 
export CC=/usr/bin/clang-18
export CXX=/usr/bin/clang++-18
# -DLLVM_LLD_ENABLE=ON -DCMAKE_STATIC_LINKER_FLAGS="-fuse-ld=lld" -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" >> changed to >> -DLLVM_USE_LINKER=lld
cmake -G Ninja -DLLVM_USE_LINKER=lld -DLLVM_DEFAULT_TARGET_TRIPLE=tisc-unknown-none -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/home/ormastes/dev/install/clang_18_tisc_debug/lib/cmake/llvm -DLLVM_TARGETS_TO_BUILD="TISC" -DLLVM_ENABLE_PROJECTS="clang" ../llvm-project/llvm 
ninja