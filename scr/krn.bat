@echo off
pushd %~dp0..

nasm kernel/isr.asm -f elf -o build/isr.o

wsl -e bash -ic ^
"i686-elf-gcc -ffreestanding -c kernel/*.c"
mv *.o build/
mkdir build\\entry
mv build/kernel_entry.o build/entry/kernel_entry.o

rem Link

wsl -e bash -ic ^
"i686-elf-ld  -T scr/linker.ld build/entry/kernel_entry.o build/*.o  -o build/kernel.elf"
wsl -e bash -ic ^
"i686-elf-objcopy -O binary build/kernel.elf build/kernel.bin"

rem wsl -e bash -ic ^
rem "i686-elf-ld  -Ttext 0x100000 build/entry/kernel_entry.o build/*.o --oformat binary -o build/kernel.bin"

popd
