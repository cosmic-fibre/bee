
/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <libsr.h>
#include <libsv.h>
#include <libsl.h>

static uint8_t
sl_vifflags(sv *v) {
	return ((slv*)v->v)->flags;
}

static uint64_t
sl_viflsn(sv *v) {
	return ((slv*)v->v)->lsn;
}

static char*
sl_vifpointer(sv *v) {
	return (char*)v->v + sizeof(slv);
}

static uint32_t
sl_vifsize(sv *v) {
	return ((slv*)v->v)->size;
}

svif sl_vif =
{
	.flags   = sl_vifflags,
	.lsn     = sl_viflsn,
	.lsnset  = NULL,
	.pointer = sl_vifpointer,
	.size    = sl_vifsize
};
