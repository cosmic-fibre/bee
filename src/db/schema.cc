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
#include "schema.h"
#include "user_def.h"
#include "engine.h"
#include "space.h"
#include "tuple.h"
#include "assoc.h"
#include "lua/utils.h"
#include "lua/space.h"
#include "key_def.h"
#include "alter.h"
#include "scoped_guard.h"
#include <stdio.h>
/**
 * @module Data Dictionary
 *
 * The data dictionary is responsible for storage and caching
 * of system metadata, such as information about existing
 * spaces, indexes, tuple formats. Space and index metadata
 * is called in dedicated spaces, _space and _index respectively.
 * The contents of these spaces is fully cached in a cache of
 * struct space objects.
 *
 * struct space is an in-memory instance representing a single
 * space with its metadata, space data, and methods to manage
 * it.
 */

/** All existing spaces. */
static struct mh_i32ptr_t *spaces;
static struct mh_i32ptr_t *funcs;
int sc_version;

bool
space_is_system(struct space *space)
{
	return space->def.id > SC_SYSTEM_ID_MIN &&
		space->def.id < SC_SYSTEM_ID_MAX;
}

/** Return space by its number */
extern "C" struct space *
space_by_id(uint32_t id)
{
	mh_int_t space = mh_i32ptr_find(spaces, id, NULL);
	if (space == mh_end(spaces))
		return NULL;
	return (struct space *) mh_i32ptr_node(spaces, space)->val;
}

extern "C" const char *
space_name_by_id(uint32_t id)
{
	struct space *space = space_by_id(id);
	return space ? space_name(space) : "";
}

/**
 * Visit all spaces and apply 'func'.
 */
void
space_foreach(void (*func)(struct space *sp, void *udata), void *udata)
{
	mh_int_t i;
	struct space *space;
	char key[6];
	assert(mp_sizeof_uint(SC_SYSTEM_ID_MIN) <= sizeof(key));
	mp_encode_uint(key, SC_SYSTEM_ID_MIN);

	/*
	 * Make sure we always visit system spaces first,
	 * in order from lowest space id to the highest..
	 * This is essential for correctly recovery from the
	 * snapshot, and harmless otherwise.
	 */
	space = space_by_id(SC_SPACE_ID);
	Index *pk = space ? space_index(space, 0) : NULL;
	if (pk) {
		struct iterator *it = pk->allocIterator();
		auto scoped_guard = make_scoped_guard([=] { it->free(it); });
		pk->initIterator(it, ITER_GE, key, 1);
		IteratorGuard it_guard(it);

		struct tuple *tuple;
		while ((tuple = it->next(it))) {
			/* Get space id, primary key, field 0. */
			uint32_t id = tuple_field_u32(tuple, 0);
			space = space_cache_find(id);
			if (! space_is_system(space))
				break;
			func(space, udata);
		}
	}

	mh_foreach(spaces, i) {
		space = (struct space *) mh_i32ptr_node(spaces, i)->val;
		if (space_is_system(space))
			continue;
		func(space, udata);
	}
}

/** Delete a space from the space cache and Lua. */
struct space *
space_cache_delete(uint32_t id)
{
	if (bee_L)
		db_lua_space_delete(bee_L, id);
	mh_int_t k = mh_i32ptr_find(spaces, id, NULL);
	assert(k != mh_end(spaces));
	struct space *space = (struct space *)mh_i32ptr_node(spaces, k)->val;
	mh_i32ptr_del(spaces, k, NULL);
	sc_version++;
	return space;
}

/**
 * Update the space in the space cache and in Lua. Returns
 * the old space instance, if any, or NULL if it's a new space.
 */
