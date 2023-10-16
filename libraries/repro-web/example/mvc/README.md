compile with

g++ main.cpp -std=c++1$(pkg-config --libs --cflags reproweb reprocurl jsoncpp libnghttp2  openssl zlib libevent_pthreads libcurl)
