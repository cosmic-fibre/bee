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
