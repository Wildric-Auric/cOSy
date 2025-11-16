@echo off
pushd %~dp0

if not exist "../build/drive.img" (
    fsutil file createnew ../build/drive.img 65536 
)

call ./boot.bat
call ./krn.bat
call ./run.bat
popd
