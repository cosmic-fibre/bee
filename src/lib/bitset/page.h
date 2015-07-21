#ifndef BEE_LIB_BITSET_PAGE_H_INCLUDED
#define BEE_LIB_BITSET_PAGE_H_INCLUDED
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

/**
 * @file
 * @brief Bitset page
 *
 * Private header file, please don't use directly.
 * @internal
 */

#include "bitset/bitset.h"

#include <stdlib.h>
#if defined(DEBUG)
#include <stdio.h> /* for dumping bitset_page to file */
#endif /* defined(DEBUG) */
#include <string.h>
#include <limits.h>
#include <assert.h>

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

enum {
	/** How many bytes to store in one page */
	BITSET_PAGE_DATA_SIZE = 160
};

#if defined(ENABLE_AVX)
typedef __m256i bitset_word_t;
#define BITSET_PAGE_DATA_ALIGNMENT 32
#elif defined(ENABLE_SSE2)
typedef __m128i bitset_word_t;
#define BITSET_PAGE_DATA_ALIGNMENT 16
#elif defined(__x86_64__)
typedef uint64_t bitset_word_t;
#define BITSET_PAGE_DATA_ALIGNMENT 1
#else
#define BITSET_PAGE_DATA_ALIGNMENT 1
typedef uint32_t bitset_word_t;
#endif

#if (defined(__GLIBC__) && (__WORDSIZE == 64) && \
     ((__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 8))) || \
    (defined(__FreeBSD__) && !defined(__arm__) && !defined(__mips__)) || \
    (defined(__APPLE__))
/**
 * @brief Defined if malloc is 16-byte aligned
 * @see http://www.gnu.org/software/libc/manual/html_node/Aligned-Memory-Blocks.html
 * @see http://svn.freebsd.org/viewvc/base/stable/6/lib/libc/stdlib/malloc.c
 * @see http://svn.freebsd.org/viewvc/base/stable/7/lib/libc/stdlib/malloc.c
 * @see man malloc(2) on OS X
 */
#define MALLOC_ALIGNMENT 16
#else
#define MALLOC_ALIGNMENT 8
#endif /* aligned malloc */

inline size_t
bitset_page_alloc_size(void *(*realloc_arg)(void *ptr, size_t size))
{
	if (BITSET_PAGE_DATA_ALIGNMENT <= 1 || (
		(MALLOC_ALIGNMENT % BITSET_PAGE_DATA_ALIGNMENT == 0) &&
		(sizeof(struct bitset_page) % BITSET_PAGE_DATA_ALIGNMENT == 0) &&
		(realloc_arg == realloc))) {

		/* Alignment is not needed */
		return sizeof(struct bitset_page) + BITSET_PAGE_DATA_SIZE;
	}

	return sizeof(struct bitset_page) + BITSET_PAGE_DATA_SIZE +
			BITSET_PAGE_DATA_ALIGNMENT;
}

#undef MALLOC_ALIGNMENT

inline void *
bitset_page_data(struct bitset_page *page)
{
	uintptr_t r = (uintptr_t) (page->data + BITSET_PAGE_DATA_ALIGNMENT - 1);
	return (void *) (r & ~((uintptr_t) BITSET_PAGE_DATA_ALIGNMENT - 1));
}

inline void
bitset_page_create(struct bitset_page *page)
{
	size_t size = ((char *) bitset_page_data(page) - (char *) page)
			+ BITSET_PAGE_DATA_SIZE;
	memset(page, 0, size);
}

inline void
bitset_page_destroy(struct bitset_page *page)
{
	(void) page;
	/* nothing */
}

inline size_t
bitset_page_first_pos(size_t pos) {
	return pos - (pos % (BITSET_PAGE_DATA_SIZE * CHAR_BIT));
}

inline void
bitset_page_set_zeros(struct bitset_page *page)
{
	void *data = bitset_page_data(page);
	memset(data, 0, BITSET_PAGE_DATA_SIZE);
}

inline void
bitset_page_set_ones(struct bitset_page *page)
{
	void *data = bitset_page_data(page);
	memset(data, -1, BITSET_PAGE_DATA_SIZE);
}

inline void
bitset_page_and(struct bitset_page *dst, struct bitset_page *src)
{
	bitset_word_t *d = (bitset_word_t *) bitset_page_data(dst);
	bitset_word_t *s = (bitset_word_t *) bitset_page_data(src);

	assert(BITSET_PAGE_DATA_SIZE % sizeof(bitset_word_t) == 0);
	int cnt = BITSET_PAGE_DATA_SIZE / sizeof(bitset_word_t);
	for (int i = 0; i < cnt; i++) {
		*d++ &= *s++;
	}
}

inline void
bitset_page_nand(struct bitset_page *dst, struct bitset_page *src)
{
	bitset_word_t *d = (bitset_word_t *) bitset_page_data(dst);
	bitset_word_t *s = (bitset_word_t *) bitset_page_data(src);

	assert(BITSET_PAGE_DATA_SIZE % sizeof(bitset_word_t) == 0);
	int cnt = BITSET_PAGE_DATA_SIZE / sizeof(bitset_word_t);
	for (int i = 0; i < cnt; i++) {
		*d++ &= ~*s++;
	}
}

inline void
bitset_page_or(struct bitset_page *dst, struct bitset_page *src)
{
	bitset_word_t *d = (bitset_word_t *) bitset_page_data(dst);
	bitset_word_t *s = (bitset_word_t *) bitset_page_data(src);

	assert(BITSET_PAGE_DATA_SIZE % sizeof(bitset_word_t) == 0);
	int cnt = BITSET_PAGE_DATA_SIZE / sizeof(bitset_word_t);
	for (int i = 0; i < cnt; i++) {
		*d++ |= *s++;
	}
}

#if defined(DEBUG)
void
bitset_page_dump(struct bitset_page *page, FILE *stream);
#endif /* defined(DEBUG) */

rb_proto(, bitset_pages_, bitset_pages_t, struct bitset_page)


#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */

#endif /* BEE_LIB_BITSET_PAGE_H_INCLUDED */
