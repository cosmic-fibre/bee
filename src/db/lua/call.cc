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
#include "db/lua/call.h"
#include "pickle.h"

#include "db/lua/error.h"
#include "db/lua/tuple.h"
#include "db/lua/index.h"
#include "db/lua/space.h"
#include "db/lua/stat.h"
#include "db/lua/info.h"
#include "db/lua/session.h"
#include "db/tuple.h"

#include "lua/utils.h"
#include "lua/msgpack.h"
#include "iobuf.h"
#include "fiber.h"
#include "scoped_guard.h"
#include "db/db.h"
#include "db/port.h"
#include "db/request.h"
#include "db/engine.h"
#include "db/txn.h"
#include "db/user_def.h"
#include "db/user.h"
#include "db/schema.h"
#include "db/session.h"
#include "db/iproto_constants.h"
#include "db/iproto_port.h"

/* contents of db.lua, misc.lua, db.net.lua respectively */
extern char session_lua[],
	schema_lua[],
	load_cfg_lua[],
	snapshot_daemon_lua[];

static const char *lua_sources[] = {
	session_lua,
	schema_lua,
	snapshot_daemon_lua,
	load_cfg_lua,
	NULL
};

/*
 * Functions, exported in db_lua.h should have prefix
 * "db_lua_"; functions, available in Lua "db"
 * should start with "ldb_".
 */

/** {{{ Lua I/O: facilities to intercept db output
 * and push into Lua stack.
 */

struct port_lua
{
	struct port_vtab *vtab;
	struct lua_State *L;
};

static inline struct port_lua *
port_lua(struct port *port) { return (struct port_lua *) port; }

/*
 * For addU32/dupU32 do nothing -- the only uint32_t Db can give
 * us is tuple count, and we don't need it, since we intercept
 * everything into Lua stack first.
 * @sa port_add_lua_multret
 */

static void
port_lua_add_tuple(struct port *port, struct tuple *tuple)
{
	lua_State *L = port_lua(port)->L;
	try {
		ldb_pushtuple(L, tuple);
	} catch (...) {
		tnt_raise(ClientError, ER_PROC_LUA, lua_tostring(L, -1));
	}
}

void
port_lua_create(struct port_lua *port, struct lua_State *L)
{
	static struct port_vtab port_lua_vtab = {
		port_lua_add_tuple,
		null_port_eof,
	};
	port->vtab = &port_lua_vtab;
	port->L = L;
}

static void
port_lua_table_add_tuple(struct port *port, struct tuple *tuple)
{
	lua_State *L = port_lua(port)->L;
	try {
		int idx = luaL_getn(L, -1);	/* TODO: can be optimized */
		ldb_pushtuple(L, tuple);
		lua_rawseti(L, -2, idx + 1);

	} catch (...) {
		tnt_raise(ClientError, ER_PROC_LUA, lua_tostring(L, -1));
	}
}

/** Add all tuples to a Lua table. */
static struct port *
port_lua_table_create(struct lua_State *L)
{
	static struct port_vtab port_lua_vtab = {
		port_lua_table_add_tuple,
		null_port_eof,
	};
	struct port_lua *port = (struct port_lua *)
			region_alloc(&fiber()->gc, sizeof(struct port_lua));
	port->vtab = &port_lua_vtab;
	port->L = L;
	/* The destination table to append tuples to. */
	lua_newtable(L);
	return (struct port *) port;
}

/* }}} */

/**
 * The main extension provided to Lua by Bee/Db --
 * ability to call INSERT/UPDATE/SELECT/DELETE from within
 * a Lua procedure.
 *
 * This is a low-level API, and it expects
 * all arguments to be packed in accordance
 * with the binary protocol format (iproto
 * header excluded).
 *
 * Signature:
 * db.process(op_code, request)
 */
static int
ldb_process(lua_State *L)
{
	uint32_t op = lua_tointeger(L, 1); /* Get the first arg. */
	size_t sz;
	const char *req = luaL_checklstring(L, 2, &sz); /* Second arg. */
	if (op == IPROTO_CALL) {
		/*
		 * We should not be doing a CALL from within a CALL.
		 * To invoke one stored procedure from another, one must
		 * do it in Lua directly. This deals with
		 * infinite recursion, stack overflow and such.
		 */
		return luaL_error(L, "db.process(CALL, ...) is not allowed");
	}
	/* Capture all output into a Lua table. */
	struct port *port_lua = port_lua_table_create(L);
	struct request request;
	request_create(&request, op);
	request_decode(&request, req, sz);
	db_process(&request, port_lua);

	return 1;
}

