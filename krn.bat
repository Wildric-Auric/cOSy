@echo off
wsl -e bash -ic ^
"i686-elf-gcc -ffreestanding -c kernel/kernel.c -o build/kernel.o"
wsl -e bash -ic ^
"i686-elf-ld  -Ttext 0x1000 build/kernel_entry.o build/kernel.o --oformat binary -o build/kernel.bin"
