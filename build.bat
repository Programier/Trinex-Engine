@echo off

if exist "./build" (
	rd /s /q ./build
) 

mkdir build
cd build 
cmake .. -G "MinGW Makefiles"
make -j%NUMBER_OF_PROCESSOR%

copy ..\src\Engine\libs\glew32.dll .\glew32.dll
copy ..\src\Engine\libs\libassimp-5.dll .\libassimp-5.dll
copy ..\src\Engine\libs\glfw.dll .\glfw3.dll
copy ..\src\Engine\libs\freetype.dll .\freetype.dll

copy src\Engine\libGraphics.dll .\libGraphics.dll
copy src\Engine\libImage.dll .\libImage.dll
copy src\Engine\libInit.dll .\libInit.dll
copy src\Engine\libWindow.dll .\libWindow.dll

pause
