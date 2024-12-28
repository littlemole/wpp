############################################
# preamble
############################################
cmake_minimum_required(VERSION 2.8.9)
cmake_policy(SET CMP0054 NEW)
project(fetchurl)

############################################
# c++ std
############################################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

############################################
# basic dependecies
############################################

find_package(OpenSSL REQUIRED)


############################################
# clang support
############################################

set(LINKERFLAGS "")
set(STDLIB "")
set(FLAGS "")


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(STDLIB "c++abi")
    set(LINKERFLAGS "-stdlib=libc++ -std=c++20")
    set(FLAGS "-stdlib=libc++" )
endif()



############################################
# OS support (Linux/Windows)
############################################

IF (WIN32)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

message("+++++++++++++++++")
message("${CMAKE_BUILD_TYPE}")
message("+++++++++++++++++")

	set(OSLIBS "Ws2_32" "Rpcrt4")
	set(BUILDFLAGS "-await ${FLAGS}")

	find_library(LIBEVENT_LIBRARY NAMES event)
	find_library(CURL_LIBRARY NAMES libcurl)
	find_library(NG2HTTP_LIBRARY NAMES nghttp2)
	set(LIBS ${NG2HTTP_LIBRARY} ${CURL_LIBRARY}  ${OPENSSL_LIBRARIES} ${STDLIB}  )

	if(${CMAKE_BUILD_TYPE} MATCHES "Release")
			find_library(PRIOCPP_LIBRARY NAMES priocpp)
        	find_library(REPROCURL_LIBRARY NAMES reprocurl)
			set(LIBS ${LIBS} ${PRIOCPP_LIBRARY} ${REPROCURL_LIBRARY})
    else()
    	find_library(PRIOCPP_LIBRARY NAMES priocpp.lib)
    	find_library(PRIOCPPD_LIBRARY NAMES priocppd.lib)
	    find_library(REPROCURL_LIBRARY NAMES reprocurl)
	    find_library(REPROCURLD_LIBRARY NAMES reprocurld)

		set(LIBS ${LIBS} debug ${PRIOCPPD_LIBRARY} optimized  ${PRIOCPP_LIBRARY} debug ${REPROCURLD_LIBRARY} optimized ${REPROCURL_LIBRARY} )
	endif()


 ELSEIF (UNIX)

    set(OSLIBS "pthread")

    find_package(PkgConfig)
    pkg_check_modules(CURL REQUIRED libcurl)
    pkg_check_modules(NGHTTP2 REQUIRED libnghttp2)
    pkg_check_modules(PRIOCPP REQUIRED priocpp)
    pkg_check_modules(LIBEVENT REQUIRED libevent_pthreads)

    set(LIBEVENT_LIBRARY ${LIBEVENT_LIBRARIES})

    set(BUILDFLAGS " ${FLAGS}")

    set(LIBS debug reprocurld optimized reprocurl debug priocpp optimized priocpp  ${OPENSSL_LIBRARIES} ${NGHTTP2_LIBRARY} ${CURL_LIBRARIES} ${STDLIB}  )

ENDIF ()

message("+++++++++++++++++")
message("LIBS: ${LIBS}")
message("+++++++++++++++++")


############################################
# set linker and compiler flags
############################################

set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LINKERFLAGS}" )
set(CMAKE_CXX_FLAGS_DEBUG  	"${CMAKE_CXX_FLAGS_DEBUG} -DMOL_PROMISE_DEBUG")
set(CMAKE_CXX_FLAGS  		"${CMAKE_CXX_FLAGS} ${BUILDFLAGS}")


############################################
# backend: libevent or boost_asio
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
# include directories
############################################

include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${PRIOCPP_INCLUDE_DIRS})


############################################
# sources
############################################

file(GLOB SOURCES "./*.cpp")


############################################
# exe
############################################

add_executable(FetchUrl test.cpp)

############################################
# libs for exe
############################################

target_link_libraries(FetchUrl ${LIBS} ${BACKEND} ${OSLIBS} )

