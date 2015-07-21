#ifndef SD_ITER_H_
#define SD_ITER_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct sditer sditer;

struct sditer {
	int validate;
	int compression;
	srbuf *compression_buf;
	sdindex *index;
	char *start, *end;
	char *page;
	char *pagesrc;
	sdpage pagev;
	uint32_t pos;
	sdv *dv;
	sv v;
} srpacked;

static inline int
sd_iternextpage(sriter *it)
{
	sditer *i = (sditer*)it->priv;
	char *page = NULL;
	if (srunlikely(i->page == NULL))
	{
		sdindexheader *h = i->index->h;
		page = i->start + h->offset + sd_indexsize(i->index->h);
		i->end = page + h->total;
	} else {
		page = i->pagesrc + sizeof(sdpageheader) + i->pagev.h->size;
	}
	if (srunlikely(page >= i->end)) {
		i->page = NULL;
		return 0;
	}
	i->pagesrc = page;
	i->page = i->pagesrc;

	/* decompression */
	if (i->compression) {
		sr_bufreset(i->compression_buf);

		/* prepare decompression buffer */
		sdpageheader *h = (sdpageheader*)i->page;
		int rc = sr_bufensure(i->compression_buf, it->r->a, h->sizeorigin + sizeof(sdpageheader));
		if (srunlikely(rc == -1)) {
			i->page = NULL;
			sr_malfunction(it->r->e, "%s", "memory allocation failed");
			return -1;
		}

		/* copy page header */
		memcpy(i->compression_buf->s, i->page, sizeof(sdpageheader));
		sr_bufadvance(i->compression_buf, sizeof(sdpageheader));

		/* decompression */
		srfilter f;
		rc = sr_filterinit(&f, (srfilterif*)it->r->compression, it->r, SR_FOUTPUT);
		if (srunlikely(rc == -1)) {
			i->page = NULL;
			sr_malfunction(it->r->e, "%s", "page decompression error");
			return -1;
		}
		rc = sr_filternext(&f, i->compression_buf, i->page + sizeof(sdpageheader), h->size);
		if (srunlikely(rc == -1)) {
			sr_filterfree(&f);
			i->page = NULL;
			sr_malfunction(it->r->e, "%s", "page decompression error");
			return -1;
		}
		sr_filterfree(&f);

		/* switch to decompressed page */
		i->page = i->compression_buf->s;
	}

	/* checksum */
	if (i->validate) {
		sdpageheader *h = (sdpageheader*)i->page;
		uint32_t crc = sr_crcs(it->r->crc, h, sizeof(sdpageheader), 0);
		if (srunlikely(crc != h->crc)) {
			i->page = NULL;
			sr_malfunction(it->r->e, "%s", "bad page header crc");
			return -1;
		}
	}
	sd_pageinit(&i->pagev, (void*)i->page);
	i->pos = 0;
	if (srunlikely(i->pagev.h->count == 0)) {
		i->page = NULL;
		i->dv = NULL;
		return 0;
	}
	i->dv = sd_pagev(&i->pagev, 0);
	sv_init(&i->v, &sd_vif, i->dv, i->pagev.h);
	return 1;
}

static inline int
sd_iter_open(sriter *i, sdindex *index, char *start, int validate,
             int compression,
             srbuf *compression_buf)
{
	sditer *ii = (sditer*)i->priv;
	ii->index       = index;
	ii->start       = start;
	ii->end         = NULL;
	ii->page        = NULL;
	ii->pagesrc     = NULL;
	ii->pos         = 0;
	ii->dv          = NULL;
	ii->validate    = validate;
	ii->compression = compression;
	ii->compression_buf = compression_buf;
	return sd_iternextpage(i);
}

static inline void
sd_iter_close(sriter *i srunused)
{
	sditer *ii = (sditer*)i->priv;
	(void)ii;
}

static inline int
sd_iter_has(sriter *i)
{
	sditer *ii = (sditer*)i->priv;
	return ii->page != NULL;
}

static inline void*
sd_iter_of(sriter *i)
{
	sditer *ii = (sditer*)i->priv;
	if (srunlikely(ii->page == NULL))
		return NULL;
	assert(ii->dv != NULL);
	assert(ii->v.v  == ii->dv);
	assert(ii->v.arg == ii->pagev.h);
	return &ii->v;
}

static inline void
sd_iter_next(sriter *i)
{
	sditer *ii = (sditer*)i->priv;
	if (srunlikely(ii->page == NULL))
		return;
	ii->pos++;
	if (srlikely(ii->pos < ii->pagev.h->count)) {
		ii->dv = sd_pagev(&ii->pagev, ii->pos);
		sv_init(&ii->v, &sd_vif, ii->dv, ii->pagev.h);
	} else {
		ii->dv = NULL;
		sd_iternextpage(i);
	}
}

extern sriterif sd_iter;

#endif
