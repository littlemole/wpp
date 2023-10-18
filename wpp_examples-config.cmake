find_package("wpp")

############################################
# dlls for windows if with libevent
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

endif()