############################################
# preamble
############################################

cmake_minimum_required(VERSION 3.22)

project(wpp)
set(VERSION 0.0.1)

include(CTest)

if(UNIX)
    option(WITH_LIBEVENT "use libevent (default:on)" ON)
else()
    option(WITH_LIBEVENT "use libevent (default:off)" Off)
endif()
option(WITH_TEST "run tests (default:on)" ON)

############################################
# c++ std
############################################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


############################################
# debug mode produces libs with d suffix
############################################

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX d)
endif()


############################################
# basic dependecies
############################################

find_package(OpenSSL REQUIRED)
find_package(expat CONFIG REQUIRED)
find_package(Iconv)

find_package(cryptoneat REQUIRED)
find_package(diycpp REQUIRED)
find_package(patex REQUIRED)
find_package(metacpp REQUIRED)


############################################
# basic compile flags
############################################

set(LINKERFLAGS "")
set(STDLIB "")

IF (WIN32)
    set(FLAGS "/W3")
else()
    set(FLAGS "-Wall -Wextra")
endif()

############################################
# clang support
############################################

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(STDLIB "c++abi")
    set(LINKERFLAGS "-stdlib=libc++ -fcoroutines-ts")
    set(FLAGS "-Wall -Wextra -stdlib=libc++ -fcoroutines-ts ")
endif()


############################################
# OS support (Linux/Windows)
############################################

IF (WIN32)

    set(OSLIBS "Ws2_32" "Rpcrt4" "Shlwapi")

    find_package(GTest REQUIRED)
    find_package(jsoncpp CONFIG REQUIRED)
    find_package(unofficial-sqlite3 CONFIG REQUIRED)
    find_package(Iconv)
    find_package(CURL REQUIRED)
    find_package(libmysql REQUIRED)
    find_package(unofficial-sqlite3 CONFIG REQUIRED)
    find_package(ZLIB REQUIRED)

    find_library(EXPAT_LIBRARY NAMES libexpat.lib)
    find_library(EXPATD_LIBRARY NAMES libexpatd.lib)
    find_library(NGHTTP2_LIBRARY NAMES nghttp2.lib)

    if(WITH_LIBEVENT)
        add_definitions(-DPROMISE_USE_LIBEVENT)    
    	find_package(Libevent CONFIG REQUIRED)
        set(BACKEND "libevent::core" "libevent::extra")
    else()
        add_definitions(-DPROMISE_USE_BOOST_ASIO)
        find_package( Boost COMPONENTS system date_time  REQUIRED )
        set(BACKEND "Boost::system" "Boost::date_time")
    endif()

    find_path(MUSTACHE_INCLUDE_DIRS "mustache.hpp")
    get_target_property(JSONCPP_INCLUDE_DIR jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)

    set(LIBS 
	    cryptoneat patex  
	    GTest::gtest JsonCpp::JsonCpp
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

    add_definitions("-D_WIN32_WINNT=0x0A00")       

ELSEIF (UNIX)

    set(OSLIBS "pthread")

    find_package(PkgConfig)
    pkg_check_modules(LIBEVENT REQUIRED libevent_pthreads)
    pkg_check_modules(CURL REQUIRED libcurl)
    pkg_check_modules(NGHTTP2 REQUIRED libnghttp2)
    pkg_check_modules(SQLITE REQUIRED sqlite3)
    pkg_check_modules(MYSQL  mysqlclient)
    if( !MYSQL_FOUND )
        pkg_check_modules(MYSQL  mariadb)
    endif()
    

    if(WITH_LIBEVENT)
        add_definitions(-DPROMISE_USE_LIBEVENT)    
        set(BACKEND ${LIBEVENT_LIBRARIES})
    else()
        add_definitions(-DPROMISE_USE_BOOST_ASIO)
        set(BACKEND "boost_system")
    endif()

    set(LIBS 
        cryptoneat patex  
	    expat ${OPENSSL_LIBRARIES} ${NGHTTP2_LIBRARIES} 
	    ${CURL_LIBRARIES} ${SQLITE_LIBRARIES} 
	    Iconv::Iconv
        "gtest" "jsoncpp_static"
	    z ${STDLIB}
    )

   set( MUSTACHE_INCLUDE_DIRS "/usr/include/kainjow")

ENDIF ()

 

###########################################
# set linker and compiler flags
############################################

set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LINKERFLAGS}" )
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -DMOL_PROMISE_DEBUG")
set(CMAKE_CXX_FLAGS  	    "${CMAKE_CXX_FLAGS} ${FLAGS}")

############################################
# gtest & jsoncpp
############################################
include(FetchContent)

if(UNIX AND WITH_TEST)

    # gtest
    FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/f8d7d77c06936315286eb55f8de22cd23c188571.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    #FetchContent_MakeAvailable(googletest)

    FetchContent_GetProperties(googletest)
    if(NOT googletest_POPULATED)
        FetchContent_Populate(googletest)
        add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
endif()

if(UNIX)
    # jsoncpp

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


############################################
# INCLUDE DIRS
############################################


include_directories(${JSONCPP_INCLUDE_DIR})
include_directories(${MUSTACHE_INCLUDE_DIRS})

include_directories(libraries/prio/include)
include_directories(libraries/prio-http/include)

include_directories(libraries/repro-curl/include)
include_directories(libraries/repro-sqlite/include)
include_directories(libraries/repro-redis/include)
include_directories(libraries/repro-mysql/include)

############################################
# Add sub-directories
############################################

add_subdirectory (libraries/prio) 
add_subdirectory (libraries/prio-http) 

add_subdirectory (libraries/repro-curl) 
add_subdirectory (libraries/repro-sqlite) 
add_subdirectory (libraries/repro-redis) 
add_subdirectory (libraries/repro-mysql) 

add_subdirectory (libraries/repro-web) 

############################################
# Inspect config
############################################

message("BLDTYPE : ${CMAKE_BUILD_TYPE}")
message("CXXFLAGS: ${CMAKE_CXX_FLAGS}")
message("DBGFLAGS: ${CMAKE_CXX_FLAGS_DEBUG}")
message("LNKFLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message("LIBS    : ${LIBS}")
message("OSLIBS  : ${OSLIBS}")
message("INCDIRS : ${INCLUDE_DIRECTORIES}")

############################################
# install wpp module with basic config 
# used by examples
############################################

install(FILES wpp-config.cmake DESTINATION lib/wpp-${VERSION})
install(FILES wpp_examples-config.cmake DESTINATION lib/wpp_examples-${VERSION})

############################################
# packaging
############################################

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
include(package)

