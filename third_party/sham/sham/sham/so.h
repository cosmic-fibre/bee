#ifndef SO_H_
#define SO_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct so so;

struct so {
	soobj o;
	srmutex apilock;
	srspinlock reqlock;
	srspinlock dblock;
	soobjindex db;
	soobjindex db_shutdown;
	soobjindex tx;
	soobjindex req;
	soobjindex snapshot;
	soobjindex ctlcursor;
	sostatus status;
	soctl ctl;
	srseq seq;
	srquota quota;
	srpager pager;
	sra a;
	sra a_db;
	sra a_v;
	sra a_cursor;
	sra a_cursorcache;
	sra a_ctlcursor;
	sra a_snapshot;
	sra a_snapshotcursor;
	sra a_tx;
	sra a_sxv;
	sra a_req;
	seconf seconf;
	se se;
	slconf lpconf;
	slpool lp;
	sxmanager xm;
	soscheduler sched;
	srinjection ei;
	srerror error;
	sr r;
};

static inline int
so_active(so *o) {
	return so_statusactive(&o->status);
}

static inline void
so_apilock(soobj *o) {
	sr_mutexlock(&((so*)o)->apilock);
}

static inline void
so_apiunlock(soobj *o) {
	sr_mutexunlock(&((so*)o)->apilock);
}

static inline so*
so_of(soobj *o) {
	return (so*)o->env;
}

soobj *so_new(void);

#endif