struct space *
space_cache_replace(struct space *space)
{
	const struct mh_i32ptr_node_t node = { space_id(space), space };
	struct mh_i32ptr_node_t old, *p_old = &old;
	mh_int_t k = mh_i32ptr_put(spaces, &node, &p_old, NULL);
	if (k == mh_end(spaces)) {
		panic_syserror("Out of memory for the data "
			       "dictionary cache.");
	}
	sc_version++;
	/*
	 * Must be after the space is put into the hash, since
	 * db.schema.space.bless() uses hash look up to find the
	 * space and create userdata objects for space objects.
	 */
	db_lua_space_new(bee_L, space);
	return p_old ? (struct space *) p_old->val : NULL;
}

/** A wrapper around space_new() for data dictionary spaces. */
struct space *
sc_space_new(struct space_def *space_def,
	     struct key_def *key_def,
	     struct trigger *trigger)
{
	struct rlist key_list;
	rlist_create(&key_list);
	rlist_add_entry(&key_list, key_def, link);
	struct space *space = space_new(space_def, &key_list);
	(void) space_cache_replace(space);
	if (trigger)
		trigger_add(&space->on_replace, trigger);
	/*
	 * Data dictionary spaces are fully built since:
	 * - they contain data right from the start
	 * - they are fully operable already during recovery
	 * - if there is a record in the snapshot which mandates
	 *   addition of a new index to a system space, this
	 *   index is built tuple-by-tuple, not in bulk, which
	 *   ensures validation of tuples when starting from
	 *   a snapshot of older version.
	 */
	space->handler->engine->initSystemSpace(space);
	return space;
}

uint32_t
schema_find_id(uint32_t system_space_id, uint32_t index_id,
	       const char *name, uint32_t len)
{
	struct space *space = space_cache_find(system_space_id);
	Index *index = index_find(space, index_id);
	struct iterator *it = index->position();
	char key[5 /* str len */ + DB_NAME_MAX];
	mp_encode_str(key, name, len);
	index->initIterator(it, ITER_EQ, key, 1);
	IteratorGuard it_guard(it);

	struct tuple *tuple;
	while ((tuple = it->next(it))) {
		/* id is always field #1 */
		return tuple_field_u32(tuple, 0);
	}
	return SC_ID_NIL;
}

/**
 * Initialize a prototype for the two mandatory data
 * dictionary spaces and create a cache entry for them.
 * When restoring data from the snapshot these spaces
 * will get altered automatically to their actual format.
 */
void
schema_init()
{
	/* Initialize the space cache. */
	spaces = mh_i32ptr_new();
	funcs = mh_i32ptr_new();
	/*
	 * Create surrogate space objects for the mandatory system
	 * spaces (the primal eggs from which we get all the
	 * chicken). Their definitions will be overwritten by the
	 * data in the snapshot, and they will thus be
	 * *re-created* during recovery.  Note, the index type
	 * must be TREE and space identifiers must be the smallest
	 * one to ensure that these spaces are always recovered
	 * (and re-created) first.
	 */
	/* _schema - key/value space with schema description */
	struct space_def def = {
		SC_SCHEMA_ID, ADMIN, 0, "_schema", "memtx", false
	};
	struct key_def *key_def = key_def_new(def.id,
					      0 /* index id */,
					      "primary", /* name */
					      TREE /* index type */,
					      true /* unique */,
					      1 /* part count */);
	key_def_set_part(key_def, 0 /* part no */, 0 /* field no */, STRING);
	(void) sc_space_new(&def, key_def, &on_replace_schema);

	/* _space - home for all spaces. */
	key_def->space_id = def.id = SC_SPACE_ID;
	snprintf(def.name, sizeof(def.name), "_space");
	key_def_set_part(key_def, 0 /* part no */, 0 /* field no */, NUM);

	(void) sc_space_new(&def, key_def, &alter_space_on_replace_space);

	/* _user - all existing users */
	key_def->space_id = def.id = SC_USER_ID;
	snprintf(def.name, sizeof(def.name), "_user");
	(void) sc_space_new(&def, key_def, &on_replace_user);

	/* _func - all executable objects on which one can have grants */
	key_def->space_id = def.id = SC_FUNC_ID;
	snprintf(def.name, sizeof(def.name), "_func");
	(void) sc_space_new(&def, key_def, &on_replace_func);
	/*
	 * _priv - association user <-> object
	 * The real index is defined in the snapshot.
	 */
	key_def->space_id = def.id = SC_PRIV_ID;
	snprintf(def.name, sizeof(def.name), "_priv");
	(void) sc_space_new(&def, key_def, &on_replace_priv);
	/*
	 * _cluster - association server uuid <-> server id
	 * The real index is defined in the snapshot.
	 */
	key_def->space_id = def.id = SC_CLUSTER_ID;
	snprintf(def.name, sizeof(def.name), "_cluster");
	(void) sc_space_new(&def, key_def, &on_replace_cluster);
	key_def_delete(key_def);

	/* _index - definition of indexes in all spaces */
	def.id = SC_INDEX_ID;
	snprintf(def.name, sizeof(def.name), "_index");
	key_def = key_def_new(def.id,
			      0 /* index id */,
			      "primary",
			      TREE /* index type */,
			      true /* unique */,
			      2 /* part count */);
	/* space no */
	key_def_set_part(key_def, 0 /* part no */, 0 /* field no */, NUM);
	/* index no */
	key_def_set_part(key_def, 1 /* part no */, 1 /* field no */, NUM);
	(void) sc_space_new(&def, key_def, &alter_space_on_replace_index);
	key_def_delete(key_def);
}

