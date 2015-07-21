
/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <libsr.h>
#include <libsv.h>
#include <libsd.h>

int sd_buildbegin(sdbuild *b, sr *r, int crc, int compress)
{
	b->crc = crc;
	b->compress = compress;
	int rc = sr_bufensure(&b->list, r->a, sizeof(sdbuildref));
	if (srunlikely(rc == -1))
		return sr_error(r->e, "%s", "memory allocation failed");
	sdbuildref *ref =
		(sdbuildref*)sr_bufat(&b->list, sizeof(sdbuildref), b->n);
	ref->k     = sr_bufused(&b->k);
	ref->ksize = 0;
	ref->v     = sr_bufused(&b->v);
	ref->vsize = 0;
	ref->c     = sr_bufused(&b->c);
	ref->csize = 0;
	rc = sr_bufensure(&b->k, r->a, sizeof(sdpageheader));
	if (srunlikely(rc == -1))
		return sr_error(r->e, "%s", "memory allocation failed");
	sdpageheader *h = sd_buildheader(b);
	memset(h, 0, sizeof(*h));
	h->lsnmin     = UINT64_MAX;
	h->lsnmindup  = UINT64_MAX;
	h->tsmin      = 0;
	memset(h->reserve, 0, sizeof(h->reserve));
	sr_bufadvance(&b->list, sizeof(sdbuildref));
	sr_bufadvance(&b->k, sizeof(sdpageheader));
	return 0;
}

int sd_buildadd(sdbuild *b, sr *r, sv *v, uint32_t flags)
{
	/* prepare metadata reference */
	int rc = sr_bufensure(&b->k, r->a, sizeof(sdv));
	if (srunlikely(rc == -1))
		return sr_error(r->e, "%s", "memory allocation failed");
	sdpageheader *h = sd_buildheader(b);
	sdv *sv = (sdv*)b->k.p;
	sv->lsn    = sv_lsn(v);
	sv->flags  = sv_flags(v) | flags;
	sv->size   = sv_size(v);
	sv->offset = sr_bufused(&b->v) - sd_buildref(b)->v;
	/* copy object */
	rc = sr_bufensure(&b->v, r->a, sv->size);
	if (srunlikely(rc == -1))
		return sr_error(r->e, "%s", "memory allocation failed");
	memcpy(b->v.p, sv_pointer(v), sv->size);
	sr_bufadvance(&b->v, sv->size);
	sr_bufadvance(&b->k, sizeof(sdv));
	/* update page header */
	h->count++;
	h->size +=  sv->size + sizeof(sdv);
	if (sv->lsn > h->lsnmax)
		h->lsnmax = sv->lsn;
	if (sv->lsn < h->lsnmin)
		h->lsnmin = sv->lsn;
	if (sv->flags & SVDUP) {
		h->countdup++;
		if (sv->lsn < h->lsnmindup)
			h->lsnmindup = sv->lsn;
	}
	return 0;
}

static inline int
sd_buildcompress(sdbuild *b, sr *r)
{
	/* reserve header */
	int rc = sr_bufensure(&b->c, r->a, sizeof(sdpageheader));
	if (srunlikely(rc == -1))
		return -1;
	sr_bufadvance(&b->c, sizeof(sdpageheader));
	/* compression (including meta-data) */
	sdbuildref *ref = sd_buildref(b);
	srfilter f;
	rc = sr_filterinit(&f, (srfilterif*)r->compression, r, SR_FINPUT);
	if (srunlikely(rc == -1))
		return -1;
	rc = sr_filterstart(&f, &b->c);
	if (srunlikely(rc == -1))
		goto error;
	rc = sr_filternext(&f, &b->c, b->k.s + ref->k + sizeof(sdpageheader),
	                   ref->ksize - sizeof(sdpageheader));
	if (srunlikely(rc == -1))
		goto error;
	rc = sr_filternext(&f, &b->c, b->v.s + ref->v, ref->vsize);
	if (srunlikely(rc == -1))
		goto error;
	rc = sr_filtercomplete(&f, &b->c);
	if (srunlikely(rc == -1))
		goto error;
	sr_filterfree(&f);
	return 0;
error:
	sr_filterfree(&f);
	return -1;
}

int sd_buildend(sdbuild *b, sr *r)
{
	/* update sizes */
	sdbuildref *ref = sd_buildref(b);
	ref->ksize = sr_bufused(&b->k) - ref->k;
	ref->vsize = sr_bufused(&b->v) - ref->v;
	ref->csize = 0;
	/* calculate data crc (non-compressed) */
	sdpageheader *h = sd_buildheader(b);
	uint32_t crc = 0;
	if (srlikely(b->crc)) {
		crc = sr_crcp(r->crc, b->k.s + ref->k, ref->ksize, 0);
		crc = sr_crcp(r->crc, b->v.s + ref->v, ref->vsize, crc);
	}
	h->crcdata = crc;
	/* compression */
	if (b->compress) {
		int rc = sd_buildcompress(b, r);
		if (srunlikely(rc == -1))
			return -1;
		ref->csize = sr_bufused(&b->c) - ref->c;
	}
	/* update page header */
	h->sizeorigin = h->size;
	if (b->compress)
		h->size = ref->csize - sizeof(sdpageheader);
	h->crc = sr_crcs(r->crc, h, sizeof(sdpageheader), 0);
	if (b->compress)
		memcpy(b->c.s + ref->c, h, sizeof(sdpageheader));
	return 0;
}

int sd_buildcommit(sdbuild *b)
{
	/* if in compression, reset kv */
	if (b->compress) {
		sr_bufreset(&b->k);
		sr_bufreset(&b->v);
	}
	b->n++;
	return 0;
}
