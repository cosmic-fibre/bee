#ifndef BEE_DB_MEMTX_BITSET_H_INCLUDED
#define BEE_DB_MEMTX_BITSET_H_INCLUDED
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
 * @brief Index API wrapper for bitset_index
 * @see bitset/index.h
 */
#include "index.h"
#include "bitset/index.h"

struct bitset_index;
struct bitset_expr;

class MemtxBitset: public Index {
public:
	MemtxBitset(struct key_def *key_def);
	virtual ~MemtxBitset();
	virtual size_t size() const;
	virtual size_t count(enum iterator_type type, const char *key,
			     uint32_t part_count) const;
	virtual struct tuple *findByKey(const char *key, uint32_t part_count) const;
	virtual struct tuple *replace(struct tuple *old_tuple,
				      struct tuple *new_tuple,
				      enum dup_replace_mode mode);

	virtual size_t bsize() const;
	virtual struct iterator *allocIterator() const;
	virtual void initIterator(struct iterator *iterator,
				  enum iterator_type type,
				  const char *key, uint32_t part_count) const;
private:
	struct bitset_index index;
};

#endif /* BEE_DB_MEMTX_BITSET_H_INCLUDED */
