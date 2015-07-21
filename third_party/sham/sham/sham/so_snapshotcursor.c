
/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <libsr.h>
#include <libsv.h>
#include <libsx.h>
#include <libsl.h>
#include <libsd.h>
#include <libsi.h>
#include <libse.h>
#include <libso.h>

static int
so_snapshotcursor_destroy(soobj *o, va_list args srunused)
{
	sosnapshotcursor *c = (sosnapshotcursor*)o;
	so *e = so_of(o);
	sr_buffree(&c->list, &e->a);
	so_objindex_unregister(&c->s->cursor, &c->o);
	sr_free(&e->a_snapshotcursor, c);
	return 0;
}

static inline int
so_snapshotcursor_next(sosnapshotcursor *c)
{
	if (srunlikely(c->pos == NULL))
		return 0;
	c->pos += sizeof(sodb**);
	if (c->pos >= c->list.p) {
		c->pos = NULL;
		c->v = NULL;
		return 0;
	}
	c->v = *(sodb**)c->pos;
	return 1;
}

static void*
so_snapshotcursor_get(soobj *o, va_list args srunused)
{
	sosnapshotcursor *c = (sosnapshotcursor*)o;
	if (c->ready) {
		c->ready = 0;
		return c->v;
	}
	if (so_snapshotcursor_next(c) == 0)
		return NULL;
	return c->v;
}

static void*
so_snapshotcursor_obj(soobj *obj, va_list args srunused)
{
	sosnapshotcursor *c = (sosnapshotcursor*)obj;
	if (c->v == NULL)
		return NULL;
	return c->v;
}

static void*
so_snapshotcursor_type(soobj *o srunused, va_list args srunused) {
	return "snapshot_cursor";
}

static soobjif sosnapshotcursorif =
{
	.ctl      = NULL,
	.open     = NULL,
	.destroy  = so_snapshotcursor_destroy,
	.error    = NULL,
	.set      = NULL,
	.get      = so_snapshotcursor_get,
	.del      = NULL,
	.drop     = NULL,
	.begin    = NULL,
	.prepare  = NULL,
	.commit   = NULL,
	.cursor   = NULL,
	.object   = so_snapshotcursor_obj,
	.type     = so_snapshotcursor_type 
};

static inline int
so_snapshotcursor_open(sosnapshotcursor *c)
{
	so *o = so_of(&c->o);
	int rc;
	uint32_t txn = c->s->t.id;
	srlist *i;
	sr_listforeach(&o->db.list, i) {
		sodb *db = (sodb*)srcast(i, soobj, link);
		int status = so_status(&db->status);
		if (status != SO_ONLINE)
			continue;
		if (txn > db->txn_min) {
			rc = sr_bufadd(&c->list, &o->a, &db, sizeof(db));
			if (srunlikely(rc == -1))
				return -1;
		}
	}
	sr_spinlock(&o->dblock);
	sr_listforeach(&o->db_shutdown.list, i) {
		sodb *db = (sodb*)srcast(i, soobj, link);
		if (db->txn_min < txn && txn <= db->txn_max) {
			rc = sr_bufadd(&c->list, &o->a, &db, sizeof(db));
			if (srunlikely(rc == -1))
				return -1;
		}
	}
	sr_spinunlock(&o->dblock);

	if (sr_bufsize(&c->list) == 0)
		return 0;
	c->ready = 1;
	c->pos = c->list.s;
	c->v = *(sodb**)c->list.s;
	return 0;
}

soobj *so_snapshotcursor_new(sosnapshot *s)
{
	so *e = so_of(&s->o);
	sosnapshotcursor *c = sr_malloc(&e->a_snapshotcursor, sizeof(sosnapshotcursor));
	if (srunlikely(c == NULL)) {
		sr_error(&e->error, "%s", "memory allocation failed");
		return NULL;
	}
	so_objinit(&c->o, SOSNAPSHOTCURSOR, &sosnapshotcursorif, &e->o);
	c->s = s;
	c->v = NULL;
	c->pos = NULL;
	c->ready = 0;
	sr_bufinit(&c->list);
	int rc = so_snapshotcursor_open(c);
	if (srunlikely(rc == -1)) {
		so_objdestroy(&c->o);
		return NULL;
	}
	so_objindex_register(&s->cursor, &c->o);
	return &c->o;
}
