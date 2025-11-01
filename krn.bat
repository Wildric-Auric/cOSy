@echo off

nasm kernel/isr.asm -f elf -o build/isr.o

wsl -e bash -ic ^
"i686-elf-gcc -ffreestanding -c kernel/*.c"
mv *.o build/
wsl -e bash -ic ^
"i686-elf-ld  -Ttext 0x1000 build/kernel_entry.o build/isr.o build/inter.o build/kernel.o --oformat binary -o build/kernel.bin"


