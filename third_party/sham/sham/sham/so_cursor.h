#ifndef SO_CURSOR_H_
#define SO_CURSOR_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct socursor socursor;

struct socursor {
	soobj o;
	int ready;
	srorder order;
	sx t;
	sicache cache;
	sov v;
	soobj *key;
	sodb *db;
} srpacked;

soobj *so_cursornew(sodb*, uint64_t, va_list);

#endif
