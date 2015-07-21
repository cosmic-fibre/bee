#ifndef SR_H_
#define SR_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct sr sr;

struct sr {
	srerror *e;
	srformat format;
	srkey *cmp;
	srseq *seq;
	sra *a;
	srinjection *i;
	void *compression;
	srcrcf crc;
};

static inline void
sr_init(sr *r,
        srerror *e,
        sra *a,
        srseq *seq,
        srformat format,
        srkey *cmp,
        srinjection *i,
        srcrcf crc,
        void *compression)
{
	r->e           = e;
	r->a           = a;
	r->seq         = seq;
	r->format      = format;
	r->cmp         = cmp;
	r->i           = i;
	r->compression = compression;
	r->crc         = crc;
}

#endif
