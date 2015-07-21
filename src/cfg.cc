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
#include "cfg.h"
#include "lua/utils.h"

enum { MAX_OPT_NAME_LEN = 256, MAX_OPT_VAL_LEN = 256, MAX_STR_OPTS = 8 };

static void
cfg_get(const char *param)
{
	char buf[MAX_OPT_NAME_LEN];
	snprintf(buf, sizeof(buf), "return db.cfg.%s", param);
	luaL_dostring(bee_L, buf);
}

int
cfg_geti(const char *param)
{
	cfg_get(param);
	int val;
	if (lua_isboolean(bee_L, -1))
		val = lua_toboolean(bee_L, -1);
	else
		val = lua_tointeger(bee_L, -1);
	lua_pop(bee_L, 1);
	return val;
}

/* Support simultaneous cfg_gets("str1") and cfg_gets("str2") */
static const char *
cfg_tostring(struct lua_State *L)
{
	static char __thread values[MAX_STR_OPTS][MAX_OPT_VAL_LEN];
	static int __thread i = 0;
	if (lua_isnil(L, -1))
		return NULL;
	else {
		snprintf(values[i % MAX_STR_OPTS], MAX_OPT_VAL_LEN,
			 "%s", lua_tostring(L, -1));
		return values[i++ % MAX_STR_OPTS];
	}
}

const char *
cfg_gets(const char *param)
{
	cfg_get(param);
	const char *val = cfg_tostring(bee_L);
	lua_pop(bee_L, 1);
	return val;
}

double
cfg_getd(const char *param)
{
	cfg_get(param);
	double val = lua_tonumber(bee_L, -1);
	lua_pop(bee_L, 1);
	return val;
}

int
cfg_getarr_size(const char *name)
{
	cfg_get(name);
	luaL_checktype(bee_L, -1, LUA_TTABLE);
	int result = luaL_getn(bee_L, -1);
	lua_pop(bee_L, 1);
	return result;
}

const char *
cfg_getarr_elem(const char *name, int i)
{
	cfg_get(name);
	luaL_checktype(bee_L, -1, LUA_TTABLE);
	lua_rawgeti(bee_L, -1, i + 1);
	const char *val = cfg_tostring(bee_L);
	lua_pop(bee_L, 2);
	return val;
}

