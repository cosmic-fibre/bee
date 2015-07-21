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
#include "lua/init.h"
#include "lua/utils.h"
#include "main.h"
#include "db/db.h"
#if defined(__FreeBSD__) || defined(__APPLE__)
#include "libgen.h"
#endif

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <lj_cdata.h>
} /* extern "C" */


#include <fiber.h>
#include <scoped_guard.h>
#include "coeio.h"
#include "lua/fiber.h"
#include "lua/ipc.h"
#include "lua/errno.h"
#include "lua/bsdsocket.h"
#include "lua/utils.h"
#include "third_party/lua-cjson/lua_cjson.h"
#include "third_party/lua-yaml/lyaml.h"
#include "lua/msgpack.h"
#include "lua/pickle.h"
#include "lua/fio.h"

#include <readline/readline.h>
#include <readline/history.h>

/**
 * The single Lua state of the transaction processor (tx) thread.
 */
struct lua_State *bee_L;
/**
 * The fiber running the startup Lua script
 */
struct fiber *script_fiber;
bool start_loop = true;

/* contents of src/lua/ files */
extern char uuid_lua[],
	msgpackffi_lua[],
	fun_lua[],
	astr_lua[],
	digest_lua[],
	init_lua[],
	fiber_lua[],
	log_lua[],
	uri_lua[],
	bsdsocket_lua[],
	console_lua[],
	net_db_lua[],
	help_lua[],
	help_en_US_lua[],
	tap_lua[],
	fio_lua[],
	shard_lua[],
	csv_lua[],
	expirationd_lua[],
	crypto_init_lua[],
    crypto_auth_lua[],
    crypto_box_lua[],
    crypto_core_lua[],
    crypto_generichash_lua[],
    crypto_hash_lua[],
    crypto_hashblocks_lua[],
    crypto_onetimeauth_lua[],
    crypto_scalarmult_lua[],
    crypto_secretbox_lua[],
    crypto_shorthash_lua[],
    crypto_secretbox_lua[],
    crypto_sign_lua[],
    crypto_stream_lua[],
    crypto_verify_lua[],
  cfg_lua[],
  randombytes_lua[];

static const char *lua_modules[] = {
	"bee", init_lua,
	"fiber", fiber_lua,
	"msgpackffi", msgpackffi_lua,
	"fun", fun_lua,
	"astr", astr_lua,
	"digest", digest_lua,
	"uuid", uuid_lua,
	"log", log_lua,
	"uri", uri_lua,
	"fio", fio_lua,
	"socket", bsdsocket_lua,
	"net.db", net_db_lua,
	"console", console_lua,
	"tap", tap_lua,
	"help.en_US", help_en_US_lua,
	"help", help_lua,
	"shard", shard_lua,
	"csv", csv_lua,
	"expirationd", expirationd_lua,
	"c_init", crypto_init_lua,
    "c_auth", crypto_auth_lua,
    "c_box", crypto_box_lua,
    "c_core", crypto_core_lua,
    "c_ghash", crypto_generichash_lua,
    "c_hash", crypto_hash_lua,
    "c_hashblock", crypto_hashblocks_lua,
    "c_onetimeauth", crypto_onetimeauth_lua,
    "c_scalarmult", crypto_scalarmult_lua,
    "c_secretbox", crypto_secretbox_lua,
    "c_shorthash", crypto_shorthash_lua,
    "c_secretbox", crypto_secretbox_lua,
    "c_sign", crypto_sign_lua,
    "c_stream", crypto_stream_lua,
    "c_verify", crypto_verify_lua,
    "c_randombytes", randombytes_lua,
  "cfg", cfg_lua,
	NULL
};

/*
 * {{{ db Lua library: common functions
 */

/**
 * Convert lua number or string to lua cdata 64bit number.
 */
