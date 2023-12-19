include("out/build/gcc-release-${BACKEND}/CPackConfig.cmake")

set(CPACK_INSTALL_CMAKE_PROJECTS
    "out/build/gcc-release-${BACKEND};${CPACK_PACKAGE_NAME};ALL;/"
    "out/build/gcc-debug-${BACKEND};${CPACK_PACKAGE_NAME};ALL;/"
    )