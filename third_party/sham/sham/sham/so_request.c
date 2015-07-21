
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
so_requestdestroy(soobj *obj, va_list args srunused)
{
	sorequest *r = (sorequest*)obj;
	so *e = so_of(&r->o);
	if (r->result)
		so_objdestroy((soobj*)r->result);
	if (r->arg_free)
		sv_vfree(&e->a, (svv*)r->arg.v);
	sr_free(&e->a_req, r);
	return 0;
}

static void*
so_requesttype(soobj *o srunused, va_list args srunused) {
	return "request";
}

static void*
so_requestget(soobj *obj, va_list args)
{
	sorequest *r = (sorequest*)obj;
	char *name = va_arg(args, char*);
	if (strcmp(name, "seq") == 0) {
		return &r->id;
	} else
	if (strcmp(name, "type") == 0) {
		return &r->op;
	} else
	if (strcmp(name, "status") == 0) {
		return &r->rc;
	} else
	if (strcmp(name, "result") == 0) {
		return r->result;
	} 
	return NULL;
}

static soobjif sorequestif =
{
	.ctl     = NULL,
	.async   = NULL,
	.open    = NULL,
	.destroy = so_requestdestroy,
	.error   = NULL,
	.set     = NULL,
	.del     = NULL,
	.drop    = NULL,
	.get     = so_requestget,
	.begin   = NULL,
	.prepare = NULL,
	.commit  = NULL,
	.cursor  = NULL,
	.object  = NULL,
	.type    = so_requesttype
};

void so_requestinit(so *e, sorequest *r, sorequestop op, soobj *object, sv *arg)
{
	so_objinit(&r->o, SOREQUEST, &sorequestif, &e->o);
	r->id = 0;
	r->op = op;
	r->object = object;
	r->arg = *arg;
	r->arg_free = 0;
	r->vlsn = 0;
	r->vlsn_generate = 0;
	r->result = NULL;
	r->rc = 0;
}

void so_requestvlsn(sorequest *r, uint64_t vlsn, int generate)
{
	r->vlsn = vlsn;
	r->vlsn_generate = generate;
}

void so_requestadd(so *e, sorequest *r)
{
	r->id = sr_seq(&e->seq, SR_RSNNEXT);
	sr_spinlock(&e->reqlock);
	so_objindex_register(&e->req, &r->o);
	sr_spinunlock(&e->reqlock);
}

void so_requestready(sorequest *r)
{
	sodb *db = (sodb*)r->object;
	/* emit callback */
	sr_triggerrun(&db->ctl.on_complete, &r->o);
	/* object left unfreed */
}

sorequest*
so_requestnew(so *e, sorequestop op, soobj *object, sv *arg)
{
	sorequest *r = sr_malloc(&e->a_req, sizeof(sorequest));
	if (srunlikely(r == NULL)) {
		sr_error(&e->error, "%s", "memory allocation failed");
		return NULL;
	}
	so_requestinit(e, r, op, object, arg);
	return r;
}

sorequest*
so_requestdispatch(so *e)
{
	sr_spinlock(&e->reqlock);
	if (e->req.n == 0) {
		sr_spinunlock(&e->reqlock);
		return NULL;
	}
	sorequest *req = (sorequest*)so_objindex_first(&e->req);
	so_objindex_unregister(&e->req, &req->o);
	sr_spinunlock(&e->reqlock);
	return req;
}

int so_requestcount(so *e)
{
	sr_spinlock(&e->reqlock);
	int n = e->req.n;
	sr_spinunlock(&e->reqlock);
	return n;
}
