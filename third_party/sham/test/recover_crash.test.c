
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

static void
recovercrash_deploy0(stc *cx srunused)
{
	void *env = sp_env();
	t( env != NULL );
	void *c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_set(c, "debug.error_injection.si_recover_0", "1") == 0 );
	t( sp_open(env) == -1 );
	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000000.0000000001.db.incomplete") == 1 );

	/* recover */
	env = sp_env();
	t( env != NULL );
	c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == -1 );
	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 0 );
	t( exists(cx->suite->dir, "0000000000.0000000001.db.incomplete") == 0 );

	env = sp_env();
	t( env != NULL );
	c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == -1 );
	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 0 );
	t( exists(cx->suite->dir, "0000000000.0000000001.db.incomplete") == 0 );
}

static void
recovercrash_branch0(stc *cx srunused)
{
	void *env = sp_env();
	t( env != NULL );
	void *c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );
	int key = 7;
	void *o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	key = 8;
	o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	key = 9;
	o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	t( sp_set(c, "debug.error_injection.si_branch_0", "1") == 0 );
	t( sp_set(c, "db.test.branch") == -1 );
	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 1 );

	/* recover */
	env = sp_env();
	t( env != NULL );
	c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	o = sp_object(db);
	t( o != NULL );
	t( sp_set(o, "order", ">=") == 0 );
	c = sp_cursor(db, o);
	t( c != NULL );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 7 );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 8 );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 9 );
	o = sp_get(c);
	t( o == NULL );
	t( sp_destroy(c) == 0 );

	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 1 );
}

static void
recovercrash_build0(stc *cx srunused)
{
	void *env = sp_env();
	t( env != NULL );
	void *c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );
	int key = 7;
	void *o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	key = 8;
	o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	key = 9;
	o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	t( sp_set(c, "debug.error_injection.sd_build_0", "1") == 0 );
	t( sp_set(c, "db.test.branch") == -1 );
	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 1 );

	/* recover */
	env = sp_env();
	t( env != NULL );
	c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	o = sp_object(db);
	t( sp_set(o, "order", ">=") == 0 );
	c = sp_cursor(db, o);
	t( c != NULL );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 7 );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 8 );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 9 );
	o = sp_get(c);
	t( o == NULL );
	t( sp_destroy(c) == 0 );

	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 1 );
}

static void
recovercrash_build1(stc *cx srunused)
{
	void *env = sp_env();
	t( env != NULL );
	void *c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );
	int key = 7;
	void *o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	key = 8;
	o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	key = 9;
	o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	t( sp_set(c, "debug.error_injection.sd_build_1", "1") == 0 );
	t( sp_set(c, "db.test.branch") == 0 ); /* seal crc is corrupted */
	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 1 );

	/* recover */
	env = sp_env();
	t( env != NULL );
	c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	o = sp_object(db);
	t( sp_set(o, "order", ">=") == 0 );
	c = sp_cursor(db, o);
	t( c != NULL );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 7 );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 8 );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 9 );
	o = sp_get(c);
	t( o == NULL );
	t( sp_destroy(c) == 0 );

	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 1 );
}

static void
recovercrash_compact0(stc *cx srunused)
{
	void *env = sp_env();
	t( env != NULL );
	void *c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );
	int key = 7;
	void *o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	key = 8;
	o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	key = 9;
	o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	t( sp_set(c, "debug.error_injection.si_compaction_0", "1") == 0 );
	t( sp_set(c, "db.test.branch") == 0 );
	t( sp_set(c, "db.test.compact") == -1 );
	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 1 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 1 );

	/* recover */
	env = sp_env();
	t( env != NULL );
	c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	o = sp_object(db);
	t( sp_set(o, "order", ">=") == 0 );
	c = sp_cursor(db, o);
	t( c != NULL );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 7 );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 8 );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 9 );
	o = sp_get(c);
	t( o == NULL );
	t( sp_destroy(c) == 0 );

	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 1 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 0 );
}

static void
recovercrash_compact1(stc *cx srunused)
{
	void *env = sp_env();
	t( env != NULL );
	void *c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );
	int key = 7;
	void *o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	key = 8;
	o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	key = 9;
	o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	t( sp_set(c, "debug.error_injection.si_compaction_1", "1") == 0 );
	t( sp_set(c, "db.test.branch") == 0 );
	t( sp_set(c, "db.test.compact") == -1 );
	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 1 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.seal") == 1 );

	/* recover */
	env = sp_env();
	t( env != NULL );
	c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	o = sp_object(db);
	t( sp_set(o, "order", ">=") == 0 );
	c = sp_cursor(db, o);
	t( c != NULL );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 7 );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 8 );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 9 );
	o = sp_get(c);
	t( o == NULL );
	t( sp_destroy(c) == 0 );

	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.seal") == 0 );
	t( exists(cx->suite->dir, "0000000003.db") == 1 );
}

