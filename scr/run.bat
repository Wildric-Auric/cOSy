@echo off
pushd %~dp0..
cat build/btl.bin build/kernel.bin > build/osimg.bin
rem start cmd /c "qemu -fda build/osimg.bin   -device ide-hd,drive=disk0   -drive file=disk.img,format=raw,id=disk0    -boot c"
rem start cmd "qemu -fda build/osimg.bin -device piix4-ide,id=ide -drive id=build/disk0,file=build/disk0.img,format=raw,if=none -device ide-hd,drive=disk0,bus=ide.0 -boot c"

start cmd /c "qemu -device ahci,id=ahci0 -drive if=none,file=build/drive.img,format=raw,id=disk0 -device ide-hd,drive=disk0,bus=ahci0.0 -fda build/osimg.bin -boot c"

popd
