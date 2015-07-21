#ifndef SV_V_H_
#define SV_V_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct svv svv;

struct svv {
	uint64_t  lsn;
	uint32_t  size;
	uint8_t   flags;
	void     *log;
	svv      *next;
	srrbnode  node;
} srpacked;

extern svif sv_vif;

static inline char*
sv_vpointer(svv *v) {
	return (char*)(v) + sizeof(svv);
}

static inline uint32_t
sv_vsize(svv *v) {
	return sizeof(svv) + v->size;
}

static inline svv*
sv_vbuild(sr *r, srformatv *keys, int count, char *data, int size)
{
	assert(r->cmp->count == count);
	int total = sr_formatsize(r->format, keys, count, size);
	svv *v = sr_malloc(r->a, sizeof(svv) + total);
	if (srunlikely(v == NULL))
		return NULL;
	v->size  = total;
	v->lsn   = 0;
	v->flags = 0;
	v->log   = NULL;
	v->next  = NULL;
	memset(&v->node, 0, sizeof(v->node));
	char *ptr = sv_vpointer(v);
	sr_formatwrite(r->format, ptr, keys, count, data, size);
	return v;
}

static inline svv*
sv_vbuildraw(sra *a, char *src, int size)
{
	svv *v = sr_malloc(a, sizeof(svv) + size);
	if (srunlikely(v == NULL))
		return NULL;
	v->size  = size;
	v->flags = 0;
	v->lsn   = 0;
	v->next  = NULL;
	v->log   = NULL;
	memset(&v->node, 0, sizeof(v->node));
	memcpy(sv_vpointer(v), src, size);
	return v;
}

static inline svv*
sv_vdup(sra *a, sv *src)
{
	svv *v = sv_vbuildraw(a, sv_pointer(src), sv_size(src));
	if (srunlikely(v == NULL))
		return NULL;
	v->flags = sv_flags(src);
	v->lsn   = sv_lsn(src);
	return v;
}

static inline void
sv_vfree(sra *a, svv *v)
{
	while (v) {
		svv *n = v->next;
		sr_free(a, v);
		v = n;
	}
}

static inline svv*
sv_visible(svv *v, uint64_t vlsn) {
	while (v && v->lsn > vlsn)
		v = v->next;
	return v;
}

#endif
