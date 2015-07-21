# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


SET(CPACK_BINARY_BUNDLE "")
SET(CPACK_BINARY_CYGWIN "")
SET(CPACK_BINARY_DEB "")
SET(CPACK_BINARY_DRAGNDROP "")
SET(CPACK_BINARY_NSIS "")
SET(CPACK_BINARY_OSXX11 "")
SET(CPACK_BINARY_PACKAGEMAKER "")
SET(CPACK_BINARY_RPM "")
SET(CPACK_BINARY_STGZ "")
SET(CPACK_BINARY_TBZ2 "")
SET(CPACK_BINARY_TGZ "")
SET(CPACK_BINARY_TZ "")
SET(CPACK_BINARY_WIX "")
SET(CPACK_BINARY_ZIP "")
SET(CPACK_CMAKE_GENERATOR "Unix Makefiles")
SET(CPACK_COMPONENTS_ALL "Runtime;Library;Header;Data;Documentation;Example;Other")
SET(CPACK_COMPONENTS_ALL_SET_BY_USER "TRUE")
SET(CPACK_COMPONENT_DATA_DESCRIPTION "Application data. Installed into share/lpeg.")
SET(CPACK_COMPONENT_DATA_DISPLAY_NAME "xsys Data")
SET(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION "Application documentation. Installed into share/lpeg/doc.")
SET(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "xsys Documentation")
SET(CPACK_COMPONENT_EXAMPLE_DESCRIPTION "Examples and their associated data. Installed into share/lpeg/example.")
SET(CPACK_COMPONENT_EXAMPLE_DISPLAY_NAME "xsys Examples")
SET(CPACK_COMPONENT_HEADER_DESCRIPTION "Headers needed for development. Installed into include.")
SET(CPACK_COMPONENT_HEADER_DISPLAY_NAME "xsys Development Headers")
SET(CPACK_COMPONENT_LIBRARY_DESCRIPTION "Static and import libraries needed for development. Installed into lib or bin.")
SET(CPACK_COMPONENT_LIBRARY_DISPLAY_NAME "xsys Development Libraries")
SET(CPACK_COMPONENT_OTHER_DESCRIPTION "Other unspecified content. Installed into share/lpeg/etc.")
SET(CPACK_COMPONENT_OTHER_DISPLAY_NAME "xsys Unspecified Content")
SET(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Executables and runtime libraries. Installed into bin.")
SET(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "xsys Runtime")
SET(CPACK_COMPONENT_TEST_DESCRIPTION "Tests and associated data. Installed into share/lpeg/test.")
SET(CPACK_COMPONENT_TEST_DISPLAY_NAME "xsys Tests")
SET(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
SET(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
SET(CPACK_GENERATOR "ZIP")
SET(CPACK_IGNORE_FILES "\\.git;_CPack_Packages;CMakeCache.txt;CPackSourceConfig.cmake;CPackConfig.cmake;CMakeFiles;\\.gz;\\.Z;\\.zip;x86_64\\.rpm;i386\\.rpm;i686\\.rpm;\\.o;\\.so;\\.a;src/bee$;src/00000000000000000001.snap;install_manifest.txt;cmake_install.cmake;test/var/;/home/wahyu/bee165b/Makefile;/home/wahyu/bee165b/test/Makefile;/home/wahyu/bee165b/test/lib/Makefile;/home/wahyu/bee165b/src/Makefile;/home/wahyu/bee165b/src/db/Makefile;/home/wahyu/bee165b/src/lib/Makefile;/home/wahyu/bee165b/src/trivia/config.h$;/home/wahyu/bee165b/src/Makefile;/home/wahyu/bee165b/extra/Makefile;/home/wahyu/bee165b/doc/Makefile;/home/wahyu/bee165b/doc/user/Makefile;/home/wahyu/bee165b/doc/developer/Makefile;/home/wahyu/bee165b/doc/man/Makefile")
SET(CPACK_INSTALLED_DIRECTORIES "/home/wahyu/bee165b;/")
SET(CPACK_INSTALL_CMAKE_PROJECTS "")
SET(CPACK_INSTALL_PREFIX "/usr/local")
SET(CPACK_MODULE_PATH "/home/wahyu/bee165b/cmake")
SET(CPACK_NSIS_DISPLAY_NAME "bee 1.6.5")
SET(CPACK_NSIS_DISPLAY_NAME_SET "TRUE")
SET(CPACK_NSIS_INSTALLER_ICON_CODE "")
SET(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
SET(CPACK_NSIS_PACKAGE_NAME "bee 1.6.5")
SET(CPACK_OUTPUT_CONFIG_FILE "/home/wahyu/bee165b/CPackConfig.cmake")
SET(CPACK_PACKAGE_DEFAULT_LOCATION "/")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "/usr/share/cmake-2.8/Templates/CPack.GenericDescription.txt")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "bee built using CMake")
SET(CPACK_PACKAGE_FILE_NAME "bee-1.6.5-150-gdc14e23-src")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "bee 1.6.5")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "bee 1.6.5")
SET(CPACK_PACKAGE_NAME "xsys")
SET(CPACK_PACKAGE_RELOCATABLE "true")
SET(CPACK_PACKAGE_VENDOR "LuaDist")
SET(CPACK_PACKAGE_VERSION "0.12")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "6")
SET(CPACK_PACKAGE_VERSION_PATCH "5")
SET(CPACK_RESOURCE_FILE_LICENSE "/usr/share/cmake-2.8/Templates/CPack.GenericLicense.txt")
SET(CPACK_RESOURCE_FILE_README "/usr/share/cmake-2.8/Templates/CPack.GenericDescription.txt")
SET(CPACK_RESOURCE_FILE_WELCOME "/usr/share/cmake-2.8/Templates/CPack.GenericWelcome.txt")
SET(CPACK_SET_DESTDIR "OFF")
SET(CPACK_SOURCE_CYGWIN "")
SET(CPACK_SOURCE_GENERATOR "TGZ")
SET(CPACK_SOURCE_IGNORE_FILES "\\.git;_CPack_Packages;CMakeCache.txt;CPackSourceConfig.cmake;CPackConfig.cmake;CMakeFiles;\\.gz;\\.Z;\\.zip;x86_64\\.rpm;i386\\.rpm;i686\\.rpm;\\.o;\\.so;\\.a;src/bee$;src/00000000000000000001.snap;install_manifest.txt;cmake_install.cmake;test/var/;/home/wahyu/bee165b/Makefile;/home/wahyu/bee165b/test/Makefile;/home/wahyu/bee165b/test/lib/Makefile;/home/wahyu/bee165b/src/Makefile;/home/wahyu/bee165b/src/db/Makefile;/home/wahyu/bee165b/src/lib/Makefile;/home/wahyu/bee165b/src/trivia/config.h$;/home/wahyu/bee165b/src/Makefile;/home/wahyu/bee165b/extra/Makefile;/home/wahyu/bee165b/doc/Makefile;/home/wahyu/bee165b/doc/user/Makefile;/home/wahyu/bee165b/doc/developer/Makefile;/home/wahyu/bee165b/doc/man/Makefile")
SET(CPACK_SOURCE_INSTALLED_DIRECTORIES "/home/wahyu/bee165b;/")
SET(CPACK_SOURCE_OUTPUT_CONFIG_FILE "/home/wahyu/bee165b/CPackSourceConfig.cmake")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "bee-1.6.5-150-gdc14e23-src")
SET(CPACK_SOURCE_TBZ2 "")
SET(CPACK_SOURCE_TGZ "")
SET(CPACK_SOURCE_TOPLEVEL_TAG "Linux-Source")
SET(CPACK_SOURCE_TZ "")
SET(CPACK_SOURCE_ZIP "")
SET(CPACK_STRIP_FILES "TRUE")
SET(CPACK_SYSTEM_NAME "Linux")
SET(CPACK_TOPLEVEL_TAG "Linux-Source")
SET(CPACK_WIX_SIZEOF_VOID_P "8")
