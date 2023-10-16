# ############################################
# # preamble
# ############################################

project(reproredis)



############################################
# include directories
############################################

include_directories(include)


############################################
# sources
############################################

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/reproredis/*.h")


############################################
# lib target
############################################

add_library(reproredis STATIC ${SOURCES})


############################################
# tests
############################################
if(WITH_TEST)

add_executable(RedisTests t/test.cpp)


############################################
# libs for tests
############################################

target_link_libraries(RedisTests  reproredis priocpp  ${LIBS} ${BACKEND} ${OSLIBS} )


############################################
# run tests
############################################

if(WIN32)
	message("skip tests on win32")
else()
	add_test(NAME AllTests COMMAND Tests)
endif()
 
endif()
 
############################################
# install rules
############################################
 
install(TARGETS reproredis DESTINATION lib EXPORT reproredis-targets)
install(FILES ${HEADERS} DESTINATION include/reproredis)
install(FILES reproredis.pc DESTINATION lib/pkgconfig)
install(EXPORT reproredis-targets DESTINATION lib/reproredis-${VERSION})


############################################
# Create and install package configuration 
# and version files.
############################################

configure_file(
   ${reproredis_SOURCE_DIR}/reproredis-config.cmake.in
   ${reproredis_BINARY_DIR}/reproredis-config.cmake @ONLY)

configure_file(
   ${reproredis_SOURCE_DIR}/reproredis-config-version.cmake.in
   ${reproredis_BINARY_DIR}/reproredis-config-version.cmake @ONLY)

install(FILES ${reproredis_BINARY_DIR}/reproredis-config.cmake
         ${reproredis_BINARY_DIR}/reproredis-config-version.cmake
         DESTINATION lib/reproredis-${VERSION})



