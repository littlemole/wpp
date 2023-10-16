# prio
promising reactive io

# doxygen documentation

[doxydocs](https://littlemole.github.io/prio/index.html)

# dependencies

repro header library for promises
openssl for ssl support 

# backend dependencies
using libevent (default) or (experimentai) boost_asio 
as main event loop backend

# features

this library implements basics to use reactive repro promise from an event loop.

implemented basics are
 - timeouts
 - async socket io
 - POSIX signal handling  
 - get multithreaded by threading out with tasks

# api

## timeouts

timeout api declarations with futures:

```cpp
    Future<> timeout(int secs, int ms) noexcept;
    Future<> timeout(int secs) noexcept;

    //usage example using a timeout in 10 seconds
    timeout(10)
    .then([]()
    {
        std::cout << "timeout!" << std::endl;
    });
```

*nextTick* helper to run a timeout as soon as possible:

```cpp
    Future<> nextTick() noexcept;

    //usage example 
    nextTick()
    .then([]()
    {
        std::cout << "now next on the event loop!" << std::endl;
    });
```

threadsafe version that can be called safely from within a prio::task

```cpp

    void timeout( const std::function<void()>& f, int secs, int ms) noexcept;
    void timeout( const  std::function<void()>& f, int secs) noexcept;
    void nextTick( const  std::function<void()>& f) noexcept;

    //usage example 
    nextTick( []()
    {
        std::cout << "now next on the event loop!" << std::endl;
    });
```    

## async socket io

### connect using a client socket

```cpp
    // someone is supposed to manage connection lifetime!
    Connection::Ptr ptr; 
    TcpConnection::connect("localhost", 9876)
    .then([&ptr](Connection::Ptr client)
    {
        ptr = client;
        return client->write("HELO WORLD");
    })
    .then([](Connection::Ptr client)
    {
        return client->read();
    })
    .then([&result,&listener](Connection::Ptr client, std::string data)
    {
        result = data;
        theLoop().exit();
    });

    theLoop().run();
``` 

### server side listening to a socket

```cpp

    // someone is supposed to manage connection lifetime
    // here we support only client, echoing the request payload
    Connection::Ptr c;

    Listener listener;
    listener.bind(9876)
    .then([&c](Connection::Ptr client)
    {
        c = client;
        client->read()
        .then( [](Connection::Ptr client,std::string data)
        {
            return client->write(data);
        })
        .then([](Connection::Ptr client)
        {
            theLoop().exit();
        });
    });
    theLoop().run();
```


# signal handling

```cpp
    signal(SIGINT)
    .then([](int s)
    {
        std::cout << "SIGINT (" << s <<") called" << std::endl;
    });

    ...

    theLoop().run();
```

# treadsafe tasking

```cpp

    int c = 0;

    task([c]()
    {
        // run and compute on separate thread
        return c++;
    })
    .then([&c](int i)
    {
        c = i;
        theLoop().exit();
    });

    theLoop().run();

    // prints 1
    std::cout << c << std::endl;

```

# install

see [repro-web](https://github.com/littlemole/repro-web/) for installing complete library chain and examples.

# install for local development

## native linux
```bash
    git clone https://github.com/littlemole/prio.git
    make 
    make test
    make install
```
self documention makefile, run *make help* for details

## cmake support

building with cmake is supported assuming build happens in a dedicated build directory different from the root src directory.

### default build CXX=g++ and BACKEND=libevent

```bash
    git clone https://github.com/littlemole/prio.git
    cd prio
    mkdir build
    cd build
    cmake ..
    make
    ctest
    sudo make install    
```

### build with different BACKEND=boost_asio

```bash
    git clone https://github.com/littlemole/prio.git
    cd prio
    mkdir build
    cd build
    cmake .. -DWITH_LIBEVENT=Off
    make
    ctest
    sudo make install    
```
### build for CXX=clang++ and default BACKEND

```bash
    git clone https://github.com/littlemole/prio.git
    cd prio
    mkdir build
    cd build
    cmake .. -DCMAKE_CXX_COMPILER="clang++" -DCMAKE_CXX_FLAGS="-std=c++14 -stdlib=libc++"
    make
    ctest
    sudo make install    
```
note this requires not only clang++ toolchain installed,
but also dependent cpp libraries built against clang++ and libc++.
for this project that means gtest and cryptoneat libraries
have to be built using clang++ toolchain.

### build for CXX=clang++ and BACKEND=boost_asio

just add both options from above to calling cmake.


## docker

the makefile has targets to build and test this lib in an
reproducible environment. see Dockerfile for concrete setup.


