@echo off
pushd %~dp0
nasm -f bin -l btl2.lst btl.asm -o btl.bin
qemu -fda btl.bin -boot c
popd
