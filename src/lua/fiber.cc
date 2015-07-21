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
#include "lua/fiber.h"

#include <fiber.h>
#include "lua/utils.h"
#include <scoped_guard.h>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
} /* extern "C" */

/*
 * }}}
 */

/* {{{ fiber Lua library: access to Bee fibers
 *
 * Each fiber can be running, suspended or dead.
 * When a fiber is created (fiber.create()) it's
 * running.
 *
 * All fibers are part of the fiber registry, fiber.
 * This registry can be searched either by
 * fiber id (fid), which is numeric, or by fiber name,
 * which is a string. If there is more than one
 * fiber with the given name, the first fiber that
 * matches is returned.
 *
 * Once fiber chunk is done or calls "return",
 * the fiber is considered dead. Its carcass is put into
 * fiber pool, and can be reused when another fiber is
 * created.
 *
 * A runaway fiber can be stopped with fiber.cancel().
 * fiber.cancel(), however, is advisory -- it works
 * only if the runaway fiber is calling fiber.testcancel()
 * once in a while. Most db.* hooks, such as db.delete()
 * or db.update(), are calling fiber.testcancel().
 *
 * Thus a runaway fiber can really only become cuckoo
 * if it does a lot of computations and doesn't check
 * whether it's been cancelled (just don't do that).
 *
 * The other potential problem comes from
 * fibers which never get scheduled, because are subscribed
 * or get no events. Such morphing fibers can be killed
 * with fiber.cancel(), since fiber.cancel()
 * sends an asynchronous wakeup event to the fiber.
 */

static const char *fiberlib_name = "fiber";

/**
 * @pre: stack top contains a table
 * @post: sets table field specified by name of the table on top
 * of the stack to a weak kv table and pops that weak table.
 */
static void
ldb_create_weak_table(struct lua_State *L, const char *name)
{
	lua_newtable(L);
	/* and a metatable */
	lua_newtable(L);
	/* weak keys and values */
	lua_pushstring(L, "kv");
	/* pops 'kv' */
	lua_setfield(L, -2, "__mode");
	/* pops the metatable */
	lua_setmetatable(L, -2);
	/* assigns and pops table */
	lua_setfield(L, -2, name);
	/* gets memoize back. */
	lua_getfield(L, -1, name);
	assert(! lua_isnil(L, -1));
}

/**
 * Push a userdata for the given fiber onto Lua stack.
 */
static void
ldb_pushfiber(struct lua_State *L, struct fiber *f)
{
	/*
	 * Use 'memoize'  pattern and keep a single userdata for
	 * the given fiber. This is important to not run __gc
	 * twice for a copy of an attached fiber -- __gc should
	 * not remove attached fiber's coro prematurely.
	 */
	luaL_getmetatable(L, fiberlib_name);
	int top = lua_gettop(L);
	lua_getfield(L, -1, "memoize");
	if (lua_isnil(L, -1)) {
		/* first access - instantiate memoize */
		/* pop the nil */
		lua_pop(L, 1);
		/* create memoize table */
		ldb_create_weak_table(L, "memoize");
	}
	/* Find out whether the fiber is  already in the memoize table. */
	lua_pushinteger(L, f->fid);
	lua_gettable(L, -2);
	if (lua_isnil(L, -1)) {
		/* no userdata for fiber created so far */
		/* pop the nil */
		lua_pop(L, 1);
		/* push the key back */
		lua_pushinteger(L, f->fid);
		/* create a new userdata */
		int *ptr = (int *) lua_newuserdata(L, sizeof(int));
		*ptr = f->fid;
		luaL_getmetatable(L, fiberlib_name);
		lua_setmetatable(L, -2);
		/* memoize it */
		lua_settable(L, -3);
		lua_pushinteger(L, f->fid);
		/* get it back */
		lua_gettable(L, -2);
	}
	/*
	 * Here we have a userdata on top of the stack and
	 * possibly some garbage just under the top. Move the
	 * result to the beginning of the stack and clear the rest.
	 */
	/* moves the current top to the old top */
	lua_replace(L, top);
	/* clears everything after the old top */
	lua_settop(L, top);
}

