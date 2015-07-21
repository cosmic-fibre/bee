#
# A macro to build the bundled libhttp
macro(libhttp_build)

    set(http_src ${PROJECT_SOURCE_DIR}/third_party/http/lib.c)

    add_library(http STATIC ${http_src})

    set(LIBHTTP_INCLUDE_DIR ${PROJECT_BINARY_DIR}/third_party/http)
    set(LIBHTTP_LIBRARIES http)

    message(STATUS "Use bundled libhttp includes: ${LIBHTTP_INCLUDE_DIR}/.h")
    message(STATUS "Use bundled libhttp library: ${LIBHTTP_LIBRARIES}")

    unset(lua_http_src)
endmacro(libhttp_build)

