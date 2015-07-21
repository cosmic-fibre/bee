
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
so_ctl(soobj *obj, va_list args srunused)
{
	so *o = (so*)obj;
	return &o->ctl;
}

static int
so_open(soobj *o, va_list args)
{
	so *e = (so*)o;
	int status = so_status(&e->status);
	if (status == SO_RECOVER) {
		assert(e->ctl.two_phase_recover == 1);
		goto online;
	}
	if (status != SO_OFFLINE)
		return -1;
	int rc;
	rc = so_ctlvalidate(&e->ctl);
	if (srunlikely(rc == -1))
		return -1;
	so_statusset(&e->status, SO_RECOVER);

	/* set memory quota (disable during recovery) */
	sr_quotaset(&e->quota, e->ctl.memory_limit);
	sr_quotaenable(&e->quota, 0);

	/* repository recover */
	rc = so_recover_repository(e);
	if (srunlikely(rc == -1))
		return -1;
	/* databases recover */
	srlist *i, *n;
	sr_listforeach_safe(&e->db.list, i, n) {
		soobj *o = srcast(i, soobj, link);
		rc = o->i->open(o, args);
		if (srunlikely(rc == -1))
			return -1;
	}
	/* recover logpool */
	rc = so_recover(e);
	if (srunlikely(rc == -1))
		return -1;
	if (e->ctl.two_phase_recover)
		return 0;

online:
	/* complete */
	sr_listforeach_safe(&e->db.list, i, n) {
		soobj *o = srcast(i, soobj, link);
		rc = o->i->open(o, args);
		if (srunlikely(rc == -1))
			return -1;
	}
	/* enable quota */
	sr_quotaenable(&e->quota, 1);
	so_statusset(&e->status, SO_ONLINE);
	/* run thread-pool and scheduler */
	rc = so_scheduler_run(&e->sched);
	if (srunlikely(rc == -1))
		return -1;
	return 0;
}

static int
so_destroy(soobj *o, va_list args srunused)
{
	so *e = (so*)o;
	int rcret = 0;
	int rc;
	so_statusset(&e->status, SO_SHUTDOWN);
	rc = so_scheduler_shutdown(&e->sched);
	if (srunlikely(rc == -1))
		rcret = -1;
	rc = so_objindex_destroy(&e->req);
	if (srunlikely(rc == -1))
		rcret = -1;
	rc = so_objindex_destroy(&e->tx);
	if (srunlikely(rc == -1))
		rcret = -1;
	rc = so_objindex_destroy(&e->snapshot);
	if (srunlikely(rc == -1))
		rcret = -1;
	rc = so_objindex_destroy(&e->ctlcursor);
	if (srunlikely(rc == -1))
		rcret = -1;
	rc = so_objindex_destroy(&e->db);
	if (srunlikely(rc == -1))
		rcret = -1;
	rc = so_objindex_destroy(&e->db_shutdown);
	if (srunlikely(rc == -1))
		rcret = -1;
	rc = sl_poolshutdown(&e->lp);
	if (srunlikely(rc == -1))
		rcret = -1;
	rc = se_close(&e->se, &e->r);
	if (srunlikely(rc == -1))
		rcret = -1;
	sx_free(&e->xm);
	so_ctlfree(&e->ctl);
	sr_quotafree(&e->quota);
	sr_mutexfree(&e->apilock);
	sr_spinlockfree(&e->reqlock);
	sr_spinlockfree(&e->dblock);
	sr_seqfree(&e->seq);
	sr_pagerfree(&e->pager);
	so_statusfree(&e->status);
	free(e);
	return rcret;
}

static void*
so_begin(soobj *o, va_list args srunused) {
	return so_txnew((so*)o);
}

static int
so_error(soobj *o, va_list args srunused)
{
	so *e = (so*)o;
	int status = sr_errorof(&e->error);
	if (status == SR_ERROR_MALFUNCTION)
		return 1;
	status = so_status(&e->status);
	if (status == SO_MALFUNCTION)
		return 1;
	return 0;
}

static void*
so_type(soobj *o srunused, va_list args srunused) {
	return "env";
}

static soobjif soif =
{
	.ctl     = so_ctl,
	.async   = NULL,
	.open    = so_open,
	.destroy = so_destroy,
	.error   = so_error,
	.set     = NULL,
	.get     = NULL,
	.del     = NULL,
	.drop    = NULL,
	.begin   = so_begin,
	.prepare = NULL,
	.commit  = NULL,
	.cursor  = NULL,
	.object  = NULL,
	.type    = so_type
};

soobj *so_new(void)
{
	so *e = malloc(sizeof(*e));
	if (srunlikely(e == NULL))
		return NULL;
	memset(e, 0, sizeof(*e));
	so_objinit(&e->o, SOENV, &soif, &e->o /* self */);
	sr_pagerinit(&e->pager, 10, 4096);
	int rc = sr_pageradd(&e->pager);
	if (srunlikely(rc == -1)) {
		free(e);
		return NULL;
	}
	sr_aopen(&e->a, &sr_stda);
	sr_aopen(&e->a_db, &sr_slaba, &e->pager, sizeof(sodb));
	sr_aopen(&e->a_v, &sr_slaba, &e->pager, sizeof(sov));
	sr_aopen(&e->a_cursor, &sr_slaba, &e->pager, sizeof(socursor));
	sr_aopen(&e->a_cursorcache, &sr_slaba, &e->pager, sizeof(sicachebranch));
	sr_aopen(&e->a_ctlcursor, &sr_slaba, &e->pager, sizeof(soctlcursor));
	sr_aopen(&e->a_snapshot, &sr_slaba, &e->pager, sizeof(sosnapshot));
	sr_aopen(&e->a_snapshotcursor, &sr_slaba, &e->pager, sizeof(sosnapshotcursor));
	sr_aopen(&e->a_tx, &sr_slaba, &e->pager, sizeof(sotx));
	sr_aopen(&e->a_sxv, &sr_slaba, &e->pager, sizeof(sxv));
	sr_aopen(&e->a_req, &sr_slaba, &e->pager, sizeof(sorequest));
	so_statusinit(&e->status);
	so_statusset(&e->status, SO_OFFLINE);
	so_ctlinit(&e->ctl, e);
	so_objindex_init(&e->db);
	so_objindex_init(&e->db_shutdown);
	so_objindex_init(&e->tx);
	so_objindex_init(&e->snapshot);
	so_objindex_init(&e->ctlcursor);
	so_objindex_init(&e->req);
	sr_mutexinit(&e->apilock);
	sr_spinlockinit(&e->reqlock);
	sr_spinlockinit(&e->dblock);
	sr_quotainit(&e->quota);
	sr_seqinit(&e->seq);
	sr_errorinit(&e->error);
	srcrcf crc = sr_crc32c_function();
	sr_init(&e->r, &e->error, &e->a, &e->seq, SR_FKV, &e->ctl.ctlcmp, &e->ei, crc, NULL);
	se_init(&e->se);
	sl_poolinit(&e->lp, &e->r);
	sx_init(&e->xm, &e->r, &e->a_sxv);
	so_scheduler_init(&e->sched, e);
	return &e->o;
}
