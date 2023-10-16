# ############################################
# # preamble
# ############################################

project(reprocurl)



############################################
# sources
############################################

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/reprocurl/*.h")

############################################
# lib target
############################################

add_library(reprocurl  STATIC ${SOURCES})

############################################
# tests
############################################

if(WITH_TEST)

add_executable(Tests t/test.cpp)
add_executable(Api t/api.cpp)

IF (WIN32)
	set_source_files_properties(t/test.cpp PROPERTIES COMPILE_FLAGS /Ob0)
ENDIF()

############################################
# libs for tests
############################################

target_link_libraries(Tests  reprocurl priocpp ${LIBS} ${BACKEND} ${OSLIBS} )
target_link_libraries(Api reprocurl priocpp ${LIBS} ${BACKEND} ${OSLIBS} )


############################################
# run tests
############################################


add_test(NAME AllTests COMMAND Tests)
add_test(NAME ApiTests COMMAND Api)

endif()


file(COPY pem DESTINATION .)
 
############################################
# install rules
############################################

install(TARGETS reprocurl DESTINATION lib EXPORT reprocurl-targets)

install(FILES ${HEADERS} DESTINATION include/reprocurl)

install(FILES reprocurl.pc DESTINATION lib/pkgconfig)

install(EXPORT reprocurl-targets DESTINATION lib/reprocurl-${VERSION})

############################################
# Create and install package configuration 
# and version files.
############################################

configure_file(
   ${reprocurl_SOURCE_DIR}/reprocurl-config.cmake.in
   ${reprocurl_BINARY_DIR}/reprocurl-config.cmake @ONLY)

configure_file(
   ${reprocurl_SOURCE_DIR}/reprocurl-config-version.cmake.in
   ${reprocurl_BINARY_DIR}/reprocurl-config-version.cmake @ONLY)

install(FILES ${reprocurl_BINARY_DIR}/reprocurl-config.cmake
         ${reprocurl_BINARY_DIR}/reprocurl-config-version.cmake
         DESTINATION lib/reprocurl-${VERSION})




