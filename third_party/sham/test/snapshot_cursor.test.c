
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
snapshot_cursor_test0(stc *cx srunused)
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
	t( sp_set(c, "db.test.index.cmp", "u32", NULL) == 0 );
	t( sp_open(env) == 0 );
	void *db = sp_get(c, "db.test");
	t( db != NULL );

	t( sp_set(c, "snapshot", "test_snapshot") == 0 );
	void *snapshot = sp_get(c, "snapshot.test_snapshot");
	t( snapshot != NULL );

	t( sp_drop(db) == 0 );
	t( sp_destroy(db) == 0 ); /* unref */
	t( sp_destroy(db) == 0 ); /* shutdown */

	t( sp_set(c, "db", "test2") == 0 );
	void *db2 = sp_get(c, "db.test2");
	t( db2 != NULL );
	t( sp_open(db2) == 0 );

	void *snapcur = sp_ctl(snapshot, "db_list");
	void *o;
	while ((o = sp_get(snapcur))) {
		t( o == db );
	}
	sp_destroy(snapcur);

	t( sp_set(c, "snapshot", "test_snapshot2") == 0 );
	void *snapshot2 = sp_get(c, "snapshot.test_snapshot2");
	t( snapshot2 != NULL );

	snapcur = sp_ctl(snapshot2, "db_list");
	while ((o = sp_get(snapcur))) {
		t( o == db2 );
	}
	sp_destroy(snapcur);

	t( sp_destroy(env) == 0 );
}

stgroup *snapshot_cursor_group(void)
{
	stgroup *group = st_group("snapshot_cursor");
	st_groupadd(group, st_test("test0", snapshot_cursor_test0));
	return group;
}
