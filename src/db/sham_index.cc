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
#include "sham_index.h"
#include "say.h"
#include "tuple.h"
#include "scoped_guard.h"
#include "errinj.h"
#include "schema.h"
#include "space.h"
#include "txn.h"
#include "cfg.h"
#include "sham_engine.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sham.h>
#include <stdio.h>
#include <inttypes.h>

enum sham_op {
	SHAM_SET,
	SHAM_DELETE
};

static inline void
sham_write(void *env, void *db, void *tx, enum sham_op op,
             struct key_def *key_def,
             struct tuple *tuple)
{
	const char *key = tuple_field(tuple, key_def->parts[0].fieldno);
	const char *keyptr = key;
	mp_next(&keyptr);
	size_t keysize = keyptr - key;
	void *o = sp_object(db);
	if (o == NULL)
		sham_raise(env);
	sp_set(o, "key", key, keysize);
	sp_set(o, "value", tuple->data, tuple->bsize);
	int rc;
	switch (op) {
	case SHAM_DELETE:
		rc = sp_delete(tx, o);
		break;
	case SHAM_SET:
		rc = sp_set(tx, o);
		break;
	}
	if (rc == -1)
		sham_raise(env);
}

static struct tuple*
sham_read(void *env, void *db, void *tx, const char *key, size_t keysize,
            struct tuple_format *format)
{
	void *o = sp_object(db);
	if (o == NULL)
		sham_raise(env);
	sp_set(o, "key", key, keysize);
	void *result = sp_get((tx) ? tx: db, o);
	if (result == NULL)
		return NULL;
	auto scoped_guard =
		make_scoped_guard([=] { sp_destroy(result); });
	int valuesize = 0;
	void *value = sp_get(result, "value", &valuesize);
	return tuple_new(format, (char*)value, (char*)value + valuesize);
}

struct sham_format {
	uint32_t offset;
	uint16_t size;
} __attribute__((packed));

static inline int
sham_compare(char *a, size_t asz __attribute__((unused)),
               char *b, size_t bsz __attribute__((unused)),
               void *arg)
{
	struct key_def *key_def = (struct key_def*)arg;
	a = a + ((struct sham_format*)a)[0].offset;
	b = b + ((struct sham_format*)b)[0].offset;
	int rc = tuple_compare_field(a, b, key_def->parts[0].type);
	return (rc == 0) ? 0 :
	       ((rc > 0) ? 1 : -1);
}

static inline void*
sham_configure(struct space *space, struct key_def *key_def)
{
	ShamEngine *engine =
		(ShamEngine*)space->handler->engine;
	void *env = engine->env;
	void *c = sp_ctl(env);
	char pointer[128];
	char pointer_arg[128];
	char name[128];
	snprintf(name, sizeof(name), "%" PRIu32, key_def->space_id);
	sp_set(c, "db", name);
	snprintf(name, sizeof(name), "db.%" PRIu32 ".format",
	         key_def->space_id);
	sp_set(c, name, "document");
	snprintf(name, sizeof(name), "db.%" PRIu32 ".index.cmp",
	         key_def->space_id);
	snprintf(pointer, sizeof(pointer), "pointer: %p", (void*)sham_compare);
	snprintf(pointer_arg, sizeof(pointer_arg), "pointer: %p", (void*)key_def);
	sp_set(c, name, pointer, pointer_arg);
	snprintf(name, sizeof(name), "db.%" PRIu32 ".compression", key_def->space_id);
	sp_set(c, name, cfg_gets("sham.compression"));
	snprintf(name, sizeof(name), "db.%" PRIu32, key_def->space_id);
	void *db = sp_get(c, name);
	if (db == NULL)
		sham_raise(env);
	return db;
}

ShamIndex::ShamIndex(struct key_def *key_def_arg __attribute__((unused)))
	: Index(key_def_arg)
{
	struct space *space = space_cache_find(key_def->space_id);
	ShamEngine *engine =
		(ShamEngine*)space->handler->engine;
	env = engine->env;
	db = sham_configure(space, key_def);
	if (db == NULL)
		sham_raise(env);
	/* start two-phase recovery for a space:
	 * a. created after snapshot recovery
	 * b. created during log recovery
	*/
	int rc = sp_open(db);
	if (rc == -1)
		sham_raise(env);
	tuple_format_ref(space->format, 1);
}

