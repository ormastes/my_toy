mkdir build
cd build
# ;libcxx;libcxxabi
#  -DCMAKE_MODULE_LINKER_FLAGS="-fuse-ld=lld" -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld" 
# -DCMAKE_STATIC_LINKER_FLAGS="-fuse-ld=lld" -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" 

cmake -G Ninja -DLLVM_LLD_ENABLE=ON -DLLVM_DEFAULT_TARGET_TRIPLE=toymsp43_-unknown-none -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/home/ormastes/dev/install/clang_17_X86_reldeb/lib/cmake/llvm -DLLVM_TARGETS_TO_BUILD="TOYMSP43_" -DLLVM_ENABLE_PROJECTS="clang" ../llvm-project/llvm 
ninja