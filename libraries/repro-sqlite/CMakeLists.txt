
project(reprosqlite)

# ############################################
# # sources
# ############################################

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/reprosqlite/*.h")


############################################
# targets
############################################

add_library(reprosqlite  STATIC ${SOURCES})

if(WITH_TEST)

add_executable(SqliteTests t/test.cpp)


############################################
# libs
############################################

target_link_libraries(SqliteTests reprosqlite priocpp ${LIBS} ${BACKEND} ${OSLIBS} )


############################################
# tests
############################################

add_test(NAME Sqlite_Tests COMMAND SqliteTests)

endif()

file(COPY test.db DESTINATION .)


############################################
# install rules
############################################
 
install(TARGETS reprosqlite DESTINATION lib EXPORT reprosqlite-targets)
install(FILES ${HEADERS} DESTINATION include/reprosqlite)
install(FILES reprosqlite.pc DESTINATION lib/pkgconfig)
install(EXPORT reprosqlite-targets DESTINATION lib/reprosqlite-${VERSION})


############################################
# Create and install package configuration 
# and version files.
############################################

configure_file(
   ${reprosqlite_SOURCE_DIR}/reprosqlite-config.cmake.in
   ${reprosqlite_BINARY_DIR}/reprosqlite-config.cmake @ONLY)

configure_file(
   ${reprosqlite_SOURCE_DIR}/reprosqlite-config-version.cmake.in
   ${reprosqlite_BINARY_DIR}/reprosqlite-config-version.cmake @ONLY)

install(FILES ${reprosqlite_BINARY_DIR}/reprosqlite-config.cmake
         ${reprosqlite_BINARY_DIR}/reprosqlite-config-version.cmake
         DESTINATION lib/reprosqlite-${VERSION})

