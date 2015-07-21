
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

static void*
so_dbctl_get(soobj *obj, va_list args)
{
	sodbctl *ctl = (sodbctl*)obj;
	src c;
	memset(&c, 0, sizeof(c));
	char *name = va_arg(args, char*);
	if (strcmp(name, "name") == 0) {
		c.name  = "name";
		c.flags = SR_CSZREF|SR_CRO;
		c.value = &ctl->name;
	} else
	if (strcmp(name, "id") == 0) {
		c.name  = "id";
		c.flags = SR_CU32|SR_CRO;
		c.value = &ctl->id;
	} else {
		return NULL;
	}
	sodb *db = ctl->parent;
	return so_ctlreturn(&c, so_of(&db->o));
}

static void*
so_dbctl_type(soobj *o srunused, va_list args srunused) {
	return "database_ctl";
}

static soobjif sodbctlif =
{
	.ctl     = NULL,
	.async   = NULL,
	.open    = NULL,
	.destroy = NULL,
	.error   = NULL,
	.set     = NULL,
	.get     = so_dbctl_get,
	.del     = NULL,
	.drop    = NULL,
	.begin   = NULL,
	.prepare = NULL,
	.commit  = NULL,
	.cursor  = NULL,
	.object  = NULL,
	.type    = so_dbctl_type
};

static int
so_dbctl_free(sodbctl *c)
{
	sodb *o = c->parent;
	so *e = so_of(&o->o);
	if (c->name) {
		sr_free(&e->a, c->name);
		c->name = NULL;
	}
	if (c->path) {
		sr_free(&e->a, c->path);
		c->path = NULL;
	}
	if (c->formatsz) {
		sr_free(&e->a, c->formatsz);
		c->formatsz = NULL;
	}
	if (c->compression) {
		sr_free(&e->a, c->compression);
		c->compression = NULL;
	}
	sr_keyfree(&c->cmp, &e->a);
	return 0;
}

static int
so_dbctl_init(sodbctl *c, char *name, void *db)
{
	memset(c, 0, sizeof(*c));
	sodb *o = db;
	so *e = so_of(&o->o);
	so_objinit(&c->o, SODBCTL, &sodbctlif, &e->o);
	c->name = sr_strdup(&e->a, name);
	if (srunlikely(c->name == NULL)) {
		sr_error(&e->error, "%s", "memory allocation failed");
		return -1;
	}
	c->parent    = db;
	c->created   = 0;
	c->scheduled = 0;
	c->dropped   = 0;
	c->dropped_by_recover = 0;
	c->sync      = 1;
	c->compression_if = NULL;
	c->compression = sr_strdup(&e->a, "none");
	if (srunlikely(c->compression == NULL)) {
		sr_free(&e->a, c->name);
		c->name = NULL;
		sr_error(&e->error, "%s", "memory allocation failed");
		return -1;
	}
	sr_triggerinit(&c->on_complete);
	/* init single key part as string */
	int rc;
	sr_keyinit(&c->cmp);
	srkeypart *part = sr_keyadd(&c->cmp, &e->a);
	if (srunlikely(part == NULL)) {
		so_dbctl_free(c);
		return -1;
	}
	rc = sr_keypart_setname(part, &e->a, "key");
	if (srunlikely(rc == -1)) {
		so_dbctl_free(c);
		return -1;
	}
	rc = sr_keypart_set(part, &e->a, "string");
	if (srunlikely(rc == -1)) {
		so_dbctl_free(c);
		return -1;
	}
	c->format = SR_FKV;
	c->formatsz = sr_strdup(&e->a, "kv");
	if (srunlikely(c->formatsz == NULL)) {
		so_dbctl_free(c);
		return -1;
	}
	return 0;
}

