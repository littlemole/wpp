############################################
# preamble
############################################

cmake_minimum_required(VERSION 3.22)

project(hello_soa_sessionservice)

find_package("wpp")


############################################
# sources
############################################

file(GLOB SOURCES "./*.cpp")

############################################
# runtime config files
############################################

file(COPY pem DESTINATION .)
file(COPY config.json DESTINATION .)

############################################
# exe
############################################

add_executable(${CMAKE_PROJECT_NAME}.bin ./main.cpp)

############################################
# libs for exe
############################################

target_link_libraries(${CMAKE_PROJECT_NAME}.bin ${LIBS} ${BACKEND} ${OSLIBS} )

