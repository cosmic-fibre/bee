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

#include "page.h"
#include "bitset/bitset.h"

extern inline size_t
bitset_page_alloc_size(void *(*realloc_arg)(void *ptr, size_t size));

extern inline void *
bitset_page_data(struct bitset_page *page);

extern inline void
bitset_page_create(struct bitset_page *page);

extern inline void
bitset_page_destroy(struct bitset_page *page);

extern inline size_t
bitset_page_first_pos(size_t pos);

extern inline void
bitset_page_set_zeros(struct bitset_page *page);

extern inline void
bitset_page_set_ones(struct bitset_page *page);

extern inline void
bitset_page_and(struct bitset_page *dst, struct bitset_page *src);

extern inline void
bitset_page_nand(struct bitset_page *dst, struct bitset_page *src);

extern inline void
bitset_page_or(struct bitset_page *dst, struct bitset_page *src);

#if defined(DEBUG)
void
bitset_page_dump(struct bitset_page *page, FILE *stream)
{
	fprintf(stream, "Page %zu:\n", page->first_pos);
	char *d = bitset_page_data(page);
	for (int i = 0; i < BITSET_PAGE_DATA_SIZE; i++) {
		fprintf(stream, "%x ", *d);
		d++;
	}
	fprintf(stream, "\n--\n");
}
#endif /* defined(DEBUG) */

static inline int
page_cmp(const struct bitset_page *a, const struct bitset_page *b)
{
	if (a->first_pos < b->first_pos) {
		return -1;
	} else if (a->first_pos > b->first_pos) {
		return 1;
	} else {
		return 0;
	}
}

rb_gen(, bitset_pages_, bitset_pages_t, struct bitset_page, node, page_cmp)
