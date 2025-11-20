@echo off
pushd %~dp0..

nasm src/kernel/isr.asm -f elf -o build/isr.o

wsl -e bash -ic ^
"i686-elf-gcc -ffreestanding -Isrc/ -Isrc/prog/ -Isrc/kernel/ -c src/*.c src/kernel/*.c src/prog/*.c"
mv *.o build/
mkdir build\\entry
mv build/kernel_entry.o build/entry/kernel_entry.o

rem Link

wsl -e bash -ic ^
"i686-elf-ld  -T scr/linker.ld build/entry/kernel_entry.o build/*.o  -o build/kernel.elf"
wsl -e bash -ic ^
"i686-elf-objcopy -O binary build/kernel.elf build/kernel.bin"


popd
