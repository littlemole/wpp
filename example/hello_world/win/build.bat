

cd out\build\x64-Debug
rem cmake -G Ninja -DWITH_LIBEVENT=%USE_LIBEVENT% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=%~dp0\..\..\..\vcpkg\scripts\buildsystems\vcpkg.cmake ../../..
cmake ../../.. --preset "win-x64-asio-debug"
cmake --build .

cd ..

cd x64-Release
rem cmake -G Ninja -DWITH_LIBEVENT=%USE_LIBEVENT% -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%~dp0\..\..\..\vcpkg\scripts\buildsystems\vcpkg.cmake ../../..
cmake ../../.. --preset "win-x64-asio-release"
cmake --build .

cd ..
cd ..
cd ..


