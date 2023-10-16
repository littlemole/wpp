############################################
# preamble
############################################

project(repromysql)


############################################
# sources
############################################

file(GLOB SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/${CMAKE_PROJECT_NAME}/*.h")


############################################
# lib target
############################################

add_library(repromysql  STATIC ${SOURCES})


############################################
# tests
############################################

#add_executable(MysqlTests t/test.cpp)


############################################
# libs for tests
############################################

#target_link_libraries(MysqlTests ${LIBS} ${BACKEND} ${OSLIBS} )


############################################
# run tests
############################################

#add_test(NAME AllTests COMMAND MysqlTests)

 
############################################
# install rules
############################################

install(TARGETS repromysql DESTINATION lib EXPORT repromysql-targets)
install(DIRECTORY "include/repromysql/" DESTINATION "include/repromysql/" )
install(FILES repromysql.pc DESTINATION lib/pkgconfig)
install(EXPORT repromysql-targets DESTINATION lib/repromysql-${VERSION})


############################################
# Create and install package configuration 
# and version files.
############################################

configure_file(
   ${repromysql_SOURCE_DIR}/repromysql-config.cmake.in
   ${repromysql_BINARY_DIR}/repromysql-config.cmake @ONLY)

configure_file(
   ${repromysql_SOURCE_DIR}/repromysql-config-version.cmake.in
   ${repromysql_BINARY_DIR}/repromysql-config-version.cmake @ONLY)

install(FILES ${repromysql_BINARY_DIR}/repromysql-config.cmake
         ${repromysql_BINARY_DIR}/repromysql-config-version.cmake
         DESTINATION lib/repromysql-${VERSION})


