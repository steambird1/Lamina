:: This script is given in order to speed up build process.
:: by steambird

@echo off
mkdir build
if "%%1"=="clean" (
	del /f /s /q .\build\*
	rd /s /q .\build\*
)
pushd build
cmake .. -G "MinGW Makefiles"
mingw32-make
popd
pause