include("out/build/gcc-release-libevent/CPackConfig.cmake")

set(CPACK_INSTALL_CMAKE_PROJECTS
    "out/build/gcc-release-libevent;${CPACK_PACKAGE_NAME};ALL;/"
    "out/build/gcc-debug-libevent;${CPACK_PACKAGE_NAME};ALL;/"
    )