void
ldb_request_create(struct request *request,
		    struct lua_State *L, enum iproto_type type,
		    int key, int tuple)
{
	request_create(request, type);
	request->space_id = lua_tointeger(L, 1);
	if (key > 0) {
		struct obuf key_buf;
		obuf_create(&key_buf, &fiber()->gc, LUAMP_ALLOC_FACTOR);
		luamp_encode_tuple(L, luaL_msgpack_default, &key_buf, key);
		request->key = obuf_join(&key_buf);
		request->key_end = request->key + obuf_size(&key_buf);
	}
	if (tuple > 0) {
		struct obuf tuple_buf;
		obuf_create(&tuple_buf, &fiber()->gc, LUAMP_ALLOC_FACTOR);
		luamp_encode_tuple(L, luaL_msgpack_default, &tuple_buf, tuple);
		request->tuple = obuf_join(&tuple_buf);
		request->tuple_end = request->tuple + obuf_size(&tuple_buf);
	}
}

static void
port_ffi_add_tuple(struct port *port, struct tuple *tuple)
{
	struct port_ffi *port_ffi = (struct port_ffi *) port;
	if (port_ffi->size >= port_ffi->capacity) {
		uint32_t capacity = (port_ffi->capacity > 0) ?
				2 * port_ffi->capacity : 1024;
		struct tuple **ret = (struct tuple **)
			realloc(port_ffi->ret, sizeof(*ret) * capacity);
		assert(ret != NULL);
		port_ffi->ret = ret;
		port_ffi->capacity = capacity;
	}
	tuple_ref(tuple);
	port_ffi->ret[port_ffi->size++] = tuple;
}

struct port_vtab port_ffi_vtab = {
	port_ffi_add_tuple,
	null_port_eof,
};

void
port_ffi_create(struct port_ffi *port)
{
	memset(port, 0, sizeof(*port));
	port->vtab = &port_ffi_vtab;
}

static inline void
port_ffi_clear(struct port_ffi *port)
{
	for (uint32_t i = 0; i < port->size; i++) {
		tuple_unref(port->ret[i]);
		port->ret[i] = NULL;
	}
	port->size = 0;
}

void
port_ffi_destroy(struct port_ffi *port)
{
	free(port->ret);
}

int
dbffi_select(struct port_ffi *port, uint32_t space_id, uint32_t index_id,
	      int iterator, uint32_t offset, uint32_t limit,
	      const char *key, const char *key_end)
{
	struct request request;
	request_create(&request, IPROTO_SELECT);
	request.space_id = space_id;
	request.index_id = index_id;
	request.limit = limit;
	request.offset = offset;
	request.iterator = iterator;
	request.key = key;
	request.key_end = key_end;

	/*
	 * A single instance of port_ffi object is used
	 * for all selects, reset it.
	 */
	port->size = 0;
	try {
		db_process(&request, (struct port *) port);
		return 0;
	} catch (Exception *e) {
		/*
		 * The tuples will be not blessed and garbage
		 * collected, unreference them here, to avoid
		 * a leak.
		 */
		port_ffi_clear(port);
		/* will be hanled by db.error() in Lua */
		return -1;
	}
}

static int
ldb_insert(lua_State *L)
{
	if (lua_gettop(L) != 2 || !lua_isnumber(L, 1))
		return luaL_error(L, "Usage space:insert(tuple)");

	struct request request;
	struct port_lua port;
	ldb_request_create(&request, L, IPROTO_INSERT, -1, 2);
	port_lua_create(&port, L);
	db_process(&request, (struct port *) &port);
	return lua_gettop(L) - 2;
}

static int
ldb_replace(lua_State *L)
{
	if (lua_gettop(L) != 2 || !lua_isnumber(L, 1))
		return luaL_error(L, "Usage space:replace(tuple)");

	struct request request;
	struct port_lua port;
	ldb_request_create(&request, L, IPROTO_REPLACE, -1, 2);
	port_lua_create(&port, L);
	db_process(&request, (struct port *) &port);
	return lua_gettop(L) - 2;
}

