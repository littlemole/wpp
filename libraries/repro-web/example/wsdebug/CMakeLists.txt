############################################
cmake_minimum_required(VERSION 2.8.9)
cmake_policy(SET CMP0054 NEW)
project(reproweb_hello_coro)
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(INSTALL_DEBUG "On")
set(INSTALL_RELEASE "On")

find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

############################################
# clang support
############################################

set(LINKERFLAGS "")
set(STDLIB "")
set(FLAGS "")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(STDLIB "c++abi")
    set(LINKERFLAGS "-stdlib=libc++ -fcoroutines-ts  -std=c++17")
    set(FLAGS "-stdlib=libc++ -fcoroutines-ts -D_RESUMABLE_FUNCTIONS_SUPPORTED")
endif()

set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LINKERFLAGS}" )


############################################
# OS support
############################################


IF (WIN32)

	set(OSLIBS "Ws2_32" "Rpcrt4" "Shlwapi")

	find_library(PRIOCPP_LIBRARY NAMES priocpp)
	find_library(PRIOCPPD_LIBRARY NAMES priocppd)
	find_library(PRIOHTTP_LIBRARY NAMES priohttp)
	find_library(PRIOHTTPD_LIBRARY NAMES priohttpd)
	find_library(NG2HTTP_LIBRARY NAMES nghttp2)
	find_library(LIBEVENT_LIBRARY NAMES event)
    find_library(CRYPTONEAT_LIBRARY NAMES cryptoneat)
    find_library(CRYPTONEATD_LIBRARY NAMES cryptoneatd)
	find_library(JSONCPP_LIBRARY NAMES jsoncpp)
    find_library(CURL_LIBRARY NAMES libcurl)
    find_library(REDIS_LIBRARY NAMES hiredis)
    find_library(SQLITE_LIBRARY NAMES sqlite3)
    find_library(REPROCURL_LIBRARY NAMES reprocurl)
    find_library(REPROCURLD_LIBRARY NAMES reprocurld)
    find_library(REPROREDIS_LIBRARY NAMES reproredis)
    find_library(REPROREDISD_LIBRARY NAMES reproredisd)
    find_library(REPROSQLITE_LIBRARY NAMES reprosqlite)
    find_library(REPROSQLITED_LIBRARY NAMES reprosqlited)
    find_library(REPROWEB_LIBRARY NAMES reproweb)
    find_library(REPROWEBD_LIBRARY NAMES reprowebd)

	set(BUILDFLAGS "-await ${FLAGS}")
	set(DEBUGFLAGS -DMOL_PROMISE_DEBUG)

	set(LIBS ${JSONCPP_LIBRARY} ${GTEST_LIBRARIES} ${OPENSSL_LIBRARIES} ${NG2HTTP_LIBRARY} $ ${SQLITE_LIBRARY} {REDIS_LIBRARY} ${CURL_LIBRARY} ${ZLIB_LIBRARIES} ${STDLIB} ${OSLIBS})
    set(DEBUG_LIBS ${REPROWEBD_LIBRARY} ${PRIOHTTPD_LIBRARY} ${REPROREDISD} ${REPROSQLITED} ${REPROCURLD_LIBRARY} ${PRIOCPPD_LIBRARY} ${CRYPTONEATD_LIBRARY} ${LIBS})
    set(RELEASE_LIBS ${REPROWEB_LIBRARY} ${PRIOHTTP_LIBRARY} ${REPROREDIS} ${REPROSQLITE} ${REPROCURL_LIBRARY} ${PRIOCPP_LIBRARY} ${CRYPTONEAT_LIBRARY} ${LIBS})

 ELSEIF(UNIX)

    set(OSLIBS "pthread")

    find_package(PkgConfig)
    pkg_check_modules(CURL REQUIRED libcurl)
    pkg_check_modules(REDIS REQUIRED hiredis)
    pkg_check_modules(SQLITE REQUIRED sqlite3)
    pkg_check_modules(NGHTTP2 REQUIRED libnghttp2)
    pkg_check_modules(PRIOCPP REQUIRED priocpp)
    pkg_check_modules(PRIOHTTP REQUIRED priohttp)
    pkg_check_modules(REDPROREDIS REQUIRED reproredis)
    pkg_check_modules(REPROSQLITE REQUIRED reprosqlite)
    pkg_check_modules(LIBEVENT REQUIRED libevent_pthreads)
    pkg_check_modules(CRYPTONEAT REQUIRED cryptoneat)
    pkg_check_modules(JSONCPP REQUIRED jsoncpp)

	set(LIBEVENT_LIBRARY ${LIBEVENT_LIBRARIES})

    set(BUILDFLAGS " ${FLAGS} ${JSONCPP_CFLAGS} ${PRIOCPP_CFLAGS}")
	set(DEBUGFLAGS "-g -DMOL_PROMISE_DEBUG")

	set(LIBS ${JSONCPP_LIBRARIES} ${GTEST_LIBRARIES} ${OPENSSL_LIBRARIES} ${NGHTTP2_LIBRARIES} ${CRYPTONEAT_LIBRARIES} ${CURL_LIBRARIES} ${SQLITE_LIBRARIES} ${REDIS_LIBRARIES} ${ZLIB_LIBRARIES} ${STDLIB} )

	set(DEBUG_LIBS reprowebd priohttpd reprocurld reproredisd reprosqlited priocppd ${LIBS} )
	set(RELEASE_LIBS reproweb priohttp reprocurl reproredis reprosqlite priocpp ${LIBS} )

ENDIF ()

############################################
# macros
############################################

macro(AddCompilerFlags target flags)
    get_target_property(CF ${target} COMPILE_FLAGS)
    if(CF STREQUAL "CF-NOTFOUND")
        SET(CF "") # set to empty string
    else()
        SET(CF "${CF} ") # a space to cleanly separate from existing content
    endif()

    SET(CF "${CF} ${flags}" )
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS ${CF} )
endmacro()

############################################
# defaults
############################################

option(WITH_LIBEVENT "use libevent (default:on)" OFF)
if(WITH_LIBEVENT)
    add_definitions(-DPROMISE_USE_LIBEVENT)    
    set(BACKEND ${LIBEVENT_LIBRARY})
else()
    add_definitions(-DPROMISE_USE_BOOST_ASIO)
if (WIN32)
find_package( Boost COMPONENTS system date_time  REQUIRED )
    set(BACKEND "Boost::system" "Boost::date_time")
else()
    set(BACKEND "boost_system")
endif()
endif()


############################################
# sources
############################################

include_directories(/usr/local/include)
include_directories(${OPENSSL_INCLUDE_DIR})

############################################
# executable
############################################

add_executable(hello_world.bin test.cpp)
add_executable(hello_world_debug.bin test.cpp)

AddCompilerFlags(hello_world.bin ${BUILDFLAGS})
AddCompilerFlags(hello_world_debug.bin ${BUILDFLAGS})
AddCompilerFlags(hello_world_debug.bin ${DEBUGFLAGS})

target_link_libraries(hello_world.bin ${RELEASE_LIBS} ${BACKEND} )
target_link_libraries(hello_world_debug.bin ${DEBUG_LIBS} ${BACKEND} )

