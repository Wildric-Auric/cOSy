@echo off
pushd %~dp0..
bochs -debugger -f scr/bochs.config
popd
