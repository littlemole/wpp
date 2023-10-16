

all:
	g++ main.cpp -std=c++20 -I/usr/include/kainjow -D_RESUMABLE_FUNCTIONS_SUPPORTED $(shell pkg-config --libs --cflags reproweb reprocurl jsoncpp libnghttp2  openssl zlib libevent_pthreads libcurl)

clean:
	-rm a.out

run:
	./a.out
	
	