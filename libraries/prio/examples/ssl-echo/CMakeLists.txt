############################################
cmake_minimum_required(VERSION 2.8.9)
project(ssl-echo)
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

############################################
# OS support
############################################


IF (WIN32)

	set(OSLIBS "Ws2_32" "Rpcrt4")

    find_package(gtest REQUIRED)
    find_package(openssl REQUIRED)
	find_library(CRYPTONEAT_LIBRARY NAMES cryptoneat)
	find_library(NG2HTTP_LIBRARY NAMES nghttp2)
	find_library(LIBEVENT_LIBRARY NAMES event)
	find_library(PRIO_LIBRARY NAMES priocpp)

    set(BUILD_INCLUDE_DIR ${OPENSSL_INCLUDE_DIRS})	
    set(BUILDFLAGS "-await")
	set(DEBUGFLAGS "-DMOL_PROMISE_DEBUG")

	set(LIBS ${PRIO_LIBRARY} ${CRYPTONEAT_LIBRARY} ${GTEST_LIBRARIES} ${OPENSSL_LIBRARIES} ${NG2HTTP_LIBRARY}  )

 ELSEIF(UNIX)

    set(OSLIBS "pthread")

    find_package(PkgConfig)
    pkg_check_modules(GTEST REQUIRED gtest)
    pkg_check_modules(OPENSSL REQUIRED openssl)
    pkg_check_modules(CRYPTONEAT REQUIRED cryptoneat)
    pkg_check_modules(NGHTTP2 REQUIRED libnghttp2)
    pkg_check_modules(LIBEVENT REQUIRED libevent_pthreads)
    pkg_check_modules(PRIOCPP REQUIRED priocpp)

	set(LIBEVENT_LIBRARY ${LIBEVENT_LIBRARIES})

    set(BUILD_INCLUDE_DIR ${OPENSSL_INCLUDE_DIRS} ${PRIOCPP_INCLUDE_DIRS})
    set(BUILDFLAGS "${GTEST_CFLAGS_OTHER}")
	set(DEBUGFLAGS "-g -DMOL_PROMISE_DEBUG")

	set(LIBS ${CRYPTONEAT_LIBRARIES} ${GTEST_LIBRARIES} ${OPENSSL_LIBRARIES} ${NGHTTP2_LIBRARIES}  )

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

include_directories(${BUILD_INCLUDE_DIR})

############################################
# targets
############################################

add_executable(ssl-echo.bin ssl-echo.cpp)
add_executable(ssl-echod.bin ssl-echo.cpp)

AddCompilerFlags(ssl-echo.bin ${BUILDFLAGS})

AddCompilerFlags(ssl-echod.bin ${BUILDFLAGS})
AddCompilerFlags(ssl-echod.bin ${DEBUGFLAGS})

############################################
# libs
############################################

target_link_libraries(ssl-echo.bin priocpp ${LIBS} ${BACKEND} ${OSLIBS} )
target_link_libraries(ssl-echod.bin priocppd ${LIBS} ${BACKEND} ${OSLIBS} )

#file(COPY pem DESTINATION .)
 
