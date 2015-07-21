#ifndef BEE_UTIL_H_INCLUDED
#define BEE_UTIL_H_INCLUDED
/*
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "trivia/config.h"

#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#define restrict __restrict__

#ifndef NDEBUG
#define TRASH(ptr) memset(ptr, '#', sizeof(*ptr))
#else
#define TRASH(ptr)
#endif

#ifndef MAX
# define MAX(a, b) ((a) > (b) ? (a) : (b))
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

/* Macros to define enum and corresponding strings. */
#define ENUM0_MEMBER(s, ...) s,
#define ENUM_MEMBER(s, v, ...) s = v,
#define ENUM0(enum_name, enum_members) enum enum_name {enum_members(ENUM0_MEMBER) enum_name##_MAX}
#define ENUM(enum_name, enum_members) enum enum_name {enum_members(ENUM_MEMBER) enum_name##_MAX}
#if defined(__cplusplus)
#define ENUM_STRS_MEMBER(s, v, ...) names[s] = #s;
/* A special hack to emulate C99 designated initializers */
#define STRS(enum_name, enum_members)					\
	const char *enum_name##_strs[enum_name##_MAX];			\
	namespace {							\
		const struct enum_name##_strs_init {			\
			enum_name##_strs_init(const char **names) {	\
				memset(names, 0, sizeof(*names) *	\
				       enum_name##_MAX);		\
				enum_members(ENUM_STRS_MEMBER)		\
			}						\
		}  enum_name##_strs_init(enum_name##_strs);		\
	}
#else /* !defined(__cplusplus) */
#define ENUM_STRS_MEMBER(s, v, ...) [s] = #s,
#define STRS(enum_name, enum_members) \
	const char *enum_name##_strs[(unsigned) enum_name##_MAX + 1] = {enum_members(ENUM_STRS_MEMBER) 0}
#endif
#define STR2ENUM(enum_name, str) ((enum enum_name) strindex(enum_name##_strs, str, enum_name##_MAX))

uint32_t
strindex(const char **haystack, const char *needle, uint32_t hmax);

// Macros for printf functions
#ifdef __x86_64__
#define PRI_SZ  "lu"
#define PRI_SSZ "ld"
#define PRI_OFFT "lu"
#define PRI_XFFT "lx"
#else
#define PRI_SZ  "u"
#define PRI_SSZ "d"
#define PRI_OFFT "llu"
#define PRI_XFFT "llx"
#endif

#define nelem(x)     (sizeof((x))/sizeof((x)[0]))
#define likely(x)    __builtin_expect((x),1)
#define unlikely(x)  __builtin_expect((x),0)
#define field_sizeof(compound_type, field) sizeof(((compound_type *)NULL)->field)

#ifndef offsetof
#define offsetof(type, member) ((size_t) &((type *)0)->member)
#endif

#ifndef __offsetof
#define __offsetof offsetof
#endif

#ifndef lengthof
#define lengthof(array) (sizeof (array) / sizeof ((array)[0]))
#endif

#ifndef TYPEALIGN
#define TYPEALIGN(ALIGNVAL,LEN)  \
        (((uintptr_t) (LEN) + ((ALIGNVAL) - 1)) & ~((uintptr_t) ((ALIGNVAL) - 1)))

#define SHORTALIGN(LEN)                 TYPEALIGN(sizeof(int16_t), (LEN))
#define INTALIGN(LEN)                   TYPEALIGN(sizeof(int32_t), (LEN))
#define MAXALIGN(LEN)                   TYPEALIGN(sizeof(int64_t), (LEN))
#define PTRALIGN(LEN)                   TYPEALIGN(sizeof(void*), (LEN))
#define CACHEALIGN(LEN)			TYPEALIGN(32, (LEN))
#endif

#define CRLF "\n"

#ifdef GCC
# define FORMAT_PRINTF gnu_printf
#else
# define FORMAT_PRINTF printf
#endif

extern int forked;
pid_t tfork();
void close_all_xcpt(int fdc, ...);
void coredump(int dump_interval);

void __gcov_flush();

/**
 * Async-signal-safe implementation of printf(), to
 * be able to write messages into the error log
 * inside a signal handler.
 */
ssize_t
fdprintf(int fd, const char *format, ...) __attribute__((format(printf, 2, 3)));

extern void *__libc_stack_end;

#ifdef ENABLE_BACKTRACE
void print_backtrace();
char *backtrace(void *frame, void *stack, size_t stack_size);

typedef int (backtrace_cb)(int frameno, void *frameret,
                           const char *func, size_t offset, void *cb_ctx);
void backtrace_foreach(backtrace_cb cb, void *frame, void *stack, size_t stack_size,
                       void *cb_ctx);
#endif /* ENABLE_BACKTRACE */

#ifdef HAVE_BFD
struct symbol {
	void *addr;
	const char *name;
	void *end;
};
struct symbol *addr2symbol(void *addr);
void symbols_load(const char *name);
void symbols_free();
#endif /* HAVE_BFD */
char *find_path(const char *argv0);

char *abspath(const char *filename);

char *
int2str(long int val);

#ifndef HAVE_MEMMEM
/* Declare memmem(). */
void *
memmem(const void *block, size_t blen, const void *pat, size_t plen);
#endif /* HAVE_MEMMEM */

#ifndef HAVE_MEMRCHR
/* Declare memrchr(). */
void *
memrchr(const void *s, int c, size_t n);
#endif /* HAVE_MEMRCHR */

#ifndef HAVE_OPEN_MEMSTREAM
/* Declare open_memstream(). */
#include <stdio.h>
FILE *
open_memstream(char **ptr, size_t *sizeloc);
#endif /* HAVE_OPEN_MEMSTREAM */

#ifndef HAVE_FMEMOPEN
/* Declare open_memstream(). */
#include <stdio.h>
FILE *
fmemopen(void *buf, size_t size, const char *mode);
#endif /* HAVE_FMEMOPEN */

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */

#endif /* BEE_UTIL_H_INCLUDED */
