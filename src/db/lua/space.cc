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
#include "db/lua/space.h"
#include "db/lua/tuple.h"
#include "lua/utils.h"
#include "lua/trigger.h"

extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
} /* extern "C" */

#include "db/space.h"
#include "db/schema.h"
#include "db/tuple.h"
#include "db/txn.h"

/**
 * Trigger function for all spaces
 */
static void
ldb_space_on_replace_trigger(struct trigger *trigger, void *event)
{
	struct txn_stmt *stmt = txn_stmt((struct txn *) event);
	lua_State *L = lua_newthread(bee_L);
	LuarefGuard coro_guard(bee_L);

	lua_rawgeti(L, LUA_REGISTRYINDEX, (intptr_t) trigger->data);

	ldb_pushtuple(L, stmt->old_tuple);
	ldb_pushtuple(L, stmt->new_tuple);
	/* @todo: maybe the space object has to be here */
	lua_pushstring(L, stmt->space->def.name);

	ldb_call(L, 3, 0);
}

/**
 * Set/Reset/Get space.on_replace trigger
 */
static int
ldb_space_on_replace(struct lua_State *L)
{
	int top = lua_gettop(L);

	if (top < 1 || !lua_istable(L, 1)) {
		luaL_error(L,
	   "usage: space:on_replace(function | nil, [function | nil])");
	}
	lua_getfield(L, 1, "id"); /* Get space id. */
	uint32_t id = lua_tointeger(L, lua_gettop(L));
	struct space *space = space_cache_find(id);
	lua_pop(L, 1);

	return ldb_trigger_reset(L, 3,
				  &space->on_replace,
				  ldb_space_on_replace_trigger);
}

/**
 * Make a single space available in Lua,
 * via db.space[] array.
 *
 * @return A new table representing a space on top of the Lua
 * stack.
 */
static void
ldb_fillspace(struct lua_State *L, struct space *space, int i)
{
	/* space.arity */
	lua_pushstring(L, "field_count");
	lua_pushnumber(L, space->def.field_count);
	lua_settable(L, i);

	/* space.n */
	lua_pushstring(L, "id");
	lua_pushnumber(L, space_id(space));
	lua_settable(L, i);

	/* space.is_temp */
	lua_pushstring(L, "temporary");
	lua_pushboolean(L, space_is_temporary(space));
	lua_settable(L, i);

	/* space.name */
	lua_pushstring(L, "name");
	lua_pushstring(L, space_name(space));
	lua_settable(L, i);

	/* space.engine */
	lua_pushstring(L, "engine");
	lua_pushstring(L, space->def.engine_name);
	lua_settable(L, i);

	lua_pushstring(L, "enabled");
	lua_pushboolean(L, space_index(space, 0) != 0);
	lua_settable(L, i);


        /* space:on_replace */
        lua_pushstring(L, "on_replace");
        lua_pushcfunction(L, ldb_space_on_replace);
        lua_settable(L, i);

	lua_getfield(L, i, "index");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		/* space.index */
		lua_pushstring(L, "index");
		lua_newtable(L);
		lua_settable(L, i);	/* push space.index */
		lua_getfield(L, i, "index");
	} else {
		/* Empty the table. */
		lua_pushnil(L);  /* first key */
		while (lua_next(L, -2) != 0) {
			lua_pop(L, 1); /* remove the value. */
			lua_pushnil(L); /* set the key to nil. */
			lua_settable(L, -3);
			lua_pushnil(L); /* start over. */
		}
	}
	/*
	 * Fill space.index table with
	 * all defined indexes.
	 */
	for (int k = 0; k <= space->index_id_max; k++) {
		Index *index = space_index(space, k);
		if (index == NULL)
			continue;
		struct key_def *key_def = index->key_def;
		lua_pushnumber(L, key_def->iid);
		lua_newtable(L);		/* space.index[k] */

		lua_pushboolean(L, key_def->is_unique);
		lua_setfield(L, -2, "unique");

		lua_pushstring(L, index_type_strs[key_def->type]);
		lua_setfield(L, -2, "type");

		lua_pushnumber(L, key_def->iid);
		lua_setfield(L, -2, "id");

		lua_pushnumber(L, space->def.id);
		lua_setfield(L, -2, "space_id");

		lua_pushstring(L, key_def->name);
		lua_setfield(L, -2, "name");

		lua_pushstring(L, "parts");
		lua_newtable(L);

		for (uint32_t j = 0; j < key_def->part_count; j++) {
			lua_pushnumber(L, j + 1);
			lua_newtable(L);

			lua_pushstring(L,
			       field_type_strs[key_def->parts[j].type]);
			lua_setfield(L, -2, "type");

			lua_pushnumber(L, key_def->parts[j].fieldno + 1);
			lua_setfield(L, -2, "fieldno");

			lua_settable(L, -3); /* index[k].parts[j] */
		}

		lua_settable(L, -3); /* space.index[k].parts */

		lua_settable(L, -3); /* space.index[k] */
		lua_rawgeti(L, -1, key_def->iid);
		lua_setfield(L, -2, key_def->name);
	}

	lua_pop(L, 1); /* pop the index field */

	lua_getfield(L, LUA_GLOBALSINDEX, "db");
	lua_pushstring(L, "schema");
	lua_gettable(L, -2);
	lua_pushstring(L, "space");
	lua_gettable(L, -2);
	lua_pushstring(L, "bless");
	lua_gettable(L, -2);

	lua_pushvalue(L, i);	/* space */
	lua_call(L, 1, 0);
	lua_pop(L, 3);	/* cleanup stack - db, schema, space */
}