static int
ldb_tonumber64(struct lua_State *L)
{
	if (lua_gettop(L) != 1)
		luaL_error(L, "tonumber64: wrong number of arguments");

	switch (lua_type(L, 1)) {
	case LUA_TNUMBER:
		return 1;
	case LUA_TSTRING:
	{
		const char *arg = luaL_checkstring(L, 1);
		char *arge;
		errno = 0;
		unsigned long long result = strtoull(arg, &arge, 10);
		if (errno == 0 && arge != arg) {
			luaL_pushuint64(L, result);
			return 1;
		}
		break;
	}
	case LUA_TCDATA:
	{
		uint32_t ctypeid = 0;
		luaL_checkcdata(L, 1, &ctypeid);
		if (ctypeid >= CTID_INT8 && ctypeid <= CTID_DOUBLE) {
			lua_pushvalue(L, 1);
			return 1;
		}
		break;
	}
	}
	lua_pushnil(L);
	return 1;
}

static int
ldb_coredump(struct lua_State *L __attribute__((unused)))
{
	coredump(60);
	lua_pushstring(L, "ok");
	return 1;
}

/* }}} */

/*
 * {{{ console library
 */

static ssize_t
readline_cb(va_list ap)
{
	const char **line = va_arg(ap, const char **);
	const char *prompt = va_arg(ap, const char *);
	/*
	 * libeio threads blocks all signals by default. Therefore, nobody
	 * can interrupt read(2) syscall inside readline() to correctly
	 * cleanup resources and restore terminal state. In case of signal
	 * a signal_cb(), a ev watcher in bee.cc will stop event
	 * loop and and stop entire process by exiting the main thread.
	 * rl_cleanup_after_signal() is called from bee_lua_free()
	 * in order to restore terminal state.
	 */
	*line = readline(prompt);
	return 0;
}

static int
bee_console_readline(struct lua_State *L)
{
	const char *prompt = ">";
	if (lua_gettop(L) > 0) {
		if (!lua_isstring(L, 1))
			luaL_error(L, "console.readline([prompt])");
		prompt = lua_tostring(L, 1);
	}

	char *line;
	if (coio_call(readline_cb, &line, prompt) != 0) {
		lua_pushnil(L);
		return 1;
	}
	auto scoped_guard = make_scoped_guard([&] { free(line); });
	if (!line) {
		lua_pushnil(L);
	} else {
		lua_pushstring(L, line);
	}
	return 1;
}

static int
bee_console_add_history(struct lua_State *L)
{
	if (lua_gettop(L) < 1 || !lua_isstring(L, 1))
		luaL_error(L, "console.add_history(string)");

	add_history(lua_tostring(L, 1));
	return 0;
}

/* }}} */

/**
 * Prepend the variable list of arguments to the Lua
 * package search path
 */
static void
bee_lua_setpaths(struct lua_State *L)
{
	const char *home = getenv("HOME");
	lua_getglobal(L, "package");
	int top = lua_gettop(L);
	lua_pushliteral(L, "./?.lua;");
	lua_pushliteral(L, "./?/init.lua;");
	if (home != NULL) {
		lua_pushstring(L, home);
		lua_pushliteral(L, "/.lib/lua/?.lua;");
		lua_pushstring(L, home);
		lua_pushliteral(L, "/.lib/lua/?/init.lua;");
		lua_pushstring(L, home);
		lua_pushliteral(L, "/.lib/lua/?.lua;");
		lua_pushstring(L, home);
		lua_pushliteral(L, "/.lib/lua/?/init.lua;");
	}
	lua_pushliteral(L, MODULE_LUAPATH ";");
	lua_getfield(L, top, "path");
	lua_concat(L, lua_gettop(L) - top);
	lua_setfield(L, top, "path");

	lua_pushliteral(L, "./?.so;");
	if (home != NULL) {
		lua_pushstring(L, home);
		lua_pushliteral(L, "/.lib/lua/?.so;");
		lua_pushstring(L, home);
		lua_pushliteral(L, "/.lib/lua/?.so;");
	}
	lua_pushliteral(L, MODULE_LIBPATH ";");
	lua_getfield(L, top, "cpath");
	lua_concat(L, lua_gettop(L) - top);
	lua_setfield(L, top, "cpath");

	assert(lua_gettop(L) == top);
	lua_pop(L, 1); /* package */
}