static int
ldb_update(lua_State *L)
{
	if (lua_gettop(L) != 4 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		return luaL_error(L, "Usage space:update(key, ops)");

	struct request request;
	struct port_lua port;
	ldb_request_create(&request, L, IPROTO_UPDATE, 3, 4);
	request.index_base = 1; /* field ids are one-indexed */
	port_lua_create(&port, L);
	/* Ignore index_id for now */
	db_process(&request, (struct port *) &port);
	return lua_gettop(L) - 4;
}

static int
ldb_delete(lua_State *L)
{
	if (lua_gettop(L) != 3 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		return luaL_error(L, "Usage space:delete(key)");

	struct request request;
	struct port_lua port;
	ldb_request_create(&request, L, IPROTO_DELETE, 3, -1);
	port_lua_create(&port, L);
	/* Ignore index_id for now */
	db_process(&request, (struct port *) &port);
	return lua_gettop(L) - 3;
}

static int
ldb_commit(lua_State * /* L */)
{
	struct txn *txn = in_txn();
	/**
	 * COMMIT is like BEGIN or ROLLBACK
	 * a "transaction-initiating statement".
	 * Do nothing if transaction is not started,
	 * it's the same as BEGIN + COMMIT.
	*/
	if (! txn)
		return 0;
	try {
		txn_commit(txn);
	} catch (...) {
		txn_rollback();
		throw;
	}
	return 0;
}

/**
 * A helper to find a Lua function by name and put it
 * on top of the stack.
 */
static int
db_lua_find(lua_State *L, const char *name, const char *name_end)
{
	int index = LUA_GLOBALSINDEX;
	int objstack = 0;
	const char *start = name, *end;

	while ((end = (const char *) memchr(start, '.', name_end - start))) {
		lua_checkstack(L, 3);
		lua_pushlstring(L, start, end - start);
		lua_gettable(L, index);
		if (! lua_istable(L, -1))
			tnt_raise(ClientError, ER_NO_SUCH_PROC,
				  name_end - name, name);
		start = end + 1; /* next piece of a.b.c */
		index = lua_gettop(L); /* top of the stack */
	}

	/* db.something:method */
	if ((end = (const char *) memchr(start, ':', name_end - start))) {
		lua_checkstack(L, 3);
		lua_pushlstring(L, start, end - start);
		lua_gettable(L, index);
		if (! (lua_istable(L, -1) ||
			lua_islightuserdata(L, -1) || lua_isuserdata(L, -1) ))
				tnt_raise(ClientError, ER_NO_SUCH_PROC,
					  name_end - name, name);
		start = end + 1; /* next piece of a.b.c */
		index = lua_gettop(L); /* top of the stack */
		objstack = index;
	}


	lua_pushlstring(L, start, name_end - start);
	lua_gettable(L, index);
	if (!lua_isfunction(L, -1) && !lua_istable(L, -1)) {
		/* lua_call or lua_gettable would raise a type error
		 * for us, but our own message is more verbose. */
		tnt_raise(ClientError, ER_NO_SUCH_PROC,
			  name_end - name, name);
	}
	/* setting stack that it would contain only
	 * the function pointer. */
	if (index != LUA_GLOBALSINDEX) {
		if (objstack == 0) {        /* no object, only a function */
			lua_replace(L, 1);
		} else if (objstack == 1) { /* just two values, swap them */
			lua_insert(L, -2);
		} else {		    /* long path */
			lua_insert(L, 1);
			lua_insert(L, 2);
			objstack = 1;
		}
		lua_settop(L, 1 + objstack);
	}
	return 1 + objstack;
}


/**
 * A helper to find lua stored procedures for db.call.
 * db.call iteslf is pure Lua, to avoid issues
 * with infinite call recursion smashing C
 * thread stack.
 */

static int
ldb_call_loadproc(struct lua_State *L)
{
	const char *name;
	size_t name_len;
	name = lua_tolstring(L, 1, &name_len);
	return db_lua_find(L, name, name + name_len);
}

/**
 * Check access to a function and change the current
 * user id if the function is a set-definer-user-id one.
 * The original user is restored in the destructor.
 */
struct SetuidGuard
{
	/** True if the function was set-user-id one. */
	bool setuid;
	struct credentials *orig_credentials;

	inline SetuidGuard(const char *name, uint32_t name_len,
			   uint8_t access);
	inline ~SetuidGuard();
};

SetuidGuard::SetuidGuard(const char *name, uint32_t name_len,
			 uint8_t access)
	:setuid(false)
	,orig_credentials(current_user())
{

	/*
	 * If the user has universal access, don't bother with setuid.
	 * No special check for ADMIN user is necessary
	 * since ADMIN has universal access.
	 */
	if ((orig_credentials->universal_access & PRIV_ALL) == PRIV_ALL)
		return;
	access &= ~orig_credentials->universal_access;
	/*
	 * We need to look up the function by name even if
	 * the user has access to it, since it could require
	 * a set of other user id.
	 */
	struct func_def *func = func_by_name(name, name_len);
	if (func == NULL && access == 0) {
		/**
		 * Well, the function is not explicitly defined,
		 * so it's obviously not a setuid one. Wasted
		 * cycles on look up while the user had universal
		 * access :(
		 */
		return;
	}
	if (func == NULL || (func->uid != orig_credentials->uid &&
	     access & ~func->access[orig_credentials->auth_token].effective)) {
		/* Access violation, report error. */
		char name_buf[DB_NAME_MAX + 1];
		snprintf(name_buf, sizeof(name_buf), "%.*s", name_len, name);
		struct user *user = user_cache_find(orig_credentials->uid);

		tnt_raise(ClientError, ER_FUNCTION_ACCESS_DENIED,
			  priv_name(access), user->name, name_buf);
	}
	if (func->setuid) {
		/** Remember and change the current user id. */
		if (func->owner_credentials.auth_token >= DB_USER_MAX) {
			/*
			 * Fill the cache upon first access, since
			 * when func_def is created, no user may
			 * be around to fill it (recovery of
			 * system spaces from a snapshot).
			 */
			struct user *owner = user_cache_find(func->uid);
			credentials_init(&func->owner_credentials, owner);
		}
		setuid = true;
		fiber_set_user(fiber(), &func->owner_credentials);
	}
}

SetuidGuard::~SetuidGuard()
{
	if (setuid)
		fiber_set_user(fiber(), orig_credentials);
}

/**
 * A quick approximation if a Lua table is an array.
 *
 * JSON can only have strings as keys, so if the first
 * table key is 1, it's definitely not a json map,
 * and very likely an array.
 */
static inline bool
lua_isarray(struct lua_State *L, int i)
{
	if (lua_istable(L, i) == false)
		return false;
	lua_pushnil(L);
	if (lua_next(L, i) == 0) /* the table is empty */
		return true;
	bool index_starts_at_1 = lua_isnumber(L, -2) &&
		lua_tonumber(L, -2) == 1;
	lua_pop(L, 2);
	return index_starts_at_1;
}

/**
 * Invoke a Lua stored procedure from the binary protocol
 * (implementation of 'CALL' command code).
 */
static inline void
execute_call(lua_State *L, struct request *request, struct obuf *out)
{
	const char *name = request->key;
	uint32_t name_len = mp_decode_strl(&name);

	/* Try to find a function by name */
	int oc = db_lua_find(L, name, name + name_len);
	/**
	 * Check access to the function and optionally change
	 * execution time user id (set user id). Sic: the order
	 * is important, as is described in
	 * https://github.com/bee/bee/issues/300
	 * - if a function does not exist, say it first.
	 */
	SetuidGuard setuid(name, name_len, PRIV_X);
	/* Push the rest of args (a tuple). */
	const char *args = request->tuple;
	uint32_t arg_count = mp_decode_array(&args);
	luaL_checkstack(L, arg_count, "call: out of stack");

	for (uint32_t i = 0; i < arg_count; i++) {
		luamp_decode(L, luaL_msgpack_default, &args);
	}
	lua_call(L, arg_count + oc - 1, LUA_MULTRET);

	/**
	 * Add all elements from Lua stack to iproto.
	 *
	 * To allow clients to understand a complex return from
	 * a procedure, we are compatible with SELECT protocol,
	 * and return the number of return values first, and
	 * then each return value as a tuple.
	 *
	 * If a Lua stack contains at least one scalar, each
	 * value on the stack is converted to a tuple. A Lua
	 * is converted to a tuple with multiple fields.
	 *
	 * If the stack is a Lua table, each member of which is
	 * not scalar, each member of the table is converted to
	 * a tuple. This way very large lists of return values can
	 * be used, since Lua stack size is limited by 8000 elements,
	 * while Lua table size is pretty much unlimited.
	 */

	uint32_t count = 0;
	struct obuf_svp svp = iproto_prepare_select(out);

	/** Check if we deal with a table of tables. */
	int nrets = lua_gettop(L);
	if (nrets == 1 && lua_isarray(L, 1)) {
		/*
		 * The table is not empty and consists of tables
		 * or tuples. Treat each table element as a tuple,
		 * and push it.
		 */
		lua_pushnil(L);
		int has_keys = lua_next(L, 1);
		if (has_keys  && (lua_istable(L, -1) || lua_istuple(L, -1))) {
			do {
				luamp_encode_tuple(L, luaL_msgpack_default,
						   out, -1);
				++count;
				lua_pop(L, 1);
			} while (lua_next(L, 1));
			goto done;
		} else if (has_keys) {
			lua_pop(L, 1);
		}
	}
	for (int i = 1; i <= nrets; ++i) {
		if (lua_isarray(L, i) || lua_istuple(L, i)) {
			luamp_encode_tuple(L, luaL_msgpack_default, out, i);
		} else {
			luamp_encode_array(luaL_msgpack_default, out, 1);
			luamp_encode(L, luaL_msgpack_default, out, i);
		}
		++count;
	}

done:
	iproto_reply_select(out, &svp, request->header->sync, count);
}

void
db_lua_call(struct request *request, struct obuf *out)
{
	lua_State *L = NULL;
	try {
		L = lua_newthread(bee_L);
		LuarefGuard coro_ref(bee_L);
		execute_call(L, request, out);
		if (in_txn()) {
			say_warn("a transaction is active at CALL return");
			txn_rollback();
		}
	} catch (Exception *e) {
		txn_rollback();
		/* Let all well-behaved exceptions pass through. */
		throw;
	} catch (...) {
		txn_rollback();
		/* Convert Lua error to a Bee exception. */
		tnt_raise(LuajitError, L != NULL ? L : bee_L);
	}
}

static inline void
execute_eval(lua_State *L, struct request *request, struct obuf *out)
{
	/* Check permissions */
	access_check_universe(PRIV_X);

	/* Compile expression */
	const char *expr = request->key;
	uint32_t expr_len = mp_decode_strl(&expr);
	if (luaL_loadbuffer(L, expr, expr_len, "=eval"))
		tnt_raise(LuajitError, L);

	/* Unpack arguments */
	const char *args = request->tuple;
	uint32_t arg_count = mp_decode_array(&args);
	luaL_checkstack(L, arg_count, "eval: out of stack");
	for (uint32_t i = 0; i < arg_count; i++) {
		luamp_decode(L, luaL_msgpack_default, &args);
	}

	/* Call compiled code */
	lua_call(L, arg_count, LUA_MULTRET);

	/* Send results of the called procedure to the client. */
	struct obuf_svp svp = iproto_prepare_select(out);
	int nrets = lua_gettop(L);
	for (int k = 1; k <= nrets; ++k) {
		luamp_encode(L, luaL_msgpack_default, out, k);
	}
	iproto_reply_select(out, &svp, request->header->sync, nrets);
}

void
db_lua_eval(struct request *request, struct obuf *out)
{
	lua_State *L = NULL;
	try {
		L = lua_newthread(bee_L);
		LuarefGuard coro_ref(bee_L);
		execute_eval(L, request, out);
	} catch (Exception *e) {
		/* Let all well-behaved exceptions pass through. */
		throw;
	} catch (...) {
		/* Convert Lua error to a Bee exception. */
		tnt_raise(LuajitError, L != NULL ? L : bee_L);
	}
}

static int
ldb_snapshot(struct lua_State *L)
{
	int ret = db_snapshot();
	if (ret == 0) {
		lua_pushstring(L, "ok");
		return 1;
	}
	luaL_error(L, "can't save snapshot, errno %d (%s)",
		   ret, strerror(ret));
	return 1;
}

static const struct luaL_reg dblib[] = {
	{"snapshot", ldb_snapshot},
	{"commit", ldb_commit},
	{NULL, NULL}
};

static const struct luaL_reg dblib_internal[] = {
	{"process", ldb_process},
	{"call_loadproc",  ldb_call_loadproc},
	{"insert", ldb_insert},
	{"replace", ldb_replace},
	{"update", ldb_update},
	{"delete", ldb_delete},
	{NULL, NULL}
};

void
db_lua_init(struct lua_State *L)
{
	/* Use luaL_register() to set _G.db */
	luaL_register(L, "db", dblib);
	lua_pop(L, 1);
	luaL_register(L, "db.internal", dblib_internal);
	lua_pop(L, 1);

	db_lua_error_init(L);
	db_lua_tuple_init(L);
	db_lua_index_init(L);
	db_lua_space_init(L);
	db_lua_info_init(L);
	db_lua_stat_init(L);
	db_lua_session_init(L);

	/* Load Lua extension */
	for (const char **s = lua_sources; *s; s++) {
		if (luaL_dostring(L, *s))
			panic("Error loading Lua source %.160s...: %s",
			      *s, lua_tostring(L, -1));
	}

	assert(lua_gettop(L) == 0);
}