static int
so_dbctl_validate(sodbctl *c)
{
	sodb *o = c->parent;
	so *e = so_of(&o->o);
	/* format */
	if (strcmp(c->formatsz, "kv") == 0) {
		c->format = SR_FKV;
	} else
	if (strcmp(c->formatsz, "document") == 0) {
		c->format = SR_FDOCUMENT;
	} else {
		sr_error(&e->error, "unknown format type '%s'", c->formatsz);
		return -1;
	}
	/* compression */
	if (strcmp(c->compression, "none") == 0) {
		c->compression_if = NULL;
	} else
	if (strcmp(c->compression, "zstd") == 0) {
		c->compression_if = &sr_zstdfilter;
	} else
	if (strcmp(c->compression, "lz4") == 0) {
		c->compression_if = &sr_lz4filter;
	} else {
		sr_error(&e->error, "unknown compression type '%s'", c->compression);
		return -1;
	}
	/* path */
	if (c->path == NULL) {
		char path[1024];
		snprintf(path, sizeof(path), "%s/%s", e->ctl.path, c->name);
		c->path = sr_strdup(&e->a, path);
		if (srunlikely(c->path == NULL)) {
			sr_error(&e->error, "%s", "memory allocation failed");
			return -1;
		}
	}
	o->r.cmp = &c->cmp;
	o->r.format = c->format;
	o->r.compression = c->compression_if;
	return 0;
}

static int
so_dbasync_set(soobj *obj, va_list args)
{
	sodbasync *o = (sodbasync*)obj;
	return so_txdbset(o->parent, 1, SVSET, args);
}

static int
so_dbasync_del(soobj *obj, va_list args)
{
	sodbasync *o = (sodbasync*)obj;
	return so_txdbset(o->parent, 1, SVDELETE, args);
}

static void*
so_dbasync_get(soobj *obj, va_list args)
{
	sodbasync *o = (sodbasync*)obj;
	return so_txdbget(o->parent, 1, 0, 1, args);
}

static void*
so_dbasync_obj(soobj *obj, va_list args srunused)
{
	sodbasync *o = (sodbasync*)obj;
	/* so_dbobj() */
	so *e = so_of(&o->o);
	return so_vnew(e, &o->parent->o);
}

static void*
so_dbasync_type(soobj *o srunused, va_list args srunused) {
	return "database_async";
}

static soobjif sodbasyncif =
{
	.ctl     = NULL,
	.async   = NULL,
	.destroy = NULL,
	.error   = NULL,
	.set     = so_dbasync_set,
	.get     = so_dbasync_get,
	.del     = so_dbasync_del,
	.drop    = NULL,
	.begin   = NULL,
	.prepare = NULL,
	.commit  = NULL,
	.cursor  = NULL,
	.object  = so_dbasync_obj,
	.type    = so_dbasync_type
};

static inline void
so_dbasync_init(sodbasync *a, sodb *db)
{
	so *e = so_of(&db->o);
	a->parent = db;
	so_objinit(&a->o, SODBASYNC, &sodbasyncif, &e->o);
}

static void*
so_dbasync(soobj *obj, va_list args srunused)
{
	sodb *o = (sodb*)obj;
	return &o->async.o;
}

static void*
so_dbctl(soobj *obj, va_list args srunused)
{
	sodb *o = (sodb*)obj;
	return &o->ctl.o;
}

static int
so_dbopen(soobj *obj, va_list args srunused)
{
	sodb *o = (sodb*)obj;
	so *e = so_of(&o->o);
	int status = so_status(&o->status);
	if (status == SO_RECOVER)
		goto online;
	if (status != SO_OFFLINE)
		return -1;
	int rc = so_dbctl_validate(&o->ctl);
	if (srunlikely(rc == -1))
		return -1;
	sx_indexset(&o->coindex, o->ctl.id, o->r.cmp);
	rc = so_recoverbegin(o);
	if (srunlikely(rc == -1))
		return -1;
	if (so_status(&e->status) == SO_RECOVER)
		return 0;
online:
	so_recoverend(o);
	rc = so_scheduler_add(&e->sched, o);
	if (srunlikely(rc == -1))
		return -1;
	o->ctl.scheduled = 1;
	return 0;
}

