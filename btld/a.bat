@echo off
nasm -f bin btl.asm -o btl.bin
qemu btl.bin
