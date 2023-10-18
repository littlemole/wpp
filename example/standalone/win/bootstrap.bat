@echo off
if not exist vcpkg (
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
cmd /C bootstrap-vcpkg.bat
cd ..
)




rem if not exist "out" mkdir out
rem cd out
rem if not exist "build" mkdir build
rem cd build
rem if not exist "x64-Debug" mkdir "x64-Debug"
rem if not exist "x64-Release" mkdir "x64-Release"
rem cd ..
rem cd ..


rem set CMAKE_TOOLCHAIN_FILE=%~dp0vcpkg\scripts\buildsystems\vcpkg.cmake
rem echo CMAKE_TOOLCHAIN_FILE=%CMAKE_TOOLCHAIN_FILE%

rem cmd /C win\build.bat