ShamIndex::~ShamIndex()
{
	if (m_position != NULL) {
		m_position->free(m_position);
		m_position = NULL;
	}
	if (db) {
		int rc = sp_destroy(db);
		if (rc == 0)
			return;
		void *c = sp_ctl(env);
		void *o = sp_get(c, "sham.error");
		char *error = (char *)sp_get(o, "value", NULL);
		say_info("sham space %d close error: %s",
		         key_def->space_id, error);
		sp_destroy(o);
	}
}

size_t
ShamIndex::size() const
{
	void *c = sp_ctl(env);
	char name[128];
	snprintf(name, sizeof(name), "db.%" PRIu32 ".index.count",
	         key_def->space_id);
	void *o = sp_get(c, name);
	if (o == NULL)
		sham_raise(env);
	uint64_t count = atoi((const char *)sp_get(o, "value", NULL));
	sp_destroy(o);
	return count;
}

size_t
ShamIndex::bsize() const
{
	void *c = sp_ctl(env);
	char name[128];
	snprintf(name, sizeof(name), "db.%" PRIu32 ".index.memory_used",
	         key_def->space_id);
	void *o = sp_get(c, name);
	if (o == NULL)
		sham_raise(env);
	uint64_t used = atoi((const char *)sp_get(o, "value", NULL));
	sp_destroy(o);
	return used;
}

struct tuple *
ShamIndex::findByKey(const char *key, uint32_t part_count) const
{
	(void) part_count;
	assert(part_count == 1);
	assert(key_def->is_unique && part_count == key_def->part_count);
	const char *keyptr = key;
	mp_next(&keyptr);
	size_t keysize = keyptr - key;
	struct space *space = space_cache_find(key_def->space_id);
	void *tx = in_txn() ? in_txn()->engine_tx : NULL;
	return sham_read(env, db, tx, key, keysize, space->format);
}

struct tuple *
ShamIndex::replace(struct tuple *old_tuple, struct tuple *new_tuple,
                     enum dup_replace_mode mode)
{
	struct space *space = space_cache_find(key_def->space_id);
	struct txn *txn = in_txn();
	assert(txn != NULL && txn->engine_tx != NULL);
	void *tx = txn->engine_tx;

	/* This method does not return old tuple for replace,
	 * insert or update.
	 *
	 * Delete does return old tuple to be properly
	 * scheduled for wal write.
	 */

	/* Switch from INSERT to REPLACE during recovery.
	 *
	 * Database might hold newer key version than currenly
	 * recovered log record.
	 */
	if (mode == DUP_INSERT) {
		ShamEngine *engine = (ShamEngine*)space->handler->engine;
		if (! engine->recovery_complete)
			mode = DUP_REPLACE_OR_INSERT;
	}

	/* delete */
	if (old_tuple && new_tuple == NULL) {
		sham_write(env, db, tx, SHAM_DELETE, key_def, old_tuple);
		return old_tuple;
	}

	/* update */
	if (old_tuple && new_tuple) {
		/* assume no primary key update is supported */
		sham_write(env, db, tx, SHAM_SET, key_def, new_tuple);
		return NULL;
	}

	/* insert or replace */
	switch (mode) {
	case DUP_INSERT: {
		const char *key = tuple_field(new_tuple, key_def->parts[0].fieldno);
		const char *keyptr = key;
		mp_next(&keyptr);
		size_t keysize = keyptr - key;
		struct tuple *dup_tuple =
			sham_read(env, db, tx, key, keysize, space->format);
		if (dup_tuple) {
			int error = tuple_compare(dup_tuple, new_tuple, key_def) == 0;
			tuple_delete(dup_tuple);
			if (error) {
				struct space *sp =
					space_cache_find(key_def->space_id);
				tnt_raise(ClientError, ER_TUPLE_FOUND,
					  index_name(this), space_name(sp));
			}
		}
	}
	case DUP_REPLACE_OR_INSERT:
		sham_write(env, db, tx, SHAM_SET, key_def, new_tuple);
		break;
	case DUP_REPLACE:
	default:
		assert(0);
		break;
	}
	return NULL;
}

