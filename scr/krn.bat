@echo off
pushd %~dp0..

nasm kernel/isr.asm -f elf -o build/isr.o

wsl -e bash -ic ^
"i686-elf-gcc -ffreestanding -c kernel/*.c"
mv *.o build/
mkdir build\\entry
mv build/kernel_entry.o build/entry/kernel_entry.o
wsl -e bash -ic ^
"i686-elf-ld  -Ttext 0x8600 build/entry/kernel_entry.o build/*.o --oformat binary -o build/kernel.bin"

popd
