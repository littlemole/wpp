# ############################################
# # preamble
# ############################################

project(reproweb)


############################################
# include directories
############################################

include_directories(include)


############################################
# sources
############################################

file(GLOB SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/reproweb*.h")


############################################
# lib target
############################################

add_library(reproweb  STATIC ${SOURCES})


############################################
# tests
############################################

if(WITH_TEST)

add_executable(ReproWebTests t/test.cpp)


############################################
# libs for tests
############################################

target_link_libraries(ReproWebTests reproweb reprocurl reproredis reprosqlite priohttp priocpp ${LIBS} ${BACKEND} ${OSLIBS} )


############################################
# run tests
############################################


add_test(NAME ReproWeb_Tests COMMAND ReproWebTests)

endif()

file(COPY pem DESTINATION .)
file(COPY htdocs DESTINATION .)
file(COPY locale DESTINATION .)
file(COPY config.json DESTINATION .)

 
############################################
# install rules
############################################

install(TARGETS reproweb DESTINATION lib EXPORT reproweb-targets)
install(DIRECTORY "include/reproweb/" DESTINATION "include/reproweb/" )
install(FILES reproweb.pc DESTINATION lib/pkgconfig)
install(EXPORT reproweb-targets DESTINATION lib/reproweb-${VERSION})


############################################
# Create and install package configuration 
# and version files.
############################################

configure_file(
   ${reproweb_SOURCE_DIR}/reproweb-config.cmake.in
   ${reproweb_BINARY_DIR}/reproweb-config.cmake @ONLY)

configure_file(
   ${reproweb_SOURCE_DIR}/reproweb-config-version.cmake.in
   ${reproweb_BINARY_DIR}/reproweb-config-version.cmake @ONLY)

install(FILES ${reproweb_BINARY_DIR}/reproweb-config.cmake
         ${reproweb_BINARY_DIR}/reproweb-config-version.cmake
         DESTINATION lib/reproweb-${VERSION})




