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

#include "info.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
} /* extern "C" */

#include "db/replica.h"
#include "db/recovery.h"
#include "db/cluster.h"
#include "main.h"
#include "db/db.h"
#include "lua/utils.h"
#include "fiber.h"

static int
ldb_info_replication(struct lua_State *L)
{
	struct recovery_state *r = recovery;

	lua_newtable(L);

	lua_pushstring(L, "status");
	lua_pushstring(L, r->remote.status);
	lua_settable(L, -3);

	if (r->remote.reader) {
		lua_pushstring(L, "lag");
		lua_pushnumber(L, r->remote.lag);
		lua_settable(L, -3);

		lua_pushstring(L, "idle");
		lua_pushnumber(L, ev_now(loop()) - r->remote.last_row_time);
		lua_settable(L, -3);

		if (r->remote.reader->exception) {
			lua_pushstring(L, "message");
			lua_pushstring(L,
				       r->remote.reader->exception->errmsg());
			lua_settable(L, -3);
		}
	}

	return 1;
}

static int
ldb_info_server(struct lua_State *L)
{
	lua_createtable(L, 0, 2);
	lua_pushliteral(L, "id");
	lua_pushinteger(L, recovery->server_id);
	lua_settable(L, -3);
	lua_pushliteral(L, "uuid");
	lua_pushlstring(L, tt_uuid_str(&recovery->server_uuid), UUID_STR_LEN);
	lua_settable(L, -3);
	lua_pushliteral(L, "lsn");
	luaL_pushint64(L, vclock_get(&recovery->vclock, recovery->server_id));
	lua_settable(L, -3);
	lua_pushliteral(L, "ro");
	lua_pushboolean(L, db_is_ro());
	lua_settable(L, -3);

	return 1;
}

static int
ldb_info_vclock(struct lua_State *L)
{
	lua_createtable(L, 0, vclock_size(&recovery->vclock));
	/* Request compact output flow */
	luaL_setmaphint(L, -1);
	struct vclock_iterator it;
	vclock_iterator_init(&it, &recovery->vclock);
	vclock_foreach(&it, server) {
		lua_pushinteger(L, server.id);
		luaL_pushuint64(L, server.lsn);
		lua_settable(L, -3);
	}

	return 1;
}

static int
ldb_info_status(struct lua_State *L)
{
	lua_pushstring(L, db_status());
	return 1;
}

static int
ldb_info_uptime(struct lua_State *L)
{
	lua_pushnumber(L, (unsigned)bee_uptime() + 1);
	return 1;
}

static int
ldb_info_snapshot_pid(struct lua_State *L)
{
	lua_pushnumber(L, snapshot_pid);
	return 1;
}

static int
ldb_info_pid(struct lua_State *L)
{
	lua_pushnumber(L, getpid());
	return 1;
}

#if 0
void sham_info(void (*)(const char*, const char*, void*), void*);

static void
ldb_info_sham_cb(const char *key, const char *value, void *arg)
{
	struct lua_State *L;
	L = (struct lua_State*)arg;
	if (value == NULL)
		return;
	lua_pushstring(L, key);
	lua_pushstring(L, value);
	lua_settable(L, -3);
}

static int
ldb_info_sham(struct lua_State *L)
{
	lua_newtable(L);
	sham_info(ldb_info_sham_cb, (void*)L);
	return 1;
}
#endif

static const struct luaL_reg
ldb_info_dynamic_meta [] =
{
	{"vclock", ldb_info_vclock},
	{"server", ldb_info_server},
	{"replication", ldb_info_replication},
	{"status", ldb_info_status},
	{"uptime", ldb_info_uptime},
	{"snapshot_pid", ldb_info_snapshot_pid},
	{"pid", ldb_info_pid},
#if 0
	{"sham", ldb_info_sham},
#endif
	{NULL, NULL}
};

/** Evaluate db.info.* function value and push it on the stack. */
static int
ldb_info_index(struct lua_State *L)
{
	lua_pushvalue(L, -1);			/* dup key */
	lua_gettable(L, lua_upvalueindex(1));   /* table[key] */

	if (!lua_isfunction(L, -1)) {
		/* No such key. Leave nil is on the stack. */
		return 1;
	}

	lua_call(L, 0, 1);
	lua_remove(L, -2);
	return 1;
}

/** Push a bunch of compile-time or start-time constants into a Lua table. */
static void
ldb_info_init_static_values(struct lua_State *L)
{
	/* bee version */
	lua_pushstring(L, "version");
	lua_pushstring(L, bee_version());
	lua_settable(L, -3);
}

/**
 * When user invokes db.info(), return a table of key/value
 * pairs containing the current info.
 */
static int
ldb_info_call(struct lua_State *L)
{
	lua_newtable(L);
	ldb_info_init_static_values(L);
	for (int i = 0; ldb_info_dynamic_meta[i].name; i++) {
		lua_pushstring(L, ldb_info_dynamic_meta[i].name);
		ldb_info_dynamic_meta[i].func(L);
		lua_settable(L, -3);
	}
	return 1;
}

/** Initialize db.info package. */
void
db_lua_info_init(struct lua_State *L)
{
	static const struct luaL_reg infolib [] = {
		{NULL, NULL}
	};

	luaL_register_module(L, "db.info", infolib);

	lua_newtable(L);		/* metatable for info */

	lua_pushstring(L, "__index");

	lua_newtable(L);
	luaL_register(L, NULL, ldb_info_dynamic_meta); /* table for __index */
	lua_pushcclosure(L, ldb_info_index, 1);
	lua_settable(L, -3);

	lua_pushstring(L, "__call");
	lua_pushcfunction(L, ldb_info_call);
	lua_settable(L, -3);

	lua_pushstring(L, "__serialize");
	lua_pushcfunction(L, ldb_info_call);
	lua_settable(L, -3);

	lua_setmetatable(L, -2);

	ldb_info_init_static_values(L);

	lua_pop(L, 1); /* info module */
}