struct sham_iterator {
	struct iterator base;
	const char *key;
	int keysize;
	uint32_t part_count;
	struct space *space;
	void *env;
	void *db;
	void *cursor;
	void *tx;
};

void
sham_iterator_free(struct iterator *ptr)
{
	assert(ptr->free == sham_iterator_free);
	struct sham_iterator *it = (struct sham_iterator *) ptr;
	if (it->cursor)
		sp_destroy(it->cursor);
	free(ptr);
}

void
sham_iterator_close(struct iterator *ptr)
{
	assert(ptr->free == sham_iterator_free);
	struct sham_iterator *it = (struct sham_iterator *) ptr;
	if (it->cursor) {
		sp_destroy(it->cursor);
		it->cursor = NULL;
	}
}

struct tuple *
sham_iterator_next(struct iterator *ptr)
{
	assert(ptr->next == sham_iterator_next);
	struct sham_iterator *it = (struct sham_iterator *) ptr;
	assert(it->cursor != NULL);
	void *o = sp_get(it->cursor);
	if (o == NULL)
		return NULL;
	int valuesize = 0;
	const char *value = (const char*)sp_get(o, "value", &valuesize);
	return tuple_new(it->space->format, value, value + valuesize);
}

struct tuple *
sham_iterator_last(struct iterator *ptr __attribute__((unused)))
{
	return NULL;
}

struct tuple *
sham_iterator_eq(struct iterator *ptr)
{
	ptr->next = sham_iterator_last;
	struct sham_iterator *it = (struct sham_iterator *) ptr;
	assert(it->cursor == NULL);
	return sham_read(it->env, it->db, it->tx, it->key, it->keysize,
	                   it->space->format);
}

struct iterator *
ShamIndex::allocIterator() const
{
	struct sham_iterator *it =
		(struct sham_iterator *) calloc(1, sizeof(*it));
	if (it == NULL) {
		tnt_raise(ClientError, ER_MEMORY_ISSUE,
		          sizeof(struct sham_iterator), "ShamIndex",
		          "iterator");
	}
	it->base.next  = sham_iterator_next;
	it->base.close = sham_iterator_close;
	it->base.free  = sham_iterator_free;
	it->cursor = NULL;
	return (struct iterator *) it;
}

void
ShamIndex::initIterator(struct iterator *ptr,
                          enum iterator_type type,
                          const char *key, uint32_t part_count) const
{
	struct sham_iterator *it = (struct sham_iterator *) ptr;
	assert(it->cursor == NULL);
	size_t keysize;
	if (part_count > 0) {
		const char *keyptr = key;
		mp_next(&keyptr);
		keysize = keyptr - key;
	} else {
		keysize = 0;
		key = NULL;
	}
	it->key = key;
	it->keysize = keysize;
	it->part_count = part_count;
	it->env = env;
	it->db = db;
	it->space = space_cache_find(key_def->space_id);
	it->tx = NULL;
	const char *compare;
	switch (type) {
	case ITER_EQ:
		it->base.next = sham_iterator_eq;
		it->tx = in_txn() ? in_txn()->engine_tx : NULL;
		return;
	case ITER_ALL:
	case ITER_GE: compare = ">=";
		break;
	case ITER_GT: compare = ">";
		break;
	case ITER_LE: compare = "<=";
		break;
	case ITER_LT: compare = "<";
		break;
	default:
		tnt_raise(ClientError, ER_UNSUPPORTED,
		          "ShamIndex", "requested iterator type");
	}
	it->base.next = sham_iterator_next;
	void *o = sp_object(db);
	if (o == NULL)
		sham_raise(env);
	sp_set(o, "order", compare);
	if (key)
		sp_set(o, "key", key, keysize);
	it->cursor = sp_cursor(db, o);
	if (it->cursor == NULL)
		sham_raise(env);
}
