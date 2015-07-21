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

#include "memtx_bitset.h"

#include <string.h>

#include "tuple.h"

static inline struct tuple *
value_to_tuple(size_t value);

static inline size_t
tuple_to_value(struct tuple *tuple)
{
	/*
	 * @todo small_ptr_compress() is broken
	 * https://github.com/bee/bee/issues/49
	 */
	/* size_t value = small_ptr_compress(tuple); */
	size_t value = (intptr_t) tuple >> 2;
	assert(value_to_tuple(value) == tuple);
	return value;
}

static inline struct tuple *
value_to_tuple(size_t value)
{
	/* return (struct tuple *) salloc_ptr_from_index(value); */
	return (struct tuple *) (value << 2);
}
struct bitset_index_iterator {
	struct iterator base; /* Must be the first member. */
	struct bitset_iterator bitset_it;
};

static struct bitset_index_iterator *
bitset_index_iterator(struct iterator *it)
{
	return (struct bitset_index_iterator *) it;
}

void
bitset_index_iterator_free(struct iterator *iterator)
{
	assert(iterator->free == bitset_index_iterator_free);
	struct bitset_index_iterator *it = bitset_index_iterator(iterator);

	bitset_iterator_destroy(&it->bitset_it);
	free(it);
}

struct tuple *
bitset_index_iterator_next(struct iterator *iterator)
{
	assert(iterator->free == bitset_index_iterator_free);
	struct bitset_index_iterator *it = bitset_index_iterator(iterator);

	size_t value = bitset_iterator_next(&it->bitset_it);
	if (value == SIZE_MAX)
		return NULL;

	return value_to_tuple(value);
}

MemtxBitset::MemtxBitset(struct key_def *key_def)
	: Index(key_def)
{
	assert(!this->key_def->is_unique);

	if (bitset_index_create(&index, realloc) != 0)
		panic_syserror("bitset_index_create");
}

MemtxBitset::~MemtxBitset()
{
	bitset_index_destroy(&index);
}

size_t
MemtxBitset::size() const
{
	return bitset_index_size(&index);
}

size_t
MemtxBitset::bsize() const
{
	return 0;
}

struct iterator *
MemtxBitset::allocIterator() const
{
	struct bitset_index_iterator *it = (struct bitset_index_iterator *)
			malloc(sizeof(*it));
	if (!it)
		return NULL;

	memset(it, 0, sizeof(*it));
	it->base.next = bitset_index_iterator_next;
	it->base.free = bitset_index_iterator_free;

	bitset_iterator_create(&it->bitset_it, realloc);

	return (struct iterator *) it;
}

struct tuple *
MemtxBitset::findByKey(const char *key, uint32_t part_count) const
{
	(void) key;
	(void) part_count;
	tnt_raise(ClientError, ER_UNSUPPORTED, "MemtxBitset", "findByKey()");
	return NULL;
}

static inline const char *
make_key(const char *field, uint32_t *key_len)
{
	static uint64_t u64key;
	switch (mp_typeof(*field)) {
	case MP_UINT:
		u64key = mp_decode_uint(&field);
		*key_len = sizeof(uint64_t);
		return (const char *) &u64key;
		break;
	case MP_STR:
		return mp_decode_str(&field, key_len);
		break;
	default:
		*key_len = 0;
		assert(false);
		return NULL;
	}
}

struct tuple *
MemtxBitset::replace(struct tuple *old_tuple, struct tuple *new_tuple,
		     enum dup_replace_mode mode)
{
	assert(!key_def->is_unique);
	assert(old_tuple != NULL || new_tuple != NULL);
	(void) mode;

	struct tuple *ret = NULL;

	if (old_tuple != NULL) {
		size_t value = tuple_to_value(old_tuple);
		if (bitset_index_contains_value(&index, value)) {
			ret = old_tuple;

			assert(old_tuple != new_tuple);
			bitset_index_remove_value(&index, value);
		}
	}

	if (new_tuple != NULL) {
		const char *field;
		field = tuple_field(new_tuple, key_def->parts[0].fieldno);
		uint32_t key_len;
		const void *key = make_key(field, &key_len);
		size_t value = tuple_to_value(new_tuple);
		if (bitset_index_insert(&index, key, key_len, value) < 0) {
			tnt_raise(ClientError, ER_MEMORY_ISSUE, 0,
				  "MemtxBitset", "insert");
		}
	}

