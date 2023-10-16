@echo off
cd ..
cd ..
if not exist vcpkg (
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
cmd /C bootstrap-vcpkg.bat
cd ..
)
cd example
cd hello_world


if not exist "out" mkdir out
cd out
if not exist "build" mkdir build
cd build
if not exist "x64-Debug" mkdir "x64-Debug"
if not exist "x64-Release" mkdir "x64-Release"
cd ..
cd ..

rem set CMAKE_TOOLCHAIN_FILE=%~dp0..\..\vcpkg\scripts\buildsystems\vcpkg.cmake
rem echo CMAKE_TOOLCHAIN_FILE=%CMAKE_TOOLCHAIN_FILE%

rem cmd /C win\build.bat

