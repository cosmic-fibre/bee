#ifndef BEE_MODULE_H_INCLUDED
#define BEE_MODULE_H_INCLUDED

#include <stddef.h>
#include <stdarg.h> /* va_list */
#include <errno.h>
#include <string.h> /* strerror(3) */
#include <stdint.h>
#include <sys/types.h> /* ssize_t */


/**
 * \file
 * Bee Module API
 */

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#include <lua.h>  /* does not have extern C wrappers */
/** \cond public */
/*
 * A string with major-minor-patch-commit-id identifier of the
 * release.
 */
#define PACKAGE_VERSION "1.6.5-150-gdc14e23"
#define PACKAGE_VERSION_MAJOR 1
#define PACKAGE_VERSION_MINOR 6
#define PACKAGE_VERSION_PATCH 5

/** \endcond public */
/** \cond public */

/*
 * predefined /etc directory prefix.
 */
#define SYSCONF_DIR "etc"
#define INSTALL_PREFIX "/usr/local"
#define BUILD_TYPE "Debug"
#define BUILD_INFO "Linux-x86_64-Debug"
#define BUILD_OPTIONS "cmake . -DCMAKE_INSTALL_PREFIX=/usr/local -DENABLE_TRACE=ON -DENABLE_BACKTRACE=ON"
#define COMPILER_INFO "/usr/bin/cc /usr/bin/c++"
#define BEE_C_FLAGS " -fno-omit-frame-pointer -fno-stack-protector -fexceptions -funwind-tables -fopenmp -msse2 -std=c11 -Wall -Wextra -Wno-sign-compare -Wno-strict-aliasing -fno-gnu89-inline -Werror"
#define BEE_CXX_FLAGS " -fno-omit-frame-pointer -fno-stack-protector -fexceptions -funwind-tables -fopenmp -msse2 -std=c++11 -Wall -Wextra -Wno-sign-compare -Wno-strict-aliasing -Wno-invalid-offsetof -Werror"

/*
 * Modules
 */
#define MODULE_LIBDIR "/usr/local/lib/x86_64-linux-gnu/bee"
#define MODULE_LUADIR "/usr/local/share/bee"
#define MODULE_INCLUDEDIR "/usr/local/include/bee"
#define MODULE_LUAPATH "/usr/local/share/bee/?.lua;/usr/local/share/bee/?/init.lua"
#define MODULE_LIBPATH "/usr/local/lib/x86_64-linux-gnu/bee/?.so"

/** \endcond public */
/** \cond public */

enum say_level {
	S_FATAL,		/* do not this value use directly */
	S_SYSERROR,
	S_ERROR,
	S_CRIT,
	S_WARN,
	S_INFO,
	S_DEBUG
};

/** \endcond public */
/** \cond public */
typedef void (*sayfunc_t)(int level, const char *filename, int line, const char *error,
                          const char *format, ...);

extern sayfunc_t _say __attribute__ ((format(printf, 5, 6)));

#define say(level, ...) ({ _say(level, __FILE__, __LINE__, __VA_ARGS__); })

#define panic_status(status, ...)	({ say(S_FATAL, NULL, __VA_ARGS__); exit(status); })
#define panic(...)			panic_status(EXIT_FAILURE, __VA_ARGS__)
#define panic_syserror(...)		({ say(S_FATAL, strerror(errno), __VA_ARGS__); exit(EXIT_FAILURE); })
#define say_syserror(...)		say(S_SYSERROR, strerror(errno), __VA_ARGS__)
#define say_error(...)			say(S_ERROR, NULL, __VA_ARGS__)
#define say_crit(...)			say(S_CRIT, NULL, __VA_ARGS__)
#define say_warn(...)			say(S_WARN, NULL, __VA_ARGS__)
#define say_info(...)			say(S_INFO, NULL, __VA_ARGS__)
#define say_debug(...)			say(S_DEBUG, NULL, __VA_ARGS__)
/** \endcond public */
/** \cond public */
ssize_t
coio_call(ssize_t (*func)(va_list ap), ...);

struct addrinfo;

int
coio_getaddrinfo(const char *host, const char *port,
		 const struct addrinfo *hints, struct addrinfo **res,
		 double timeout);
/** \endcond public */
/** \cond public */

/**
 * @brief Allocate a new block of memory with the given size, push onto the
 * stack a new cdata of type ctypeid with the block address, and return
 * this address. Allocated memory is a subject of GC.
 * CTypeID must be used from FFI at least once.
 * @param L Lua State
 * @param ctypeid FFI's CTypeID of this cdata
 * @param size size to allocate
 * @sa luaL_checkcdata
 * @return memory associated with this cdata
 */
LUA_API void *
luaL_pushcdata(struct lua_State *L, uint32_t ctypeid, uint32_t size);

/**
 * @brief Checks whether the function argument idx is a cdata
 * @param L Lua State
 * @param idx stack index
 * @param ctypeid FFI's CTypeID of this cdata
 * @sa luaL_pushcdata
 * @return memory associated with this cdata
 */
LUA_API void *
luaL_checkcdata(struct lua_State *L, int idx, uint32_t *ctypeid);

/**
 * @brief Sets finalizer function on a cdata object.
 * Equivalent to call ffi.gc(obj, function).
 * Finalizer function must be on the top of the stack.
 * @param L Lua State
 * @param idx object
 * @return 1
 */
LUA_API int
luaL_setcdatagc(struct lua_State *L, int idx);

/**
* @brief Return CTypeID (FFI) of given СDATA type
* @param L Lua State
* @param ctypename С type name as string (e.g. "struct request" or "uint32_t")
* @sa luaL_pushcdata
* @sa luaL_checkcdata
* @return CTypeID
*/
LUA_API uint32_t
luaL_ctypeid(struct lua_State *L, const char *ctypename);

/** \endcond public */
/** \cond public */

/**
 * push uint64_t to Lua stack
 *
 * @param L is a Lua State
 * @param val is a value to push
 *
 */
LUA_API int
luaL_pushuint64(struct lua_State *L, uint64_t val);

/**
 * @copydoc luaL_pushnumber64
 */
LUA_API int
luaL_pushint64(struct lua_State *L, int64_t val);

/** \endcond public */
#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */

#endif /* BEE_MODULE_H_INCLUDED */
