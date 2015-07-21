#ifndef SO_SNAPSHOTCURSOR_H_
#define SO_SNAPSHOTCURSOR_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct sosnapshotcursor sosnapshotcursor;

struct sosnapshotcursor {
	soobj o;
	int ready;
	srbuf list;
	char *pos;
	sodb *v;
	sosnapshot *s;
} srpacked;

soobj *so_snapshotcursor_new(sosnapshot*);

#endif