static struct fiber *
ldb_checkfiber(struct lua_State *L, int index)
{
	uint32_t fid = *(uint32_t *) luaL_checkudata(L, index, fiberlib_name);
	struct fiber *f = fiber_find(fid);
	if (f == NULL)
		luaL_error(L, "the fiber is dead");
	return f;
}

static struct fiber *
ldb_isfiber(struct lua_State *L, int narg)
{
	if (lua_getmetatable(L, narg) == 0)
		return NULL;
	luaL_getmetatable(L, fiberlib_name);
	struct fiber *f = NULL;
	if (lua_equal(L, -1, -2))
		f = fiber_find(* (int *) lua_touserdata(L, narg));
	lua_pop(L, 2);
	return f;
}

static int
ldb_fiber_id(struct lua_State *L)
{
	uint32_t fid;
	if (lua_gettop(L)  == 0)
		fid = fiber()->fid;
	else
		fid = *(uint32_t *) luaL_checkudata(L, 1, fiberlib_name);
	lua_pushinteger(L, fid);
	return 1;
}

static int
ldb_fiber_gc(struct lua_State *L)
{
	if (lua_gettop(L) == 0)
		return 0;
	struct fiber *f = ldb_isfiber(L, 1);
	(void) f;
	return 0;
}

#ifdef ENABLE_BACKTRACE
static int
fiber_backtrace_cb(int frameno, void *frameret, const char *func, size_t offset, void *cb_ctx)
{
	char buf[512];
	int l = snprintf(buf, sizeof(buf), "#%-2d %p in ", frameno, frameret);
	if (func)
		snprintf(buf + l, sizeof(buf) - l, "%s+%" PRI_SZ "", func, offset);
	else
		snprintf(buf + l, sizeof(buf) - l, "?");
	struct lua_State *L = (struct lua_State*)cb_ctx;
	lua_pushnumber(L, frameno + 1);
	lua_pushstring(L, buf);
	lua_settable(L, -3);
	return 0;
}
#endif

static int
ldb_fiber_statof(struct fiber *f, void *cb_ctx)
{
	struct lua_State *L = (struct lua_State *) cb_ctx;

	lua_pushinteger(L, f->fid);
	lua_newtable(L);

	lua_pushliteral(L, "name");
	lua_pushstring(L, fiber_name(f));
	lua_settable(L, -3);

	lua_pushstring(L, "fid");
	lua_pushnumber(L, f->fid);
	lua_settable(L, -3);

	lua_pushstring(L, "csw");
	lua_pushnumber(L, f->csw);
	lua_settable(L, -3);

#ifdef ENABLE_BACKTRACE
	lua_pushstring(L, "backtrace");
	lua_newtable(L);
	backtrace_foreach(fiber_backtrace_cb,
	                  f->last_stack_frame,
	                  f->coro.stack, f->coro.stack_size, L);
	lua_settable(L, -3);
#endif /* ENABLE_BACKTRACE */

	lua_settable(L, -3);
	return 0;
}

/**
 * Return fiber statistics.
 */
static int
ldb_fiber_info(struct lua_State *L)
{
	lua_newtable(L);
	fiber_stat(ldb_fiber_statof, L);
	lua_createtable(L, 0, 1);
	lua_pushliteral(L, "mapping"); /* YAML will use block mode */
	lua_setfield(L, -2, LUAL_SERIALIZE);
	lua_setmetatable(L, -2);
	return 1;
}

static void
db_lua_fiber_run(va_list ap)
{
	LuarefGuard coro_guard(va_arg(ap, int));
	struct lua_State *L = va_arg(ap, struct lua_State *);
	auto storage_guard = make_scoped_guard([=] {
		/* Destroy local storage */
		int storage_ref = (int)(intptr_t)
			fiber_get_key(fiber(), FIBER_KEY_LUA_STORAGE);
		if (storage_ref > 0)
			lua_unref(L, storage_ref);
	});

	try {
		ldb_call(L, lua_gettop(L) - 1, LUA_MULTRET);
	} catch (FiberCancelException *e) {
		throw;
	} catch (Exception *e) {
		e->log();
	}
}

