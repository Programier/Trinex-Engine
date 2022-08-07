@echo off

if exist "./build" (
	rd /s /q ./build
) 

mkdir build
cd build 
cmake .. -G "MinGW Makefiles"
mingw32-make install -j%NUMBER_OF_PROCESSOR%
pause