void
schema_free(void)
{
	if (spaces == NULL)
		return;
	while (mh_size(spaces) > 0) {
		mh_int_t i = mh_first(spaces);

		struct space *space = (struct space *)
				mh_i32ptr_node(spaces, i)->val;
		space_cache_delete(space_id(space));
		space_delete(space);
	}
	mh_i32ptr_delete(spaces);
	while (mh_size(funcs) > 0) {
		mh_int_t i = mh_first(funcs);

		struct func_def *func = (struct func_def *)
				mh_i32ptr_node(funcs, i)->val;
		func_cache_delete(func->fid);
	}
	mh_i32ptr_delete(funcs);
}

void
func_cache_replace(struct func_def *func)
{
	struct func_def *old = func_by_id(func->fid);
	if (old) {
		*old = *func;
		return;
	}
	if (mh_size(funcs) >= DB_FUNCTION_MAX)
		tnt_raise(ClientError, ER_FUNCTION_MAX, DB_FUNCTION_MAX);
	void *ptr = malloc(sizeof(*func));
	if (ptr == NULL) {
error:
		panic_syserror("Out of memory for the data "
			       "dictionary cache.");
	}
	memcpy(ptr, func, sizeof(*func));
	func = (struct func_def *) ptr;
	const struct mh_i32ptr_node_t node = { func->fid, func };
	mh_int_t k = mh_i32ptr_put(funcs, &node, NULL, NULL);
	if (k == mh_end(funcs))
		goto error;
}

void
func_cache_delete(uint32_t fid)
{
	mh_int_t k = mh_i32ptr_find(funcs, fid, NULL);
	if (k == mh_end(funcs))
		return;
	struct func_def *func = (struct func_def *)
		mh_i32ptr_node(funcs, k)->val;
	mh_i32ptr_del(funcs, k, NULL);
	free(func);
}

struct func_def *
func_by_id(uint32_t fid)
{
	mh_int_t func = mh_i32ptr_find(funcs, fid, NULL);
	if (func == mh_end(funcs))
		return NULL;
	return (struct func_def *) mh_i32ptr_node(funcs, func)->val;
}

bool
schema_find_grants(const char *type, uint32_t id)
{
	struct space *priv = space_cache_find(SC_PRIV_ID);
	/** "object" index */
	Index *index = index_find(priv, 2);
	struct iterator *it = index->position();
	char key[10 + DB_NAME_MAX];
	mp_encode_uint(mp_encode_str(key, type, strlen(type)), id);
	index->initIterator(it, ITER_EQ, key, 2);
	IteratorGuard it_guard(it);
	return it->next(it);
}
