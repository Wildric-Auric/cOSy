@echo off
pushd %~dp0
nasm -f bin btl.asm -o btl.bin
qemu -fda btl.bin -boot c
popd