/** Export a space to Lua */
void
db_lua_space_new(struct lua_State *L, struct space *space)
{
	lua_getfield(L, LUA_GLOBALSINDEX, "db");
	lua_getfield(L, -1, "space");

	if (!lua_istable(L, -1)) {
		lua_pop(L, 1); /* pop nil */
		lua_newtable(L);
		lua_setfield(L, -2, "space");
		lua_getfield(L, -1, "space");
	}
	lua_rawgeti(L, -1, space_id(space));
	if (lua_isnil(L, -1)) {
		/*
		 * If the space already exists, modify it, rather
		 * than create a new one -- to not invalidate
		 * Lua variable references to old space outside
		 * the db.space[].
		 */
		lua_pop(L, 1);
		lua_newtable(L);
		lua_rawseti(L, -2, space_id(space));
		lua_rawgeti(L, -1, space_id(space));
	} else {
		/* Clear the reference to old space by old name. */
		lua_getfield(L, -1, "name");
		lua_pushnil(L);
		lua_settable(L, -4);
	}
	ldb_fillspace(L, space, lua_gettop(L));
	lua_setfield(L, -2, space_name(space));

	lua_pop(L, 2); /* db, space */
}

/** Delete a given space in Lua */
void
db_lua_space_delete(struct lua_State *L, uint32_t id)
{
	lua_getfield(L, LUA_GLOBALSINDEX, "db");
	lua_getfield(L, -1, "space");
	lua_rawgeti(L, -1, id);
	lua_getfield(L, -1, "name");
	lua_pushnil(L);
	lua_rawset(L, -4);
	lua_pop(L, 1); /* pop space */

	lua_pushnil(L);
	lua_rawseti(L, -2, id);
	lua_pop(L, 2); /* db, space */
}


void
db_lua_space_init(struct lua_State *L)
{
	lua_getfield(L, LUA_GLOBALSINDEX, "db");
	lua_newtable(L);
	lua_setfield(L, -2, "schema");
	lua_getfield(L, -1, "schema");
	lua_pushnumber(L, SC_SCHEMA_ID);
	lua_setfield(L, -2, "SCHEMA_ID");
	lua_pushnumber(L, SC_SPACE_ID);
	lua_setfield(L, -2, "SPACE_ID");
	lua_pushnumber(L, SC_VSPACE_ID);
	lua_setfield(L, -2, "VSPACE_ID");
	lua_pushnumber(L, SC_INDEX_ID);
	lua_setfield(L, -2, "INDEX_ID");
	lua_pushnumber(L, SC_VINDEX_ID);
	lua_setfield(L, -2, "VINDEX_ID");
	lua_pushnumber(L, SC_USER_ID);
	lua_setfield(L, -2, "USER_ID");
	lua_pushnumber(L, SC_VUSER_ID);
	lua_setfield(L, -2, "VUSER_ID");
	lua_pushnumber(L, SC_FUNC_ID);
	lua_setfield(L, -2, "FUNC_ID");
	lua_pushnumber(L, SC_VFUNC_ID);
	lua_setfield(L, -2, "VFUNC_ID");
	lua_pushnumber(L, SC_PRIV_ID);
	lua_setfield(L, -2, "PRIV_ID");
	lua_pushnumber(L, SC_VPRIV_ID);
	lua_setfield(L, -2, "VPRIV_ID");
	lua_pushnumber(L, SC_CLUSTER_ID);
	lua_setfield(L, -2, "CLUSTER_ID");
	lua_pushnumber(L, SC_SYSTEM_ID_MIN);
	lua_setfield(L, -2, "SYSTEM_ID_MIN");
	lua_pushnumber(L, SC_SYSTEM_ID_MAX);
	lua_setfield(L, -2, "SYSTEM_ID_MAX");
	lua_pushnumber(L, DB_INDEX_MAX);
	lua_setfield(L, -2, "INDEX_MAX");
	lua_pushnumber(L, DB_SPACE_MAX);
	lua_setfield(L, -2, "SPACE_MAX");
	lua_pushnumber(L, DB_FIELD_MAX);
	lua_setfield(L, -2, "FIELD_MAX");
	lua_pushnumber(L, DB_INDEX_FIELD_MAX);
	lua_setfield(L, -2, "INDEX_FIELD_MAX");
	lua_pushnumber(L, DB_INDEX_PART_MAX);
	lua_setfield(L, -2, "INDEX_PART_MAX");
	lua_pushnumber(L, DB_NAME_MAX);
	lua_setfield(L, -2, "NAME_MAX");
	lua_pushnumber(L, FORMAT_ID_MAX);
	lua_setfield(L, -2, "FORMAT_ID_MAX");
	lua_pop(L, 2); /* db, schema */
}
