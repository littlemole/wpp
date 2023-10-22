find_package("wpp")

############################################
# copy dlls for windows & set inc dir
############################################

if(WIN32)

    cmake_path(GET NGHTTP2_LIBRARY PARENT_PATH  LIB_DIR)
    cmake_path(GET LIB_DIR PARENT_PATH  BIN_PARENT)

    file(GLOB DLL_FILES ${BIN_PARENT}/bin/*.dll)

    if(WITH_LIBEVENT)
        file(COPY ${DLL_FILES} DESTINATION . PATTERN "boost*" EXCLUDE)
    else()
        file(COPY ${DLL_FILES} DESTINATION . PATTERN "event*" EXCLUDE)
    endif()

    get_filename_component(PRESET "${CMAKE_BINARY_DIR}" NAME)
    message("WPP_INCLUDES: ${CMAKE_CURRENT_LIST_DIR}/../../include")
    include_directories("${CMAKE_CURRENT_LIST_DIR}/../../include")

 endif()

