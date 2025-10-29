nasm -f bin -l ../debug/btl.lst btl.asm -o ../btl.bin
nasm  kernel_entry.asm -l ../debug/kernel_entry.lst -f elf -o ../kernel_entry.o
