@echo off
pushd %~dp0..
cat build/btl.bin build/kernel.bin > build/osimg.bin
start cmd /c "qemu -fda build/osimg.bin -boot c"
popd
