############################################
# preamble
############################################

cmake_minimum_required(VERSION 3.22)

project(hello_world)

find_package("wpp")


############################################
# sources
############################################

file(GLOB SOURCES "./*.cpp")

############################################
# runtime config files
############################################

file(COPY pem DESTINATION .)
file(COPY htdocs DESTINATION .)
file(COPY view DESTINATION .)
file(COPY locale DESTINATION .)
file(COPY config.json DESTINATION .)
file(COPY mime.types DESTINATION .)

############################################
# exe
############################################

add_executable(${CMAKE_PROJECT_NAME}.bin ./main.cpp)

if(WIN32)
	file(COPY ${CURL_LIBRARY} DESTINATION ${CMAKE_BINARY_DIR})
endif()

############################################
# libs for exe
############################################

target_link_libraries(${CMAKE_PROJECT_NAME}.bin  ${LIBS} ${BACKEND} ${OSLIBS} )

