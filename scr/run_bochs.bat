@echo off
pushd %~dp0..

rem Check if the first argument is -d or --d
if /I "%1"=="-d" (
    bochs -debugger -f scr/bochs.config
) else if /I "%1"=="--debug" (
    bochs -debugger -f scr/bochs.config
) else (
    bochs -f scr/bochs.config
)

popd