static int
so_dbdestroy(soobj *obj, va_list args srunused)
{
	sodb *o = (sodb*)obj;
	so *e = so_of(&o->o);

	int rcret = 0;
	int rc;
	int status = so_status(&e->status);
	if (status == SO_SHUTDOWN)
		goto shutdown;

	uint32_t ref;
	status = so_status(&o->status);
	switch (status) {
	case SO_MALFUNCTION:
	case SO_ONLINE:
	case SO_RECOVER:
		ref = so_dbunref(o, 0);
		if (ref > 0)
			return 0;
		/* set last visible transaction id */
		o->txn_max = sx_max(&e->xm);
		if (o->ctl.scheduled) {
			rc = so_scheduler_del(&e->sched, o);
			if (srunlikely(rc == -1))
				return -1;
		}
		so_objindex_unregister(&e->db, &o->o);
		sr_spinlock(&e->dblock);
		so_objindex_register(&e->db_shutdown, &o->o);
		sr_spinunlock(&e->dblock);
		so_statusset(&o->status, SO_SHUTDOWN);
		return 0;
	case SO_SHUTDOWN:
		/* this intended to be called from a
		 * background gc task */
		assert(so_dbrefof(o, 0) == 0);
		ref = so_dbrefof(o, 1);
		if (ref > 0)
			return 0;
		goto shutdown;
	case SO_OFFLINE:
		so_objindex_unregister(&e->db, &o->o);
		goto shutdown;
	default: assert(0);
	}

shutdown:;
	rc = so_objindex_destroy(&o->cursor);
	if (srunlikely(rc == -1))
		rcret = -1;
	sx_indexfree(&o->coindex, &e->xm);
	rc = si_close(&o->index, &o->r);
	if (srunlikely(rc == -1))
		rcret = -1;
	so_dbctl_free(&o->ctl);
	sd_cfree(&o->dc, &o->r);
	so_statusfree(&o->status);
	sr_spinlockfree(&o->reflock);
	sr_free(&e->a_db, o);
	return rcret;
}

static int
so_dbdrop(soobj *obj, va_list args srunused)
{
	sodb *o = (sodb*)obj;
	int status = so_status(&o->status);
	if (srunlikely(! so_statusactive_is(status)))
		return -1;
	o->ctl.dropped = 1;
	return 0;
}

static int
so_dberror(soobj *obj, va_list args srunused)
{
	sodb *o = (sodb*)obj;
	int status = so_status(&o->status);
	if (status == SO_MALFUNCTION)
		return 1;
	return 0;
}

static int
so_dbset(soobj *obj, va_list args)
{
	sodb *o = (sodb*)obj;
	return so_txdbset(o, 0, SVSET, args);
}

static int
so_dbdel(soobj *obj, va_list args)
{
	sodb *o = (sodb*)obj;
	return so_txdbset(o, 0, SVDELETE, args);
}

static void*
so_dbget(soobj *obj, va_list args)
{
	sodb *o = (sodb*)obj;
	return so_txdbget(o, 0, 0, 1, args);
}

static void*
so_dbcursor(soobj *o, va_list args)
{
	sodb *db = (sodb*)o;
	return so_cursornew(db, 0, args);
}

static void*
so_dbobj(soobj *obj, va_list args srunused)
{
	sodb *o = (sodb*)obj;
	so *e = so_of(&o->o);
	return so_vnew(e, obj);
}

static void*
so_dbtype(soobj *o srunused, va_list args srunused) {
	return "database";
}

static soobjif sodbif =
{
	.ctl      = so_dbctl,
	.async    = so_dbasync,
	.open     = so_dbopen,
	.destroy  = so_dbdestroy,
	.error    = so_dberror,
	.set      = so_dbset,
	.get      = so_dbget,
	.del      = so_dbdel,
	.drop     = so_dbdrop,
	.begin    = NULL,
	.prepare  = NULL,
	.commit   = NULL,
	.cursor   = so_dbcursor,
	.object   = so_dbobj,
	.type     = so_dbtype
};

soobj *so_dbnew(so *e, char *name)
{
	sodb *o = sr_malloc(&e->a_db, sizeof(sodb));
	if (srunlikely(o == NULL)) {
		sr_error(&e->error, "%s", "memory allocation failed");
		return NULL;
	}
	memset(o, 0, sizeof(*o));
	so_objinit(&o->o, SODB, &sodbif, &e->o);
	so_objindex_init(&o->cursor);
	so_statusinit(&o->status);
	so_statusset(&o->status, SO_OFFLINE);
	o->r     = e->r;
	o->r.cmp = &o->ctl.cmp;
	int rc = so_dbctl_init(&o->ctl, name, o);
	if (srunlikely(rc == -1)) {
		sr_free(&e->a_db, o);
		return NULL;
	}
	so_dbasync_init(&o->async, o);
	rc = si_init(&o->index, &o->r, &e->quota);
	if (srunlikely(rc == -1)) {
		sr_free(&e->a_db, o);
		so_dbctl_free(&o->ctl);
		return NULL;
	}
	o->ctl.id = sr_seq(&e->seq, SR_DSNNEXT);
	sx_indexinit(&o->coindex, o);
	sr_spinlockinit(&o->reflock);
	o->ref_be = 0;
	o->ref = 0;
	o->txn_min = sx_min(&e->xm);
	o->txn_max = o->txn_min;
	sd_cinit(&o->dc);
	return &o->o;
}