static void
recovercrash_compact2(stc *cx srunused)
{
	void *env = sp_env();
	t( env != NULL );
	void *c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );
	int key = 7;
	void *o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	key = 8;
	o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	key = 9;
	o = sp_object(db);
	t( o != 0 );
	t( sp_set(o, "key", &key, sizeof(key)) == 0 );
	t( sp_set(db, o) == 0 );
	t( sp_set(c, "debug.error_injection.si_compaction_2", "1") == 0 );
	t( sp_set(c, "db.test.branch") == 0 );
	t( sp_set(c, "db.test.compact") == -1 );
	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.seal") == 1 );

	/* recover */
	env = sp_env();
	t( env != NULL );
	c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	o = sp_object(db);
	t( sp_set(o, "order", ">=") == 0 );
	c = sp_cursor(db, o);
	t( c != NULL );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 7 );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 8 );
	o = sp_get(c);
	t( o != NULL );
	t( *(int*)sp_get(o, "key", NULL) == 9 );
	o = sp_get(c);
	t( o == NULL );
	t( sp_destroy(c) == 0 );

	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.seal") == 0 );
	t( exists(cx->suite->dir, "0000000003.db") == 1 );
}

static void
recovercrash_compact3(stc *cx srunused)
{
	void *env = sp_env();
	t( env != NULL );
	void *c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "compaction.node_size", "60") == 0 );
	t( sp_set(c, "compaction.page_size", "60") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	int i = 0;
	while (i < 20) {
		void *o = sp_object(db);
		t( o != 0 );
		t( sp_set(o, "key", &i, sizeof(i)) == 0 );
		t( sp_set(db, o) == 0 );
		i++;
	}
	t( sp_set(c, "debug.error_injection.si_compaction_0", "1") == 0 );
	t( sp_set(c, "db.test.branch") == 0 );
	t( sp_set(c, "db.test.compact") == -1 );
	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 1 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 1 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.seal") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.incomplete") == 1 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.seal") == 0 );

	/* recover */
	env = sp_env();
	t( env != NULL );
	c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	void *o = sp_object(db);
	t( sp_set(o, "order", ">=") == 0 );
	c = sp_cursor(db, o);
	t( c != NULL );
	i = 0;
	while ((o = sp_get(c))) {
		t( *(int*)sp_get(o, "key", NULL) == i );
		i++;
	}
	t( sp_destroy(c) == 0 );

	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 1 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.seal") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.seal") == 0 );
}

static void
recovercrash_compact4(stc *cx srunused)
{
	void *env = sp_env();
	t( env != NULL );
	void *c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "compaction.node_size", "45") == 0 );
	t( sp_set(c, "compaction.page_size", "45") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	int i = 0;
	while (i < 20) {
		void *o = sp_object(db);
		t( o != 0 );
		t( sp_set(o, "key", &i, sizeof(i)) == 0 );
		t( sp_set(db, o) == 0 );
		i++;
	}
	t( sp_set(c, "debug.error_injection.si_compaction_1", "1") == 0 );
	t( sp_set(c, "db.test.branch") == 0 );
	t( sp_set(c, "db.test.compact") == -1 );
	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 1 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.seal") == 1 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.seal") == 1 );

	/* recover */
	env = sp_env();
	t( env != NULL );
	c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "8") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	void *o = sp_object(db);
	t( sp_set(o, "order", ">=") == 0 );
	c = sp_cursor(db, o);
	t( c != NULL );
	i = 0;
	while ((o = sp_get(c))) {
		t( *(int*)sp_get(o, "key", NULL) == i );
		i++;
	}
	t( sp_destroy(c) == 0 );

	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.seal") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.seal") == 0 );
	t( exists(cx->suite->dir, "0000000003.db") == 1 );
	t( exists(cx->suite->dir, "0000000004.db") == 1 );
}

static void
recovercrash_compact5(stc *cx srunused)
{
	void *env = sp_env();
	t( env != NULL );
	void *c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "compaction.node_size", "45") == 0 );
	t( sp_set(c, "compaction.page_size", "45") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	int i = 0;
	while (i < 20) {
		void *o = sp_object(db);
		t( o != 0 );
		t( sp_set(o, "key", &i, sizeof(i)) == 0 );
		t( sp_set(db, o) == 0 );
		i++;
	}
	t( sp_set(c, "debug.error_injection.si_compaction_2", "1") == 0 );
	t( sp_set(c, "db.test.branch") == 0 );
	t( sp_set(c, "db.test.compact") == -1 );
	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.seal") == 1 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.seal") == 1 );

	/* recover */
	env = sp_env();
	t( env != NULL );
	c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	void *o = sp_object(db);
	t( sp_set(o, "order", ">=") == 0 );
	c = sp_cursor(db, o);
	t( c != NULL );
	i = 0;
	while ((o = sp_get(c))) {
		t( *(int*)sp_get(o, "key", NULL) == i );
		i++;
	}
	t( sp_destroy(c) == 0 );

	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.seal") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.seal") == 0 );
	t( exists(cx->suite->dir, "0000000003.db") == 1 );
	t( exists(cx->suite->dir, "0000000004.db") == 1 );
}

