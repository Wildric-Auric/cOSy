export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"
#i686-elf-gcc -o -ffreestanding -c f.c -o f.o 
#i686-elf-ld -o f.bin -Ttext 0x0 --oformat binary f.o

i686-elf-gcc -ffreestanding -c kernel.c -o ../kernel.o 
i686-elf-ld  -Ttext 0x1000 ../kernel_entry.o ../kernel.o --oformat binary -o ../kernel.bin