soobj *so_dbmatch(so *e, char *name)
{
	srlist *i;
	sr_listforeach(&e->db.list, i) {
		sodb *db = (sodb*)srcast(i, soobj, link);
		if (strcmp(db->ctl.name, name) == 0)
			return &db->o;
	}
	return NULL;
}

soobj *so_dbmatch_id(so *e, uint32_t id)
{
	srlist *i;
	sr_listforeach(&e->db.list, i) {
		sodb *db = (sodb*)srcast(i, soobj, link);
		if (db->ctl.id == id)
			return &db->o;
	}
	return NULL;
}

void so_dbref(sodb *o, int be)
{
	sr_spinlock(&o->reflock);
	if (be)
		o->ref_be++;
	else
		o->ref++;
	sr_spinunlock(&o->reflock);
}

uint32_t so_dbunref(sodb *o, int be)
{
	uint32_t prev_ref = 0;
	sr_spinlock(&o->reflock);
	if (be) {
		prev_ref = o->ref_be;
		if (o->ref_be > 0)
			o->ref_be--;
	} else {
		prev_ref = o->ref;
		if (o->ref > 0)
			o->ref--;
	}
	sr_spinunlock(&o->reflock);
	return prev_ref;
}

uint32_t so_dbrefof(sodb *o, int be)
{
	uint32_t ref = 0;
	sr_spinlock(&o->reflock);
	if (be)
		ref = o->ref_be;
	else
		ref = o->ref;
	sr_spinunlock(&o->reflock);
	return ref;
}

int so_dbgarbage(sodb *o)
{
	sr_spinlock(&o->reflock);
	int v = o->ref_be == 0 && o->ref == 0;
	sr_spinunlock(&o->reflock);
	return v;
}

int so_dbvisible(sodb *db, uint32_t txn)
{
	return db->txn_min < txn && txn <= db->txn_max;
}

void so_dbbind(so *o)
{
	srlist *i;
	sr_listforeach(&o->db.list, i) {
		sodb *db = (sodb*)srcast(i, soobj, link);
		int status = so_status(&db->status);
		if (so_statusactive_is(status))
			so_dbref(db, 1);
	}
}

void so_dbunbind(so *o, uint32_t txn)
{
	srlist *i;
	sr_listforeach(&o->db.list, i) {
		sodb *db = (sodb*)srcast(i, soobj, link);
		int status = so_status(&db->status);
		if (status != SO_ONLINE)
			continue;
		if (txn > db->txn_min)
			so_dbunref(db, 1);
	}
	sr_spinlock(&o->dblock);
	sr_listforeach(&o->db_shutdown.list, i) {
		sodb *db = (sodb*)srcast(i, soobj, link);
		if (so_dbvisible(db, txn))
			so_dbunref(db, 1);
	}
	sr_spinunlock(&o->dblock);
}

int so_dbmalfunction(sodb *o)
{
	so_statusset(&o->status, SO_MALFUNCTION);
	return -1;
}

svv *so_dbv(sodb *db, sov *o, int search)
{
	so *e = so_of(&db->o);
	svv *v;
	/* reuse object */
	if (o->v.v) {
		v = sv_vdup(db->r.a, &o->v);
		goto ret;
	}
	/* create object from raw data */
	if (o->raw) {
		v = sv_vbuildraw(db->r.a, o->raw, o->rawsize);
		goto ret;
	}
	/* create object using current format, supplied
	 * key-chain and value */
	if (srunlikely(o->keyc != db->ctl.cmp.count)) {
		sr_error(&e->error, "%s", "bad object key");
		return NULL;
	}
	/* switch to key-value format to avoid value
	 * copy during search operations */
	sr *runtime = &db->r;
	sr  runtime_search;
	if (search && db->r.format == SR_FDOCUMENT) {
		runtime_search = db->r;
		runtime_search.format = SR_FKV;
		runtime = &runtime_search;
	}
	v = sv_vbuild(runtime, o->keyv, o->keyc,
	              o->value,
	              o->valuesize);
ret:
	if (srunlikely(v == NULL)) {
		sr_error(&e->error, "%s", "memory allocation failed");
		return NULL;
	}
	return v;
}
