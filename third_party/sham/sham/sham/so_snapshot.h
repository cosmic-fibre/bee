#ifndef SO_SNAPSHOT_H_
#define SO_SNAPSHOT_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct sosnapshot sosnapshot;

struct sosnapshot {
	soobj o;
	sx t;
	soobjindex cursor;
	uint64_t vlsn;
	char *name;
} srpacked;

soobj *so_snapshotnew(so*, uint64_t, char*);
int    so_snapshotupdate(sosnapshot*);

#endif
