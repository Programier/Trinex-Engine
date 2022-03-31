@echo off

if exist "./build" (
	rd /s /q ./build
) 

mkdir build
cd build 
cmake .. -G "MinGW Makefiles"
mingw32-make -j%NUMBER_OF_PROCESSOR%

for /r "..\src\Engine\" %%x in (*.dll) do copy "%%x" ".\"
for /r "src\Engine\" %%x in (*.dll) do copy "%%x" ".\"

pause
