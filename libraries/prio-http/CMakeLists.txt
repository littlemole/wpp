# ############################################
# # preamble
# ############################################

project(priohttp)


#set(LIBS priohttp priocpp ${LIBS} )


############################################
# include directories
############################################

include_directories(include)


############################################
# sources
############################################

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/priohttp/*.h")

############################################
# lib target
############################################

add_library(priohttp STATIC ${SOURCES})

############################################
# tests
############################################
if(WITH_TEST)

add_executable(PrioHttpTests t/test.cpp)

############################################
# libs for tests
############################################

target_link_libraries(PrioHttpTests priohttp priocpp ${LIBS} ${BACKEND} ${OSLIBS} )


############################################
# run tests
############################################


add_test(NAME PrioHttp_Tests COMMAND PrioHttpTests)

endif()


file(COPY pem DESTINATION .)
 
############################################
# install rules
############################################

install(TARGETS priohttp DESTINATION lib EXPORT priohttp-targets)

install(FILES ${HEADERS} DESTINATION include/priohttp)

install(FILES priohttp.pc DESTINATION lib/pkgconfig)

install(EXPORT priohttp-targets DESTINATION lib/priohttp-${VERSION})

############################################
# Create and install package configuration 
# and version files.
############################################

configure_file(
   ${priohttp_SOURCE_DIR}/priohttp-config.cmake.in
   ${priohttp_BINARY_DIR}/priohttp-config.cmake @ONLY)

configure_file(
   ${priohttp_SOURCE_DIR}/priohttp-config-version.cmake.in
   ${priohttp_BINARY_DIR}/priohttp-config-version.cmake @ONLY)

install(FILES ${priohttp_BINARY_DIR}/priohttp-config.cmake
         ${priohttp_BINARY_DIR}/priohttp-config-version.cmake
         DESTINATION lib/priohttp-${VERSION})




