############################################
# preamble
############################################

cmake_minimum_required(VERSION 3.22)

project(hello_standalone)


############################################
# base config
#
# alternatively replace 'find_package("wpp")'
# with the contents wpp-config.cmake for
# customization
############################################

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
file(COPY user.db DESTINATION .)

############################################
# exe
############################################

add_executable(${CMAKE_PROJECT_NAME}.bin ./main.cpp)

############################################
# libs for exe
############################################

target_link_libraries(${CMAKE_PROJECT_NAME}.bin  ${LIBS} ${BACKEND} ${OSLIBS} )