/**
 * Create, resume and detach a fiber
 * given the function and its arguments.
 */
static int
ldb_fiber_create(struct lua_State *L)
{
	if (lua_gettop(L) < 1 || !lua_isfunction(L, 1))
		luaL_error(L, "fiber.create(function, ...): bad arguments");
	if (fiber_checkstack()) {
		lua_pushstring(L, "out of fiber stack");
		tnt_raise(LuajitError, L);
	}

	struct fiber *f = fiber_new("lua", db_lua_fiber_run);
	/* Not a system fiber. */
	struct lua_State *child_L = lua_newthread(L);
	int coro_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	/* Move the arguments to the new coro */
	lua_xmove(L, child_L, lua_gettop(L));
	ldb_pushfiber(L, f);
	fiber_start(f, coro_ref, child_L);
	return 1;
}

/**
 * Get fiber status.
 * This follows the rules of Lua coroutine.status() function:
 * Returns the status of fibier, as a string:
 * - "running", if the fiber is running (that is, it called status);
 * - "suspended", if the fiber is suspended in a call to yield(),
 *    or if it has not started running yet;
 * - "dead" if the fiber has finished its body function, or if it
 *   has stopped with an error.
 */
static int
ldb_fiber_status(struct lua_State *L)
{
	struct fiber *f;
	if (lua_gettop(L)) {
		uint32_t fid = *(uint32_t *)
			luaL_checkudata(L, 1, fiberlib_name);
		f = fiber_find(fid);
	} else {
		f = fiber();
	}
	const char *status;
	if (f == NULL || f->fid == 0) {
		/* This fiber is dead. */
		status = "dead";
	} else if (f == fiber()) {
		/* The fiber is the current running fiber. */
		status = "running";
	} else {
		/* None of the above: must be suspended. */
		status = "suspended";
	}
	lua_pushstring(L, status);
	return 1;
}

/** Get or set fiber name.
 * With no arguments, gets or sets the current fiber
 * name. It's also possible to get/set the name of
 * another fiber.
 */
static int
ldb_fiber_name(struct lua_State *L)
{
	struct fiber *f = fiber();
	int name_index = 1;
	if (lua_gettop(L) >= 1 && ldb_isfiber(L, 1)) {
		f = ldb_checkfiber(L, 1);
		name_index = 2;
	}
	if (lua_gettop(L) == name_index) {
		/* Set name. */
		const char *name = luaL_checkstring(L, name_index);
		fiber_set_name(f, name);
		return 0;
	} else {
		lua_pushstring(L, fiber_name(f));
		return 1;
	}
}

static int
ldb_fiber_storage(struct lua_State *L)
{
	struct fiber *f = ldb_checkfiber(L, 1);
	int storage_ref = (int)(intptr_t)
		fiber_get_key(f, FIBER_KEY_LUA_STORAGE);
	if (storage_ref <= 0) {
		lua_newtable(L); /* create local storage on demand */
		storage_ref = luaL_ref(L, LUA_REGISTRYINDEX);
		fiber_set_key(f, FIBER_KEY_LUA_STORAGE,
			      (void *)(intptr_t) storage_ref);
	}
	lua_rawgeti(L, LUA_REGISTRYINDEX, storage_ref);
	return 1;
}

static int
ldb_fiber_index(struct lua_State *L)
{
	if (lua_gettop(L) < 2)
		return 0;
	if (lua_isstring(L, 2) && strcmp(lua_tostring(L, 2), "storage") == 0)
		return ldb_fiber_storage(L);

	/* Get value from metatable */
	lua_getmetatable(L, 1);
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	return 1;
}

/**
 * Yield to the sched fiber and sleep.
 * @param[in]  amount of time to sleep (double)
 *
 * Only the current fiber can be made to sleep.
 */
static int
ldb_fiber_sleep(struct lua_State *L)
{
	if (! lua_isnumber(L, 1) || lua_gettop(L) != 1)
		luaL_error(L, "fiber.sleep(delay): bad arguments");
	double delay = lua_tonumber(L, 1);
	fiber_sleep(delay);
	return 0;
}

static int
ldb_fiber_yield(struct lua_State * /* L */)
{
	fiber_sleep(0);
	return 0;
}

