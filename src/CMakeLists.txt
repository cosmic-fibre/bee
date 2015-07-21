#
# Set compile flags for entire src/ directory
#
enable_tnt_compile_flags()

add_subdirectory(trivia)

include_directories(${LIBEV_INCLUDE_DIR})
include_directories(${LIBEIO_INCLUDE_DIR})
include_directories(${LIBCORO_INCLUDE_DIR})
include_directories(${LIBGOPT_INCLUDE_DIR})
include_directories(${READLINE_INCLUDE_DIR})


# Compile src/lua/*.lua files into src/lua/*.lua.c sources
set(lua_sources)
lua_source(lua_sources lua/init.lua)
lua_source(lua_sources lua/fiber.lua)
lua_source(lua_sources lua/uuid.lua)
lua_source(lua_sources lua/digest.lua)
lua_source(lua_sources lua/msgpackffi.lua)
lua_source(lua_sources lua/uri.lua)
lua_source(lua_sources lua/console.lua)
lua_source(lua_sources lua/bsdsocket.lua)
lua_source(lua_sources lua/errno.lua)
lua_source(lua_sources lua/log.lua)
lua_source(lua_sources lua/net_db.lua)
lua_source(lua_sources lua/help.lua)
lua_source(lua_sources lua/help_en_US.lua)
lua_source(lua_sources lua/tap.lua)
lua_source(lua_sources lua/fio.lua)
lua_source(lua_sources ../third_party/luafun/fun.lua)
lua_source(lua_sources ../third_party/astr/astr.lua)
lua_source(lua_sources ../third_party/tools/shard.lua)
lua_source(lua_sources ../third_party/tools/csv.lua)
lua_source(lua_sources ../third_party/expiration/expirationd.lua)
lua_source(lua_sources ../third_party/sodium/crypto_init.lua)
lua_source(lua_sources ../third_party/sodium/crypto_auth.lua)
lua_source(lua_sources ../third_party/sodium/crypto_box.lua)
lua_source(lua_sources ../third_party/sodium/crypto_core.lua)
lua_source(lua_sources ../third_party/sodium/crypto_generichash.lua)
lua_source(lua_sources ../third_party/sodium/crypto_hash.lua)
lua_source(lua_sources ../third_party/sodium/crypto_hashblocks.lua)
lua_source(lua_sources ../third_party/sodium/crypto_onetimeauth.lua)
lua_source(lua_sources ../third_party/sodium/crypto_scalarmult.lua)
lua_source(lua_sources ../third_party/sodium/crypto_secretbox.lua)
lua_source(lua_sources ../third_party/sodium/crypto_shorthash.lua)
lua_source(lua_sources ../third_party/sodium/crypto_secretbox.lua)
lua_source(lua_sources ../third_party/sodium/crypto_sign.lua)
lua_source(lua_sources ../third_party/sodium/crypto_stream.lua)
lua_source(lua_sources ../third_party/sodium/crypto_verify.lua)
lua_source(lua_sources ../third_party/sodium/randombytes.lua)
lua_source(lua_sources ../third_party/config/cfg.lua)


add_custom_target(generate_lua_sources
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/db
    DEPENDS ${lua_sources})
set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${lua_sources})

add_custom_target(ragel
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND ragel -G2 src/uri.rl -o src/uri.c)

set (core_sources
     say.cc
     memory.cc
     fiber.cc
     exception.cc
     coro.cc
     object.cc
     assoc.c
 )

add_library(core STATIC ${core_sources})
target_link_libraries(core
    salad small pthread
    ${LIBEV_LIBRARIES}
    ${LIBCORO_LIBRARIES}
)

set (server_sources
     util.cc
     find_path.c
     sio.cc
     evio.cc
     coio.cc
     coeio.cc
     iobuf.cc
     coio_buf.cc
     pickle.cc
     stat.cc
     ipc.cc
     errinj.cc
     fio.c
     crc32.c
     random.c
     scramble.c
     opts.c
     cfg.cc
     cpu_feature.c
     fiob.c
     tt_uuid.c
     ffisyms.cc
     uri.c
     coeio_file.cc
     lua/digest.cc
     lua/init.cc
     lua/fiber.cc
     lua/trigger.cc
     lua/ipc.cc
     lua/msgpack.cc
     lua/utils.cc
     lua/errno.c
     lua/bsdsocket.cc
     lua/pickle.cc
     lua/fio.cc
     ${lua_sources}
)

if (ENABLE_TRACE)
    set (server_sources ${server_sources} trace.c)
endif()

set_source_files_compile_flags(${server_sources})
add_library(server STATIC ${server_sources})
target_link_libraries(server core)

set (common_libraries server core misc bitset msgpuck)

list(APPEND common_libraries
    ${LIBEIO_LIBRARIES}
    ${LIBGOPT_LIBRARIES}
    ${LIBCJSON_LIBRARIES}
    ${LIBYAML_LIBRARIES}
    ${READLINE_LIBRARIES}
    ${LUAJIT_LIB}
)

if (TARGET_OS_LINUX OR TARGET_OS_DEBIAN_FREEBSD)
    set (common_libraries ${common_libraries} dl)
endif()

if (TARGET_OS_FREEBSD AND NOT TARGET_OS_DEBIAN_FREEBSD)
    find_library (INTL intl)
    if (NOT INTL)
        message(FATAL_ERROR "intl library not found")
    else()
        set (common_libraries ${common_libraries} ${INTL})
    endif()
endif()

if (ENABLE_BACKTRACE AND HAVE_BFD)
    find_package (ZLIB)
    set (common_libraries ${common_libraries} bfd ${ZLIB_LIBRARIES})
    if (NOT TARGET_OS_DARWIN)
        set (common_libraries ${common_libraries} iberty)
    endif()
    if (TARGET_OS_FREEBSD AND NOT TARGET_OS_DEBIAN_FREEBSD)
        set (common_libraries ${common_libraries} iconv)
    endif()
endif()

set (common_libraries ${common_libraries} ${LIBUUID_LIBRARIES})
set (common_libraries ${common_libraries} PARENT_SCOPE)

add_subdirectory(lib)
add_subdirectory(db)

# Save CMAKE_XXX_FLAGS from this directory for config.h (used in --version)
set(BEE_C_FLAGS ${CMAKE_C_FLAGS} PARENT_SCOPE)
set(BEE_CXX_FLAGS ${CMAKE_CXX_FLAGS} PARENT_SCOPE)

add_executable(bee main.cc)
add_dependencies(bee build_bundled_libs)
target_link_libraries(bee db ${common_libraries})

if (TARGET_OS_DARWIN)
    # Necessary to make LuaJIT work on Darwin, see
    # http://luajit.org/install.html
    set_target_properties(bee PROPERTIES
        LINK_FLAGS "-pagezero_size 10000 -image_base 100000000")
elseif (TARGET_OS_FREEBSD AND NOT TARGET_OS_DEBIAN_FREEBSD)
    # Helps symbol resolution of plug-ins
    set_target_properties(bee PROPERTIES LINK_FLAGS "-rdynamic")
endif()
install (TARGETS bee DESTINATION bin)
