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
#include "db/lua/error.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
} /* extern "C" */

#include <fiber.h>
#include <errinj.h>

#include "lua/utils.h"
#include "db/error.h"

static int
ldb_error_raise(lua_State *L)
{
	uint32_t code = 0;
	const char *reason = NULL;
	const char *file = "";
	unsigned line = 0;
	lua_Debug info;

	/* lua_type(L, 1) == LUA_TTABLE - db.error table */
	int top = lua_gettop(L);
	if (top <= 1) {
		/* re-throw saved exceptions (if any) */
		if (fiber()->exception)
			fiber()->exception->raise();
		return 0;
	} else if (top >= 2 && lua_type(L, 2) == LUA_TNUMBER) {
		code = lua_tointeger(L, 2);
		reason = tnt_errcode_desc(code);
		if (top > 2) {
			/* Call string.format(reason, ...) to format message */
			lua_getglobal(L, "string");
			if (lua_isnil(L, -1))
				goto raise;
			lua_getfield(L, -1, "format");
			if (lua_isnil(L, -1))
				goto raise;
			lua_pushstring(L, reason);
			for (int i = 3; i <= top; i++)
				lua_pushvalue(L, i);
			lua_call(L, top - 1, 1);
			reason = lua_tostring(L, -1);
		}
	} else if (top == 2 && lua_istable(L, 2)) {
		/* A special case that rethrows raw error (used by net.db) */
		lua_getfield(L, 2, "code");
		code = lua_tointeger(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 2, "reason");
		reason = lua_tostring(L, -1);
		if (reason == NULL)
			reason = "";
		lua_pop(L, 1);
	} else {
		luaL_error(L, "db.error(): bad arguments");
	}

raise:
	if (lua_getstack(L, 1, &info) && lua_getinfo(L, "Sl", &info)) {
		if (*info.short_src) {
			file = info.short_src;
		} else if (*info.source) {
			file = info.source;
		} else {
			file = "eval";
		}
		line = info.currentline;
	}

	/* see tnt_raise() */
	say_debug("ClientError at %s:%i", file, line);
	throw new ClientError(file, line, reason, code);
	return 0;
}

static int
ldb_error_last(lua_State *L)
{
	if (lua_gettop(L) >= 1)
		luaL_error(L, "db.error.last(): bad arguments");

	Exception *e = fiber()->exception;

	if (e == NULL) {
		lua_pushnil(L);
	} else {
		lua_newtable(L);

		lua_pushstring(L, "message");
		lua_pushstring(L, e->errmsg());
		lua_settable(L, -3);

		lua_pushstring(L, "type");
		lua_pushstring(L, e->type());
		lua_settable(L, -3);

		lua_pushstring(L, "code");
		lua_pushinteger(L, ClientError::get_errcode(e));
		lua_settable(L, -3);

		if (SystemError *se = dynamic_cast<SystemError *>(e)) {
			lua_pushstring(L, "errno");
			lua_pushinteger(L, se->errnum());
			lua_settable(L, -3);
		}
       }
       return 1;
}

static int
ldb_error_clear(lua_State *L)
{
	if (lua_gettop(L) >= 1)
		luaL_error(L, "db.error.clear(): bad arguments");

	Exception::clear(&fiber()->exception);
	return 0;
}

static int
ldb_errinj_set(struct lua_State *L)
{
	char *name = (char*)luaL_checkstring(L, 1);
	bool state = lua_toboolean(L, 2);
	if (errinj_set_byname(name, state)) {
		lua_pushfstring(L, "error: can't find error injection '%s'", name);
		return 1;
	}
	lua_pushstring(L, "ok");
	return 1;
}

static inline int
ldb_errinj_cb(struct errinj *e, void *cb_ctx)
{
	struct lua_State *L = (struct lua_State*)cb_ctx;
	lua_pushstring(L, e->name);
	lua_newtable(L);
	lua_pushstring(L, "state");
	lua_pushboolean(L, e->state);
	lua_settable(L, -3);
	lua_settable(L, -3);
	return 0;
}

static int
ldb_errinj_info(struct lua_State *L)
{
	lua_newtable(L);
	errinj_foreach(ldb_errinj_cb, L);
	return 1;
}

void
db_lua_error_init(struct lua_State *L) {
	static const struct luaL_reg errorlib[] = {
		{NULL, NULL}
	};
	luaL_register_module(L, "db.error", errorlib);
	for (int i = 0; i < tnt_error_codes_enum_MAX; i++) {
		const char *name = tnt_error_codes[i].errstr;
		if (strstr(name, "UNUSED") || strstr(name, "RESERVED"))
			continue;
		assert(strncmp(name, "ER_", 3) == 0);
		lua_pushnumber(L, i);
		/* cut ER_ prefix from constant */
		lua_setfield(L, -2, name + 3);
	}
	lua_newtable(L);
	{
		lua_pushcfunction(L, ldb_error_raise);
		lua_setfield(L, -2, "__call");

		lua_newtable(L);
		{
			lua_pushcfunction(L, ldb_error_last);
			lua_setfield(L, -2, "last");
		}
		{
			lua_pushcfunction(L, ldb_error_clear);
			lua_setfield(L, -2, "clear");
		}
		{
			lua_pushcfunction(L, ldb_error_raise);
			lua_setfield(L, -2, "raise");
		}
		lua_setfield(L, -2, "__index");
	}
	lua_setmetatable(L, -2);

	lua_pop(L, 1);

	static const struct luaL_reg errinjlib[] = {
		{"info", ldb_errinj_info},
		{"set", ldb_errinj_set},
		{NULL, NULL}
	};
	/* db.error.injection is not set by register_module */
	luaL_register_module(L, "db.error.injection", errinjlib);
	lua_pop(L, 1);
}
