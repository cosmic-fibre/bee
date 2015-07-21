#ifndef SD_V_H_
#define SD_V_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct sdv sdv;

struct sdv {
	uint64_t lsn;
	uint8_t  flags;
	uint32_t offset;
	uint32_t size;
} srpacked;

extern svif sd_vif;

#endif
