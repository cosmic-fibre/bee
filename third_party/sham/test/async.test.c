
/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <libsr.h>
#include <libst.h>
#include <sham.h>

static int events = 0;

static void
on_complete(void *o, void *arg srunused)
{
	events++;
}

static void
async_set(stc *cx srunused)
{
	void *env = sp_env();
	t( env != NULL );
	void *c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );

	char pointer[64];
	snprintf(pointer, sizeof(pointer), "pointer: %p", (void*)on_complete);
	t( sp_set(c, "db.test.on_complete", pointer, NULL) == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	void *async = sp_async(db);
	t( async != NULL );

	uint32_t key = 7;
	void *o = sp_object(db);
	t( o != NULL );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(o, "value", &key, sizeof(key)) == 0 );
	t( sp_set(async, o) == 0 );

	t( sp_set(c, "scheduler.run") == 0 );
	t( sp_set(c, "scheduler.run") == 0 );
	t( events == 1 );

	t( sp_destroy(env) == 0 );
}

static void
async_get(stc *cx srunused)
{
	void *env = sp_env();
	t( env != NULL );
	void *c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );

	char pointer[64];
	snprintf(pointer, sizeof(pointer), "pointer: %p", (void*)on_complete);
	t( sp_set(c, "db.test.on_complete", pointer, NULL) == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	void *async = sp_async(db);
	t( async != NULL );

	events = 0;

	uint32_t key = 7;
	void *o = sp_object(async);
	t( o != NULL );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(o, "value", &key, sizeof(key)) == 0 );
	t( sp_set(async, o) == 0 );

	t( sp_set(c, "scheduler.run") == 0 );
	t( events == 1 );

	o = sp_object(async);
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	o = sp_get(async, o);
	t( o != NULL );
	t( strcmp(sp_type(o), "request") == 0 );

	t( *(uint64_t*)sp_get(o, "seq") == 2 );
	t( *(int*)sp_get(o, "type") == 1 );
	t( *(int*)sp_get(o, "status") == 0 );
	t( sp_get(o, "result") == NULL );

	t( sp_set(c, "scheduler.run") == 0 );
	t( events == 2 );

	void *req = o;
	o = sp_get(req, "result");
	t( o != NULL );
	t( *(int*)sp_get(o, "value", NULL) == key );
	sp_destroy(req);
	t( sp_destroy(env) == 0 );
}

stgroup *async_group(void)
{
	stgroup *group = st_group("async");
	st_groupadd(group, st_test("set", async_set));
	st_groupadd(group, st_test("get", async_get));
	return group;
}
