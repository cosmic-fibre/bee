
#
# List generators
#
set (CPACK_GENERATOR "TGZ")
set (CPACK_SOURCE_GENERATOR "TGZ")

#
# Ignoring generated files
#
set (CPACK_SOURCE_IGNORE_FILES
    "\\\\.git" "_CPack_Packages"
    "CMakeCache.txt" "CPackSourceConfig.cmake" "CPackConfig.cmake"
    "CMakeFiles" "\\\\.gz" "\\\\.Z" "\\\\.zip"
    "x86_64\\\\.rpm" "i386\\\\.rpm" "i686\\\\.rpm"
    "\\\\.o" "\\\\.so" "\\\\.a"
    "src/bee$"
    "src/00000000000000000001.snap"
    "install_manifest.txt"
    "cmake_install.cmake" "test/var/"
)

set (CPACK_SOURCE_IGNORE_FILES "${CPACK_SOURCE_IGNORE_FILES}"
    "${CMAKE_SOURCE_DIR}/Makefile"
    "${CMAKE_SOURCE_DIR}/test/Makefile"
    "${CMAKE_SOURCE_DIR}/test/lib/Makefile"
    "${CMAKE_SOURCE_DIR}/src/Makefile"
    "${CMAKE_SOURCE_DIR}/src/db/Makefile"
    "${CMAKE_SOURCE_DIR}/src/lib/Makefile"
    "${CMAKE_SOURCE_DIR}/src/trivia/config.h$"
    "${CMAKE_SOURCE_DIR}/src/Makefile"
    "${CMAKE_SOURCE_DIR}/extra/Makefile"
    "${CMAKE_SOURCE_DIR}/doc/Makefile"
    "${CMAKE_SOURCE_DIR}/doc/user/Makefile"
    "${CMAKE_SOURCE_DIR}/doc/developer/Makefile"
    "${CMAKE_SOURCE_DIR}/doc/man/Makefile"
)

set (CPACK_SOURCE_PACKAGE_FILE_NAME "bee-${PACKAGE_VERSION}-src")

#
# Provide options for the binary distribution.
#
string (TOLOWER "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}" platform)

set (CPACK_PACKAGE_FILE_NAME "bee-${PACKAGE_VERSION}-${platform}")
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set (CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-debug")
endif()


if (${TARGET_OS_DARWIN})
    set (CPACK_GENERATOR "PackageMaker")
    set (CPACK_SOURCE_GENERATOR "PackageMaker")
    set (CPACK_PACKAGE_VENDOR "bee")
    set (CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
    set (CPACK_POSTFLIGHT_SCRIPT "${CMAKE_BINARY_DIR}/extra/postflight")
    set (CPACK_RESOURCE_FILE_WELCOME "${PROJECT_SOURCE_DIR}/extra/dmg/DESCRIPTION.rtf")
    set (CPACK_RESOURCE_FILE_README  "${PROJECT_SOURCE_DIR}/extra/dmg/README.rtf")
    set (CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/extra/dmg/LICENSE.rtf")
endif()

##
include (CPack)
