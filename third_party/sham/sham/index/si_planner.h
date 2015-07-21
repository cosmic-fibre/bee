#ifndef SI_PLANNER_H_
#define SI_PLANNER_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct siplanner siplanner;
typedef struct siplan siplan;

struct siplanner {
	srrq branch;
	srrq compact;
};

/* plan */
#define SI_BRANCH        1
#define SI_AGE           2
#define SI_COMPACT       4
#define SI_CHECKPOINT    8
#define SI_GC            16
#define SI_BACKUP        32
#define SI_SHUTDOWN      64
#define SI_DROP          128

/* explain */
#define SI_ENONE         0
#define SI_ERETRY        1
#define SI_EINDEX_SIZE   2
#define SI_EINDEX_AGE    4
#define SI_EBRANCH_COUNT 3

struct siplan {
	int explain;
	int plan;
	/* branch:
	 *   a: index_size
	 *   b: ttl
	 *   c: ttl_wm
	 * age:
	 *   a: ttl
	 *   b: ttl_wm
	 *   c:
	 * compact:
	 *   a: branches
	 *   b:
	 *   c:
	 * checkpoint:
	 *   a: lsn
	 *   b:
	 *   c:
	 * gc:
	 *   a: lsn
	 *   b: percent
	 *   c:
	 * backup:
	 *   a: bsn
	 *   b:
	 *   c:
	 * shutdown:
	 * drop:
	 */
	uint64_t a, b, c;
	sinode *node;
};

int si_planinit(siplan*);
int si_plannerinit(siplanner*, sra*);
int si_plannerfree(siplanner*, sra*);
int si_plannertrace(siplan*, srtrace*);
int si_plannerupdate(siplanner*, int, sinode*);
int si_plannerremove(siplanner*, int, sinode*);
int si_planner(siplanner*, siplan*);

#endif
