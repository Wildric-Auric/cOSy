@echo off
pushd %~dp0
call ./boot.bat
call ./krn.bat
call ./run.bat
popd
