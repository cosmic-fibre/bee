#ifndef SD_INDEXITER_H_
#define SD_INDEXITER_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct sdindexiter sdindexiter;

struct sdindexiter {
	sdindex *index;
	sdindexpage *v;
	int pos;
	srorder cmp;
	void *key;
	int keysize;
} srpacked;

static inline int
sd_indexiter_seek(sriter *i, void *key, int size, int *minp, int *midp, int *maxp)
{
	sdindexiter *ii = (sdindexiter*)i->priv;
	int match = 0;
	int min = 0;
	int max = ii->index->h->count - 1;
	int mid = 0;
	while (max >= min)
	{
		mid = min + ((max - min) >> 1);
		sdindexpage *page = sd_indexpage(ii->index, mid);

		int rc = sd_indexpage_cmp(ii->index, page, key, size, i->r->cmp);
		switch (rc) {
		case -1: min = mid + 1;
			continue;
		case  1: max = mid - 1;
			continue;
		default: match = 1;
			goto done;
		}
	}
done:
	*minp = min;
	*midp = mid;
	*maxp = max;
	return match;
}

static inline int
sd_indexiter_route(sriter *i)
{
	sdindexiter *ii = (sdindexiter*)i->priv;
	int mid, min, max;
	int rc = sd_indexiter_seek(i, ii->key, ii->keysize, &min, &mid, &max);
	if (srlikely(rc))
		return mid;
	if (srunlikely(min >= (int)ii->index->h->count))
		min = ii->index->h->count - 1;
	return min;
}

static inline int
sd_indexiter_open(sriter *i, sdindex *index, srorder o, void *key, int keysize)
{
	sdindexiter *ii = (sdindexiter*)i->priv;
	ii->index   = index;
	ii->cmp     = o;
	ii->key     = key;
	ii->keysize = keysize;
	ii->v       = NULL;
	ii->pos     = 0;
	if (ii->index->h->count == 1) {
		ii->pos = 0;
		if (ii->index->h->lsnmin == UINT64_MAX &&
		    ii->index->h->lsnmax == 0) {
			/* skip bootstrap node  */
			return 0;
		}
		ii->v = sd_indexpage(ii->index, ii->pos);
		return 0;
	}
	if (ii->key == NULL) {
		switch (ii->cmp) {
		case SR_LT:
		case SR_LTE: ii->pos = ii->index->h->count - 1;
			break;
		case SR_GT:
		case SR_GTE: ii->pos = 0;
			break;
		default:
			assert(0);
		}
	} else {
		ii->pos = sd_indexiter_route(i);
		sdindexpage *p = sd_indexpage(ii->index, ii->pos);
		switch (ii->cmp) {
		case SR_LTE: break;
		case SR_LT: {
			int l = sr_compare(i->r->cmp, sd_indexpage_min(ii->index, p), p->sizemin,
			                   ii->key, ii->keysize);
			if (srunlikely(l == 0))
				ii->pos--;
			break;
		}
		case SR_GTE: break;
		case SR_GT: {
			int r = sr_compare(i->r->cmp, sd_indexpage_max(ii->index, p), p->sizemax,
			                   ii->key, ii->keysize);
			if (srunlikely(r == 0))
				ii->pos++;
			break;
		}
		default: assert(0);
		}
	}
	if (srunlikely(ii->pos == -1 ||
	               ii->pos >= (int)ii->index->h->count))
		return 0;
	ii->v = sd_indexpage(ii->index, ii->pos);
	return 0;
}

static inline void
sd_indexiter_close(sriter *i srunused)
{ }

static inline int
sd_indexiter_has(sriter *i)
{
	sdindexiter *ii = (sdindexiter*)i->priv;
	return ii->v != NULL;
}

static inline void*
sd_indexiter_of(sriter *i)
{
	sdindexiter *ii = (sdindexiter*)i->priv;
	return ii->v;
}

static inline void
sd_indexiter_next(sriter *i)
{
	sdindexiter *ii = (sdindexiter*)i->priv;
	switch (ii->cmp) {
	case SR_LT:
	case SR_LTE:
		ii->pos--;
		break;
	case SR_GT:
	case SR_GTE:
		ii->pos++;
		break;
	default:
		assert(0);
		break;
	}
	if (srunlikely(ii->pos < 0))
		ii->v = NULL;
	else
	if (srunlikely(ii->pos >= (int)ii->index->h->count))
		ii->v = NULL;
	else
		ii->v = sd_indexpage(ii->index, ii->pos);
}

extern sriterif sd_indexiter;

#endif
