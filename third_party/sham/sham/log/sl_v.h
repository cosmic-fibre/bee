#ifndef SL_V_H_
#define SL_V_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct slv slv;

struct slv {
	uint32_t crc;
	uint64_t lsn;
	uint32_t dsn;
	uint32_t size;
	uint8_t  flags;
	uint64_t reserve;
} srpacked;

extern svif sl_vif;

static inline uint32_t
sl_vdsn(sv *v) {
	return ((slv*)v->v)->dsn;
}

#endif