	return ret;
}

void
MemtxBitset::initIterator(struct iterator *iterator, enum iterator_type type,
			  const char *key, uint32_t part_count) const
{
	assert(iterator->free == bitset_index_iterator_free);
	assert(part_count == 0 || key != NULL);
	(void) part_count;

	struct bitset_index_iterator *it = bitset_index_iterator(iterator);

	const void *bitset_key = NULL;
	uint32_t bitset_key_size = 0;

	if (type != ITER_ALL) {
		assert(part_count == 1);
		bitset_key = make_key(key, &bitset_key_size);
	}

	struct bitset_expr expr;
	bitset_expr_create(&expr, realloc);
	try {
		int rc = 0;
		switch (type) {
		case ITER_ALL:
			rc = bitset_index_expr_all(&expr);
			break;
		case ITER_EQ:
			rc = bitset_index_expr_equals(&expr, bitset_key,
						      bitset_key_size);
			break;
		case ITER_BITS_ALL_SET:
			rc = bitset_index_expr_all_set(&expr, bitset_key,
						       bitset_key_size);
			break;
		case ITER_BITS_ALL_NOT_SET:
			rc = bitset_index_expr_all_not_set(&expr, bitset_key,
							   bitset_key_size);
			break;
		case ITER_BITS_ANY_SET:
			rc = bitset_index_expr_any_set(&expr, bitset_key,
						       bitset_key_size);
			break;
		default:
			tnt_raise(ClientError, ER_UNSUPPORTED,
				  "MemtxBitset", "requested iterator type");
		}

		if (rc != 0) {
			tnt_raise(ClientError, ER_MEMORY_ISSUE,
				  0, "MemtxBitset", "iterator expression");
		}

		if (bitset_index_init_iterator((bitset_index *) &index,
					       &it->bitset_it,
					       &expr) != 0) {
			tnt_raise(ClientError, ER_MEMORY_ISSUE,
				  0, "MemtxBitset", "iterator state");
		}

		bitset_expr_destroy(&expr);
	} catch (Exception *e) {
		bitset_expr_destroy(&expr);
		throw;
	}
}

size_t
MemtxBitset::count(enum iterator_type type, const char *key,
		   uint32_t part_count) const
{
	if (type == ITER_ALL)
		return bitset_index_size(&index);

	assert(part_count == 1); /* checked by key_validate() */
	uint32_t bitset_key_size = 0;
	const void *bitset_key = make_key(key, &bitset_key_size);
	struct bit_iterator bit_it;
	size_t bit;
	if (type == ITER_BITS_ANY_SET) {
		/*
		 * Optimization: get the number of items for each requested bit
		 * and then found the maximum.
		 */
		bit_iterator_init(&bit_it, bitset_key, bitset_key_size, true);
		size_t result = 0;
		while ((bit = bit_iterator_next(&bit_it)) != SIZE_MAX) {
			size_t count = bitset_index_count(&index, bit);
			result = MAX(result, count);
		}
		return result;
	} else if (type == ITER_BITS_ALL_SET) {
		/**
		 * Optimization: for an empty key return the number of items
		 * in the index.
		 */
		bit_iterator_init(&bit_it, bitset_key, bitset_key_size, true);
		bit = bit_iterator_next(&bit_it);
		if (bit == SIZE_MAX)
			return bitset_index_size(&index);
		/**
		 * Optimiation: for a single bit key use
		 * bitset_index_count().
		 */
		if (bit_iterator_next(&bit_it) == SIZE_MAX)
			return bitset_index_count(&index, bit);
	} else if (type == ITER_BITS_ALL_NOT_SET) {
		/**
		 * Optimization: for an empty key return the number of items
		 * in the index.
		 */
		bit_iterator_init(&bit_it, bitset_key, bitset_key_size, true);
		bit = bit_iterator_next(&bit_it);
		if (bit == SIZE_MAX)
			return bitset_index_size(&index);
		/**
		 * Optimiation: for the single bit key use
		 * bitset_index_count().
		 */
		if (bit_iterator_next(&bit_it) == SIZE_MAX)
			return bitset_index_size(&index) - bitset_index_count(&index, bit);
	}

	/* Call generic method */
	return Index::count(type, key, part_count);
}