static int
luaopen_bee(lua_State *L)
{
	/* Set _G._BEE (like _VERSION) */
	lua_pushstring(L, bee_version());
	lua_setfield(L, LUA_GLOBALSINDEX, "_BEE");

	static const struct luaL_reg initlib[] = {
		{NULL, NULL}
	};
	luaL_register_module(L, "bee", initlib);


	/* version */
	lua_pushstring(L, bee_version());
	lua_setfield(L, -2, "version");

	/* build */
	lua_pushstring(L, "build");
	lua_newtable(L);

	/* build.target */
	lua_pushstring(L, "target");
	lua_pushstring(L, BUILD_INFO);
	lua_settable(L, -3);

	/* build.options */
	lua_pushstring(L, "options");
	lua_pushstring(L, BUILD_OPTIONS);
	lua_settable(L, -3);

	/* build.compiler */
	lua_pushstring(L, "compiler");
	lua_pushstring(L, COMPILER_INFO);
	lua_settable(L, -3);

	/* build.flags */
	lua_pushstring(L, "flags");
	lua_pushstring(L, BEE_C_FLAGS);
	lua_settable(L, -3);

	lua_settable(L, -3);    /* db.info.build */
	return 1;
}

void
bee_lua_init(const char *bee_bin, int argc, char **argv)
{
	lua_State *L = luaL_newstate();
	if (L == NULL) {
		panic("failed to initialize Lua");
	}
	luaL_openlibs(L);
	bee_lua_setpaths(L);

	/* Initialize ffi to enable luaL_pushcdata/luaL_checkcdata functions */
	luaL_loadstring(L, "return require('ffi')");
	lua_call(L, 0, 0);

	lua_register(L, "tonumber64", ldb_tonumber64);
	lua_register(L, "coredump", ldb_coredump);

	bee_lua_utils_init(L);
	bee_lua_fiber_init(L);
	bee_lua_ipc_init(L);
	bee_lua_errno_init(L);
	bee_lua_fio_init(L);
	bee_lua_bsdsocket_init(L);
	bee_lua_pickle_init(L);
	luaopen_msgpack(L);
	lua_pop(L, 1);
	luaopen_yaml(L);
	lua_pop(L, 1);
	luaopen_json(L);
	lua_pop(L, 1);

#if defined(HAVE_GNU_READLINE)
	/*
	 * Disable libreadline signals handlers. All signals are handled in
	 * main thread by libev watchers.
	 */
	rl_catch_signals = 0;
	rl_catch_sigwinch = 0;
#endif
	static const struct luaL_reg consolelib[] = {
		{"readline", bee_console_readline},
		{"add_history", bee_console_add_history},
		{NULL, NULL}
	};
	luaL_register_module(L, "console", consolelib);
	lua_pop(L, 1);

	lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
	for (const char **s = lua_modules; *s; s += 2) {
		const char *modname = *s;
		const char *modsrc = *(s + 1);
		const char *modfile = lua_pushfstring(L,
			"@builtin/%s.lua", modname);
		if (luaL_loadbuffer(L, modsrc, strlen(modsrc), modfile))
			panic("Error loading Lua module %s...: %s",
			      modname, lua_tostring(L, -1));
		lua_call(L, 0, 1);
		lua_setfield(L, -3, modname); /* package.loaded.modname = t */
		lua_pop(L, 1); /* chunkname */
	}
	lua_pop(L, 1); /* _LOADED */

	luaopen_bee(L);
	lua_pop(L, 1);

	db_lua_init(L);

	lua_newtable(L);
	lua_pushinteger(L, -1);
	lua_pushstring(L, bee_bin);
	lua_settable(L, -3);
	for (int i = 0; i < argc; i++) {
		lua_pushinteger(L, i);
		lua_pushstring(L, argv[i]);
		lua_settable(L, -3);
	}
	lua_setfield(L, LUA_GLOBALSINDEX, "arg");

	/* clear possible left-overs of init */
	lua_settop(L, 0);
	bee_L = L;
}

