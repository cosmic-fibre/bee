#ifndef SD_INDEX_H_
#define SD_INDEX_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct sdindexheader sdindexheader;
typedef struct sdindexpage sdindexpage;
typedef struct sdindex sdindex;

struct sdindexheader {
	uint32_t  crc;
	srversion version;
	sdid      id;
	uint64_t  offset;
	uint32_t  size;
	uint32_t  count;
	uint32_t  keys;
	uint64_t  total;
	uint64_t  totalorigin;
	uint32_t  tsmin;
	uint64_t  lsnmin;
	uint64_t  lsnmax;
	uint32_t  dupkeys;
	uint64_t  dupmin;
	uint32_t  extension;
	char      reserve[32];
} srpacked;

struct sdindexpage {
	uint64_t offset;
	uint32_t offsetindex;
	uint32_t size;
	uint32_t sizeorigin;
	uint16_t sizemin;
	uint16_t sizemax;
	uint64_t lsnmin;
	uint64_t lsnmax;
} srpacked;

struct sdindex {
	srbuf i, v;
	sdindexheader *h;
};

static inline char*
sd_indexpage_min(sdindex *i, sdindexpage *p) {
	return (char*)i->i.s + sizeof(sdindexheader) +
	             (i->h->count * sizeof(sdindexpage)) + p->offsetindex;
}

static inline char*
sd_indexpage_max(sdindex *i, sdindexpage *p) {
	return sd_indexpage_min(i, p) + p->sizemin;
}

static inline void
sd_indexinit(sdindex *i) {
	sr_bufinit(&i->i);
	sr_bufinit(&i->v);
	i->h = NULL;
}

static inline void
sd_indexfree(sdindex *i, sr *r) {
	sr_buffree(&i->i, r->a);
	sr_buffree(&i->v, r->a);
}

static inline sdindexheader*
sd_indexheader(sdindex *i) {
	return (sdindexheader*)(i->i.s);
}

static inline sdindexpage*
sd_indexpage(sdindex *i, uint32_t pos)
{
	assert(pos < i->h->count);
	char *p = (char*)sr_bufat(&i->i, sizeof(sdindexpage), pos);
   	p += sizeof(sdindexheader);
	return (sdindexpage*)p;
}

static inline sdindexpage*
sd_indexmin(sdindex *i) {
	return sd_indexpage(i, 0);
}

static inline sdindexpage*
sd_indexmax(sdindex *i) {
	return sd_indexpage(i, i->h->count - 1);
}

static inline uint32_t
sd_indexkeys(sdindex *i)
{
	if (srunlikely(i->i.s == NULL))
		return 0;
	return sd_indexheader(i)->keys;
}

static inline uint32_t
sd_indextotal(sdindex *i)
{
	if (srunlikely(i->i.s == NULL))
		return 0;
	return sd_indexheader(i)->total;
}

static inline uint32_t
sd_indexsize(sdindexheader *h)
{
	return sizeof(sdindexheader) + h->size;
}

static inline int
sd_indexpage_cmp(sdindex *i, sdindexpage *p, void *key, int size, srkey *c)
{
	int l = sr_compare(c, sd_indexpage_min(i, p), p->sizemin, key, size);
	int r = sr_compare(c, sd_indexpage_max(i, p), p->sizemax, key, size);
	/* inside page range */
	if (l <= 0 && r >= 0)
		return 0;
	/* key > page */
	if (l == -1)
		return -1;
	/* key < page */
	assert(r == 1);
	return 1;
}

int sd_indexbegin(sdindex*, sr*, uint64_t);
int sd_indexcommit(sdindex*, sr*, sdid*);
int sd_indexadd(sdindex*, sr*, sdbuild*);
int sd_indexcopy(sdindex*, sr*, sdindexheader*);

#endif