static int
ldb_fiber(struct lua_State *L)
{
	ldb_pushfiber(L, fiber());
	return 1;
}

static int
ldb_fiber_find(struct lua_State *L)
{
	if (lua_gettop(L) != 1)
		luaL_error(L, "fiber.find(id): bad arguments");
	int fid = lua_tointeger(L, -1);
	struct fiber *f = fiber_find(fid);
	if (f)
		ldb_pushfiber(L, f);
	else
		lua_pushnil(L);
	return 1;
}

/**
 * Running and suspended fibers can be cancelled.
 * Zombie fibers can't.
 */
static int
ldb_fiber_cancel(struct lua_State *L)
{
	struct fiber *f = ldb_checkfiber(L, 1);
	fiber_cancel(f);
	return 0;
}

/** Kill a fiber by id. */
static int
ldb_fiber_kill(struct lua_State *L)
{
	if (lua_gettop(L) != 1)
		luaL_error(L, "fiber.kill(): bad arguments");
	int fid = lua_tointeger(L, -1);
	struct fiber *f = fiber_find(fid);
	if (f == NULL)
		luaL_error(L, "fiber.kill(): fiber not found");
	fiber_cancel(f);
	return 0;
}

static int
ldb_fiber_serialize(struct lua_State *L)
{
	struct fiber *f = ldb_checkfiber(L, 1);
	lua_createtable(L, 0, 1);
	lua_pushinteger(L, f->fid);
	lua_setfield(L, -2, "id");
	lua_pushstring(L, fiber_name(f));
	lua_setfield(L, -2, "name");
	ldb_fiber_status(L);
	lua_setfield(L, -2, "status");
	return 1;
}

static int
ldb_fiber_tostring(struct lua_State *L)
{
	char buf[20];
	struct fiber *f = ldb_checkfiber(L, 1);
	snprintf(buf, sizeof(buf), "fiber: %d", f->fid);
	lua_pushstring(L, buf);
	return 1;
}

/**
 * Check if this current fiber has been cancelled and
 * throw an exception if this is the case.
 */

static int
ldb_fiber_testcancel(struct lua_State *L)
{
	if (lua_gettop(L) != 0)
		luaL_error(L, "fiber.testcancel(): bad arguments");
	fiber_testcancel();
	return 0;
}

static int
ldb_fiber_wakeup(struct lua_State *L)
{
	struct fiber *f = ldb_checkfiber(L, 1);
	/*
	 * It's unsafe to wakeup fibers which don't expect
	 * it.
	 */
	if (f->flags & FIBER_IS_CANCELLABLE)
		fiber_wakeup(f);
	return 0;
}

static const struct luaL_reg ldb_fiber_meta [] = {
	{"id", ldb_fiber_id},
	{"name", ldb_fiber_name},
	{"cancel", ldb_fiber_cancel},
	{"status", ldb_fiber_status},
	{"testcancel", ldb_fiber_testcancel},
	{"__serialize", ldb_fiber_serialize},
	{"__tostring", ldb_fiber_tostring},
	{"wakeup", ldb_fiber_wakeup},
	{"__index", ldb_fiber_index},
	{"__gc", ldb_fiber_gc},
	{NULL, NULL}
};

static const struct luaL_reg fiberlib[] = {
	{"info", ldb_fiber_info},
	{"sleep", ldb_fiber_sleep},
	{"yield", ldb_fiber_yield},
	{"self", ldb_fiber},
	{"id", ldb_fiber_id},
	{"find", ldb_fiber_find},
	{"kill", ldb_fiber_kill},
	{"cancel", ldb_fiber_cancel},
	{"testcancel", ldb_fiber_testcancel},
	{"create", ldb_fiber_create},
	{"status", ldb_fiber_status},
	{"name", ldb_fiber_name},
	{NULL, NULL}
};

void
bee_lua_fiber_init(struct lua_State *L)
{
	luaL_register_module(L, fiberlib_name, fiberlib);
	lua_pop(L, 1);
	luaL_register_type(L, fiberlib_name, ldb_fiber_meta);
}

/*
 * }}}
 */
