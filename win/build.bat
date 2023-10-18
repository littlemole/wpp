

cmake . --preset "win-x64-asio-debug"
cmake --build  --preset "win-x64-asio-debug" --target install

cmake  --preset "win-x64-asio-release"
cmake --build  --preset "win-x64-asio-release" --target install

