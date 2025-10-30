@echo off
pushd %~dp0
nasm -f bin -l debug/btl.lst btld/btl.asm -o build/btl.bin
nasm  btld/kernel_entry.asm -l debug/kernel_entry.lst -f elf -o build/kernel_entry.o
popd