static void
recovercrash_compact6(stc *cx srunused)
{
	void *env = sp_env();
	t( env != NULL );
	void *c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "compaction.node_size", "45") == 0 );
	t( sp_set(c, "compaction.page_size", "45") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	int i = 0;
	while (i < 20) {
		void *o = sp_object(db);
		t( o != 0 );
		t( sp_set(o, "key", &i, sizeof(i)) == 0 );
		t( sp_set(db, o) == 0 );
		i++;
	}
	t( sp_set(c, "debug.error_injection.si_compaction_3", "1") == 0 );
	t( sp_set(c, "db.test.branch") == 0 );
	t( sp_set(c, "db.test.compact") == -1 );
	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 1 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.seal") == 1 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.incomplete") == 1 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.seal") == 0 );

	/* recover */
	env = sp_env();
	t( env != NULL );
	c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	void *o = sp_object(db);
	t( sp_set(o, "order", ">=") == 0 );
	c = sp_cursor(db, o);
	t( c != NULL );
	i = 0;
	while ((o = sp_get(c))) {
		t( *(int*)sp_get(o, "key", NULL) == i );
		i++;
	}
	t( sp_destroy(c) == 0 );

	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 1 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.seal") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.seal") == 0 );
	t( exists(cx->suite->dir, "0000000003.db") == 0 );
	t( exists(cx->suite->dir, "0000000004.db") == 0 );
}

static void
recovercrash_compact7(stc *cx srunused)
{
	void *env = sp_env();
	t( env != NULL );
	void *c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "compaction.node_size", "45") == 0 );
	t( sp_set(c, "compaction.page_size", "45") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	int i = 0;
	while (i < 20) {
		void *o = sp_object(db);
		t( o != 0 );
		t( sp_set(o, "key", &i, sizeof(i)) == 0 );
		t( sp_set(db, o) == 0 );
		i++;
	}
	t( sp_set(c, "debug.error_injection.si_compaction_4", "1") == 0 );
	t( sp_set(c, "db.test.branch") == 0 );
	t( sp_set(c, "db.test.compact") == -1 );
	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.seal") == 0 );
	t( exists(cx->suite->dir, "0000000003.db") == 1 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.seal") == 1 );

	/* recover */
	env = sp_env();
	t( env != NULL );
	c = sp_ctl(env);
	t( c != NULL );
	t( sp_set(c, "sham.path", cx->suite->shamdir) == 0 );
	t( sp_set(c, "scheduler.threads", "0") == 0 );
	t( sp_set(c, "compaction.0.branch_wm", "1") == 0 );
	t( sp_set(c, "log.path", cx->suite->logdir) == 0 );
	t( sp_set(c, "log.sync", "0") == 0 );
	t( sp_set(c, "log.rotate_sync", "0") == 0 );
	t( sp_set(c, "db", "test") == 0 );
	t( sp_set(c, "db.test.path", cx->suite->dir) == 0 );
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	void *o = sp_object(db);
	t( sp_set(o, "order", ">=") == 0 );
	c = sp_cursor(db, o);
	t( c != NULL );
	i = 0;
	while ((o = sp_get(c))) {
		t( *(int*)sp_get(o, "key", NULL) == i );
		i++;
	}
	t( sp_destroy(c) == 0 );

	t( sp_destroy(env) == 0 );

	t( exists(cx->suite->dir, "0000000001.db") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000003.db.seal") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.incomplete") == 0 );
	t( exists(cx->suite->dir, "0000000001.0000000004.db.seal") == 0 );
	t( exists(cx->suite->dir, "0000000003.db") == 1 );
	t( exists(cx->suite->dir, "0000000004.db") == 1 );
}

stgroup *recovercrash_group(void)
{
	stgroup *group = st_group("recover_crash");
	st_groupadd(group, st_test("deploy_case0",  recovercrash_deploy0));
	st_groupadd(group, st_test("branch_case0",  recovercrash_branch0));
	st_groupadd(group, st_test("build_case0",   recovercrash_build0));
	st_groupadd(group, st_test("build_case1",   recovercrash_build1));
	st_groupadd(group, st_test("compact_case0", recovercrash_compact0));
	st_groupadd(group, st_test("compact_case1", recovercrash_compact1));
	st_groupadd(group, st_test("compact_case2", recovercrash_compact2));
	st_groupadd(group, st_test("compact_case3", recovercrash_compact3));
	st_groupadd(group, st_test("compact_case4", recovercrash_compact4));
	st_groupadd(group, st_test("compact_case5", recovercrash_compact5));
	st_groupadd(group, st_test("compact_case6", recovercrash_compact6));
	st_groupadd(group, st_test("compact_case7", recovercrash_compact7));
	return group;
}
