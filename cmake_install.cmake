# Install script for directory: /home/wahyu/bee165b

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Debug")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/bee" TYPE FILE FILES
    "/home/wahyu/bee165b/third_party/luajit/src/lua.h"
    "/home/wahyu/bee165b/third_party/luajit/src/lualib.h"
    "/home/wahyu/bee165b/third_party/luajit/src/lauxlib.h"
    "/home/wahyu/bee165b/third_party/luajit/src/luaconf.h"
    "/home/wahyu/bee165b/third_party/luajit/src/lua.hpp"
    "/home/wahyu/bee165b/third_party/luajit/src/luajit.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/bee" TYPE FILE FILES
    "/home/wahyu/bee165b/README.md"
    "/home/wahyu/bee165b/LICENSE"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/home/wahyu/bee165b/src/cmake_install.cmake")
  INCLUDE("/home/wahyu/bee165b/extra/cmake_install.cmake")
  INCLUDE("/home/wahyu/bee165b/test/cmake_install.cmake")
  INCLUDE("/home/wahyu/bee165b/doc/cmake_install.cmake")
  INCLUDE("/home/wahyu/bee165b/third_party/http/cmake_install.cmake")
  INCLUDE("/home/wahyu/bee165b/third_party/lpeg/cmake_install.cmake")
  INCLUDE("/home/wahyu/bee165b/third_party/lanes/cmake_install.cmake")
  INCLUDE("/home/wahyu/bee165b/third_party/xsys/cmake_install.cmake")
  INCLUDE("/home/wahyu/bee165b/third_party/sci/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)

FILE(WRITE "/home/wahyu/bee165b/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "/home/wahyu/bee165b/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)
