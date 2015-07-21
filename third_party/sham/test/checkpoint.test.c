
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
checkpoint_test0(stc *cx srunused)
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
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	int key = 0;
	while (key < 10) {
		void *o = sp_object(db);
		t( o != NULL );
		t( sp_set(o, "key", &key, sizeof(key)) == 0 );
		t( sp_set(o, "value", &key, sizeof(key)) == 0 );
		t( sp_set(db, o) == 0 );
		key++;
	}

	void *o = sp_get(c, "scheduler.checkpoint_active");
	t( o != NULL );
	t( strcmp(sp_get(o, "value", NULL), "0") == 0 );
	sp_destroy(o);
	o = sp_get(c, "scheduler.checkpoint_lsn");
	t( o != NULL );
	t( strcmp(sp_get(o, "value", NULL), "0") == 0 );
	sp_destroy(o);
	o = sp_get(c, "scheduler.checkpoint_lsn_last");
	t( o != NULL );
	t( strcmp(sp_get(o, "value", NULL), "0") == 0 );
	sp_destroy(o);

	t( sp_set(c, "scheduler.checkpoint") == 0 );

	o = sp_get(c, "scheduler.checkpoint_active");
	t( o != NULL );
	t( strcmp(sp_get(o, "value", NULL), "1") == 0 );
	sp_destroy(o);
	o = sp_get(c, "scheduler.checkpoint_lsn");
	t( o != NULL );
	t( strcmp(sp_get(o, "value", NULL), "10") == 0 );
	sp_destroy(o);
	o = sp_get(c, "scheduler.checkpoint_lsn_last");
	t( o != NULL );
	t( strcmp(sp_get(o, "value", NULL), "0") == 0 );
	sp_destroy(o);

	t( sp_set(c, "scheduler.run") == 1 );
	t( sp_set(c, "scheduler.run") == 1 );

	o = sp_get(c, "scheduler.checkpoint_active");
	t( o != NULL );
	t( strcmp(sp_get(o, "value", NULL), "0") == 0 );
	sp_destroy(o);
	o = sp_get(c, "scheduler.checkpoint_lsn");
	t( o != NULL );
	t( strcmp(sp_get(o, "value", NULL), "0") == 0 );
	sp_destroy(o);
	o = sp_get(c, "scheduler.checkpoint_lsn_last");
	t( o != NULL );
	t( strcmp(sp_get(o, "value", NULL), "10") == 0 );
	sp_destroy(o);

	t( sp_destroy(env) == 0 );
}

static void
checkpoint_test1(stc *cx srunused)
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
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	int key = 0;
	while (key < 20) {
		void *o = sp_object(db);
		t( o != NULL );
		t( sp_set(o, "key", &key, sizeof(key)) == 0 );
		t( sp_set(o, "value", &key, sizeof(key)) == 0 );
		t( sp_set(db, o) == 0 );
		key++;
	}

	t( sp_set(c, "log.rotate") == 0 );
	void *o = sp_get(c, "log.files");
	t( o != NULL );
	t( strcmp(sp_get(o, "value", NULL), "2") == 0 );
	sp_destroy(o);

	key = 40;
	while (key < 80) {
		void *o = sp_object(db);
		t( o != NULL );
		t( sp_set(o, "key", &key, sizeof(key)) == 0 );
		t( sp_set(o, "value", &key, sizeof(key)) == 0 );
		t( sp_set(db, o) == 0 );
		key++;
	}

	t( sp_set(c, "log.rotate") == 0 );
	o = sp_get(c, "log.files");
	t( o != NULL );
	t( strcmp(sp_get(o, "value", NULL), "3") == 0 );
	sp_destroy(o);

	t( sp_set(c, "scheduler.checkpoint") == 0 );
	t( sp_set(c, "scheduler.run") == 1 );
	t( sp_set(c, "scheduler.run") == 1 );

	o = sp_get(c, "log.files");
	t( o != NULL );
	t( strcmp(sp_get(o, "value", NULL), "1") == 0 );
	sp_destroy(o);

	t( sp_destroy(env) == 0 );
}

static int trigger_called = 0;

static inline void
checkpoint_trigger(void *object, void *arg)
{
	trigger_called = 1;
}

static void
checkpoint_test2(stc *cx srunused)
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
	t( sp_set(c, "db.test.sync", "0") == 0 );
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );

	char pointer[64];
	snprintf(pointer, sizeof(pointer), "pointer: %p", (void*)checkpoint_trigger);
	t( sp_set(c, "scheduler.checkpoint_on_complete", pointer, NULL) == 0 );

	void *db = sp_get(c, "db.test");
	t( db != NULL );
	t( sp_open(env) == 0 );

	int key = 0;
	while (key < 20) {
		void *o = sp_object(db);
		t( o != NULL );
		t( sp_set(o, "key", &key, sizeof(key)) == 0 );
		t( sp_set(o, "value", &key, sizeof(key)) == 0 );
		t( sp_set(db, o) == 0 );
		key++;
	}

	t( sp_set(c, "log.rotate") == 0 );
	void *o = sp_get(c, "log.files");
	t( o != NULL );
	t( strcmp(sp_get(o, "value", NULL), "2") == 0 );
	sp_destroy(o);

	key = 40;
	while (key < 80) {
		void *o = sp_object(db);
		t( o != NULL );
		t( sp_set(o, "key", &key, sizeof(key)) == 0 );
		t( sp_set(o, "value", &key, sizeof(key)) == 0 );
		t( sp_set(db, o) == 0 );
		key++;
	}

	t( sp_set(c, "log.rotate") == 0 );
	o = sp_get(c, "log.files");
	t( o != NULL );
	t( strcmp(sp_get(o, "value", NULL), "3") == 0 );
	sp_destroy(o);

	t( sp_set(c, "scheduler.checkpoint") == 0 );
	t( sp_set(c, "scheduler.run") == 1 );
	t( sp_set(c, "scheduler.run") == 1 );

	t( trigger_called == 1 );

	o = sp_get(c, "log.files");
	t( o != NULL );
	t( strcmp(sp_get(o, "value", NULL), "1") == 0 );
	sp_destroy(o);

	t( sp_destroy(env) == 0 );
}


stgroup *checkpoint_group(void)
{
	stgroup *group = st_group("checkpoint");
	st_groupadd(group, st_test("test0", checkpoint_test0));
	st_groupadd(group, st_test("test1", checkpoint_test1));
	st_groupadd(group, st_test("test2", checkpoint_test2));
	return group;
}
