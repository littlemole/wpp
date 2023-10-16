# ############################################
# # preamble
# ############################################

project(priocpp)

#include_directories(${OPENSSL_INCLUDE_DIR} ${GTEST_INCLUDE_DIR} ) #${CRYPTONEAT_INCLUDE_DIR} )


############################################
# sources
############################################

file(GLOB SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/priocpp/*.h")

############################################
# lib target
############################################

add_library(priocpp  STATIC ${SOURCES})


############################################
# tests
############################################

if(WITH_TEST)

add_executable(SimpleTest t/test.cpp)
add_executable(HttpTest t/http.cpp)

############################################
# libs for tests
############################################

target_link_libraries(SimpleTest priocpp ${LIBS} ${BACKEND} ${OSLIBS} )
target_link_libraries(HttpTest priocpp ${LIBS} ${BACKEND} ${OSLIBS} )

############################################
# run tests
############################################

add_test(NAME Simple_Test COMMAND SimpleTest)
#add_test(NAME Http_Test COMMAND HttpTest)

add_test(NAME Http_Test COMMAND HttpTest WORKING_DIRECTORY ${UNIT_TEST_BIN_OUTPUT_DIR})

endif()

file(COPY pem DESTINATION .)
 
############################################
# install rules
############################################
 
install(TARGETS priocpp DESTINATION lib EXPORT priocpp-targets)

install(DIRECTORY include/priocpp DESTINATION include)

install(FILES priocpp.pc DESTINATION lib/pkgconfig)

install(EXPORT priocpp-targets DESTINATION lib/priocpp-${VERSION})

############################################
# Create and install package configuration 
# and version files.
############################################

configure_file(
   ${priocpp_SOURCE_DIR}/priocpp-config.cmake.in
   ${priocpp_BINARY_DIR}/priocpp-config.cmake @ONLY)

configure_file(
   ${priocpp_SOURCE_DIR}/priocpp-config-version.cmake.in
   ${priocpp_BINARY_DIR}/priocpp-config-version.cmake @ONLY)

install(FILES ${priocpp_BINARY_DIR}/priocpp-config.cmake
         ${priocpp_BINARY_DIR}/priocpp-config-version.cmake
         DESTINATION lib/priocpp-${VERSION})



