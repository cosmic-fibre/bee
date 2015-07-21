# The set of languages for which implicit dependencies are needed:
SET(CMAKE_DEPENDS_LANGUAGES
  "C"
  "CXX"
  )
# The set of files for implicit dependencies of each language:
SET(CMAKE_DEPENDS_CHECK_C
  "/home/wahyu/bee165b/src/assoc.c" "/home/wahyu/bee165b/src/CMakeFiles/core.dir/assoc.c.o"
  )
SET(CMAKE_C_COMPILER_ID "GNU")
SET(CMAKE_DEPENDS_CHECK_CXX
  "/home/wahyu/bee165b/src/coro.cc" "/home/wahyu/bee165b/src/CMakeFiles/core.dir/coro.cc.o"
  "/home/wahyu/bee165b/src/exception.cc" "/home/wahyu/bee165b/src/CMakeFiles/core.dir/exception.cc.o"
  "/home/wahyu/bee165b/src/fiber.cc" "/home/wahyu/bee165b/src/CMakeFiles/core.dir/fiber.cc.o"
  "/home/wahyu/bee165b/src/memory.cc" "/home/wahyu/bee165b/src/CMakeFiles/core.dir/memory.cc.o"
  "/home/wahyu/bee165b/src/object.cc" "/home/wahyu/bee165b/src/CMakeFiles/core.dir/object.cc.o"
  "/home/wahyu/bee165b/src/say.cc" "/home/wahyu/bee165b/src/CMakeFiles/core.dir/say.cc.o"
  )
SET(CMAKE_CXX_COMPILER_ID "GNU")

# Preprocessor definitions for this target.
SET(CMAKE_TARGET_DEFINITIONS
  "CORO_ASM"
  "_FILE_OFFSET_BITS=64"
  "_GNU_SOURCE"
  "__STDC_CONSTANT_MACROS=1"
  "__STDC_FORMAT_MACROS=1"
  "__STDC_LIMIT_MACROS=1"
  )

# Targets to which this target links.
SET(CMAKE_TARGET_LINKED_INFO_FILES
  "/home/wahyu/bee165b/src/lib/salad/CMakeFiles/salad.dir/DependInfo.cmake"
  "/home/wahyu/bee165b/src/lib/small/CMakeFiles/small.dir/DependInfo.cmake"
  "/home/wahyu/bee165b/CMakeFiles/ev.dir/DependInfo.cmake"
  "/home/wahyu/bee165b/CMakeFiles/coro.dir/DependInfo.cmake"
  )

# The include file search paths:
SET(CMAKE_C_TARGET_INCLUDE_PATH
  "src"
  "src/lib"
  "."
  "third_party/luajit/src"
  "third_party"
  "third_party/coro"
  "third_party/gopt"
  )
SET(CMAKE_CXX_TARGET_INCLUDE_PATH ${CMAKE_C_TARGET_INCLUDE_PATH})
SET(CMAKE_Fortran_TARGET_INCLUDE_PATH ${CMAKE_C_TARGET_INCLUDE_PATH})
SET(CMAKE_ASM_TARGET_INCLUDE_PATH ${CMAKE_C_TARGET_INCLUDE_PATH})
