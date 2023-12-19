
set(VERSION "$ENV{VERSION}")

if(UNIX)
    option(WITH_LIBEVENT "use libevent (default:on)" On)
else()
    option(WITH_LIBEVENT "use libevent (default:off)" Off)
endif()

############################################
# c++ std
############################################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)



############################################
# basic dependencies
############################################

find_package(OpenSSL REQUIRED)
find_package(expat CONFIG REQUIRED)

find_package(cryptoneat CONFIG REQUIRED)
find_package(diycpp REQUIRED)
find_package(patex REQUIRED)
find_package(metacpp REQUIRED)

find_package(priocpp CONFIG REQUIRED)
find_package(priohttp CONFIG REQUIRED)

find_package(reprocurl CONFIG REQUIRED)
find_package(reproredis CONFIG REQUIRED)
find_package(reprosqlite CONFIG REQUIRED)
find_package(repromysql REQUIRED)

find_package(reproweb CONFIG REQUIRED)


############################################
# basic compile flags
############################################

set(LINKERFLAGS "")
set(STDLIB "")

IF(WIN32)
    set(FLAGS "/W3")
else()
    set(FLAGS "-Wall -Wextra")
endif()


############################################
# clang support
############################################

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(STDLIB "c++abi")
    set(LINKERFLAGS "-stdlib=libc++ -fcoroutines-ts ")
    set(FLAGS "-Wall -Wextra -stdlib=libc++ -fcoroutines-ts")
endif()


############################################
# OS support (Linux/Windows)
############################################

IF (WIN32)

    if(NOT CMAKE_BUILD_TYPE)
	    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
    endif()

    set(VERSION "${CMAKE_CXX_COMPILER_ID}_${CMAKE_BUILD_TYPE}")

    set(OSLIBS "Ws2_32" "Rpcrt4" "Shlwapi")

    find_library(EXPAT_LIBRARY NAMES libexpat.lib)
    find_library(EXPATD_LIBRARY NAMES libexpatd.lib)
    find_library(NGHTTP2_LIBRARY NAMES nghttp2)

    find_package(jsoncpp REQUIRED)
    find_package(unofficial-sqlite3 CONFIG REQUIRED)
    find_package(Iconv)
    find_package(CURL REQUIRED)
    find_package(unofficial-libmysql CONFIG REQUIRED)
    find_package(ZLIB REQUIRED)

    if(WITH_LIBEVENT)
        add_definitions(-DPROMISE_USE_LIBEVENT)    

        if(CMAKE_BUILD_TYPE MATCHES "Debug")
           find_library(LIBEVENT_LIBRARY NAMES "eventd.lib")
        else()
            find_library(LIBEVENT_LIBRARY NAMES "event.lib")
        endif()

        set(BACKEND "${LIBEVENT_LIBRARY}") 
    else()
        add_definitions(-DPROMISE_USE_BOOST_ASIO)
        find_package( Boost COMPONENTS system date_time  REQUIRED )
        set(BACKEND "Boost::system" "Boost::date_time")
    endif()

    find_path(MUSTACHE_INCLUDE_DIRS "mustache.hpp")
    get_target_property(JSONCPP_INCLUDE_DIR jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)


    set(LIBS 
	    reproweb reprocurl repromysql reprosqlite reproredis
    	priohttp priocpp
	    cryptoneat patex  
	    JsonCpp::JsonCpp
	    CURL::libcurl
	    ${MYSQL_LIBRARIES}
	    unofficial::sqlite3::sqlite3
	    ${NGHTTP2_LIBRARY} 
	    ${OPENSSL_LIBRARIES} 
	    expat::expat
	    Iconv::Iconv
	    ZLIB::ZLIB
	    ${STDLIB}	
    )

 ELSEIF (UNIX)

    set(OSLIBS "pthread")

    find_package(PkgConfig)
    pkg_check_modules(CURL REQUIRED libcurl)
    pkg_check_modules(NGHTTP2 REQUIRED libnghttp2)
    pkg_check_modules(LIBEVENT REQUIRED libevent_pthreads)
    pkg_check_modules(SQLITE REQUIRED sqlite3)
    pkg_check_modules(MYSQL  mysqlclient)
    if( NOT MYSQL )
        pkg_check_modules(MYSQL  mariadb)
    endif()

    set(LIBEVENT_LIBRARY ${LIBEVENT_LIBRARIES})

    if(WITH_LIBEVENT)
        add_definitions(-DPROMISE_USE_LIBEVENT)    
        set(BACKEND ${LIBEVENT_LIBRARIES})
    else()
        add_definitions(-DPROMISE_USE_BOOST_ASIO)
        set(BACKEND "boost_system")
    endif()

    set(LIBS
    	reproweb reprocurl repromysql reprosqlite reproredis
    	priohttp priocpp
    	cryptoneat patex 
    	jsoncpp_static expat 
	    ${OPENSSL_LIBRARIES} ${NGHTTP2_LIBRARIES} 
	    ${CURL_LIBRARIES} ${SQLITE_LIBRARIES} 
	    ${MYSQL_LIBRARIES}
	    ${STDLIB} 
	    z
    )

ENDIF ()



############################################
# set linker and compiler flags
############################################

set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LINKERFLAGS}" )
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -DMOL_PROMISE_DEBUG")
set(CMAKE_CXX_FLAGS  	    "${CMAKE_CXX_FLAGS} ${FLAGS} -DVERSION=${VERSION}")

############################################
# jsoncpp
############################################
if(UNIX)

    include(FetchContent)

    set(JSONCPP_LIB_BUILD_STATIC ON)
    set(BUILD_STATIC_LIBS On)
    set(BUILD_SHARED_LIBS Off)
    set(JSONCPP_WITH_TESTS Off) 

    FetchContent_Declare(
        jsoncpp
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/open-source-parsers/jsoncpp/archive/69098a18b9af0c47549d9a271c054d13ca92b006.zip
    )

    FetchContent_GetProperties(jsoncpp)
    if(NOT jsoncpp_POPULATED)
        FetchContent_Populate(jsoncpp)
        add_subdirectory(${jsoncpp_SOURCE_DIR} ${jsoncpp_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
    
    get_target_property(JSONCPP_INCLUDE_DIR jsoncpp_static INTERFACE_INCLUDE_DIRECTORIES)
    

endif()


message("+++++++++++++++++")
message("OSLIBS: ${OSLIBS}")
message("LIBS: ${LIBS}")
message("LINKER: ${CMAKE_EXE_LINKER_FLAGS}" )
message("DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message("FLAGS: ${CMAKE_CXX_FLAGS}" )
message("BACKEND: ${BACKEND}")
message("+++++++++++++++++")


############################################
# include directories
############################################

include_directories(${JSONCPP_INCLUDE_DIR})

if(UNIX)
    include_directories("/usr/include/kainjow")
endif()

