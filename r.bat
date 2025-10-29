cat btl.bin kernel.bin > osimg.bin
qemu -fda osimg.bin -boot c