char *history = NULL;

extern "C" const char *
bee_error_message(void)
{
	/* called only from error handler */
	assert(fiber()->exception != NULL);
	return fiber()->exception->errmsg();
}

/**
 * Execute start-up script.
 */
static void
run_script(va_list ap)
{
	struct lua_State *L = va_arg(ap, struct lua_State *);
	const char *path = va_arg(ap, const char *);
	int argc = va_arg(ap, int);
	char **argv = va_arg(ap, char **);

	/*
	 * Return control to bee_lua_run_script.
	 * bee_lua_run_script then will start an auxiliary event
	 * loop and re-schedule this fiber.
	 */
	fiber_sleep(0.0);

	if (access(path, F_OK) == 0) {
		/* Execute script. */
		if (luaL_loadfile(L, path) != 0)
			panic("%s", lua_tostring(L, -1));
	} else if (!isatty(STDIN_FILENO)) {
		/* Execute stdin */
		if (luaL_loadfile(L, NULL) != 0)
			panic("%s", lua_tostring(L, -1));
	} else {
		say_crit("Laa ghaliba illallah");
		/* get console.start from package.loaded */
		lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
		lua_getfield(L, -1, "console");
		lua_getfield(L, -1, "start");
		lua_remove(L, -2); /* remove package.loaded.console */
		lua_remove(L, -2); /* remove package.loaded */
		start_loop = false;
	}
	try {
		lua_checkstack(L, argc - 1);
		for (int i = 1; i < argc; i++)
			lua_pushstring(L, argv[i]);
		ldb_call(L, lua_gettop(L) - 1, 0);
	} catch (Exception *e) {
		panic("%s", e->errmsg());
	}

	/* clear the stack from return values. */
	lua_settop(L, 0);
	/*
	 * Lua script finished. Stop the auxiliary event loop and
	 * return control back to bee_lua_run_script.
	 */
	ev_break(loop(), EVBREAK_ALL);
}

void
bee_lua_run_script(char *path, int argc, char **argv)
{
	const char *title = path ? basename(path) : "interactive";
	/*
	 * init script can call db.fiber.yield (including implicitly via
	 * db.insert, db.update, etc...), but db.fiber.yield() today,
	 * when called from 'sched' fiber crashes the server.
	 * To work this problem around we must run init script in
	 * a separate fiber.
	 */
	script_fiber = fiber_new(title, run_script);
	fiber_start(script_fiber, bee_L, path, argc, argv);

	/*
	 * Run an auxiliary event loop to re-schedule run_script fiber.
	 * When this fiber finishes, it will call ev_break to stop the loop.
	 */
	ev_run(loop(), 0);
	/* The fiber running the startup script has ended. */
	script_fiber = NULL;
}

void
bee_lua_free()
{
	/*
	 * Some part of the start script panicked, and called
	 * exit().  The call stack in this case leads us back to
	 * luaL_call() in run_script(). Trying to free a Lua state
	 * from within luaL_call() is not the smartest idea (@sa
	 * gh-612).
	 */
	if (script_fiber)
		return;
	/*
	 * Got to be done prior to anything else, since GC
	 * handlers can refer to other subsystems (e.g. fibers).
	 */
	if (bee_L) {
		/* collects garbage, invoking userdata gc */
		lua_close(bee_L);
	}
	bee_L = NULL;

	if (isatty(STDIN_FILENO)) {
		/* See comments in readline_cb() */
		rl_cleanup_after_signal();
	}
}
