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
#include "db/lua/slab.h"
#include "lua/utils.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
} /* extern "C" */

#include "db/tuple.h"
#include "small/small.h"
#include "small/quota.h"
#include "memory.h"

/** A callback passed into salloc_stat() and invoked for every slab class. */
extern "C" {

static int
small_stats_lua_cb(const struct mempool_stats *stats, void *cb_ctx)
{
	/** Don't publish information about empty slabs. */
	if (stats->slabcount == 0)
		return 0;

	struct lua_State *L = (struct lua_State *) cb_ctx;

	/*
	 * Create a Lua table for every slab class. A class is
	 * defined by its item size.
	 */
	/** Assign next slab size to the next member of an array. */
	lua_pushnumber(L, lua_objlen(L, -1) + 1);
	lua_newtable(L);
	/**
	 * This is in fact only to force YaML flow "compact" for this
	 * table.
	 */
	luaL_setmaphint(L, -1);

	lua_pushstring(L, "mem_used");
	luaL_pushuint64(L, stats->totals.used);
	lua_settable(L, -3);

	lua_pushstring(L, "slab_size");
	luaL_pushuint64(L, stats->slabsize);
	lua_settable(L, -3);

	lua_pushstring(L, "mem_free");
	luaL_pushuint64(L, stats->totals.total - stats->totals.used);
	lua_settable(L, -3);

	lua_pushstring(L, "item_size");
	luaL_pushuint64(L, stats->objsize);
	lua_settable(L, -3);

	lua_pushstring(L, "slab_count");
	luaL_pushuint64(L, stats->slabcount);
	lua_settable(L, -3);

	lua_pushstring(L, "item_count");
	luaL_pushuint64(L, stats->objcount);
	lua_settable(L, -3);

	lua_settable(L, -3);
	return 0;
}

} /* extern "C" */

static int
ldb_slab_info(struct lua_State *L)
{
	struct small_stats totals;

	lua_newtable(L);
	lua_pushstring(L, "slabs");
	lua_newtable(L);
	small_stats(&memtx_alloc, &totals, small_stats_lua_cb, L);
	lua_settable(L, -3);

	lua_pushstring(L, "arena_used");
	luaL_pushuint64(L, totals.used);
	lua_settable(L, -3);

	lua_pushstring(L, "arena_size");
	luaL_pushuint64(L, totals.total);
	lua_settable(L, -3);

	char value[32];
	double items_used_ratio = 100
		* ((double)totals.used)
		/ ((double)memtx_alloc.cache->arena->prealloc + 0.0001);
	snprintf(value, sizeof(value), "%0.1lf%%", items_used_ratio);
	lua_pushstring(L, "items_used_ratio");
	lua_pushstring(L, value);
	lua_settable(L, -3);

	double arena_used_ratio = 100
		* ((double)memtx_alloc.cache->arena->used)
		/ ((double)memtx_alloc.cache->arena->prealloc + 0.0001);
	snprintf(value, sizeof(value), "%0.1lf%%", arena_used_ratio);
	lua_pushstring(L, "arena_used_ratio");
	lua_pushstring(L, value);
	lua_settable(L, -3);

	return 1;
}

static int
ldb_runtime_info(struct lua_State *L)
{
	lua_newtable(L);

	lua_pushstring(L, "used");
	luaL_pushuint64(L, runtime.used);
	lua_settable(L, -3);

	lua_pushstring(L, "maxalloc");
	luaL_pushuint64(L, quota_get(runtime.quota));
	lua_settable(L, -3);

	return 1;
}

static int
ldb_slab_check(struct lua_State *L __attribute__((unused)))
{
	slab_cache_check(memtx_alloc.cache);
	return 0;
}

/** Initialize db.slab package. */
void
db_lua_slab_init(struct lua_State *L)
{
	lua_getfield(L, LUA_GLOBALSINDEX, "db");
	lua_pushstring(L, "slab");
	lua_newtable(L);

	lua_pushstring(L, "info");
	lua_pushcfunction(L, ldb_slab_info);
	lua_settable(L, -3);

	lua_pushstring(L, "check");
	lua_pushcfunction(L, ldb_slab_check);
	lua_settable(L, -3);

	lua_settable(L, -3); /* db.slab */

	lua_pushstring(L, "runtime");
	lua_newtable(L);

	lua_pushstring(L, "info");
	lua_pushcfunction(L, ldb_runtime_info);
	lua_settable(L, -3);

	lua_settable(L, -3); /* db.runtime */

	lua_pop(L, 1); /* db. */
}
