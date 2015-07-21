#ifndef SL_H_
#define SL_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct sl sl;
typedef struct slpool slpool;
typedef struct sltx sltx;

struct sl {
	uint32_t id;
	srgc gc;
	srmutex filelock;
	srfile file;
	slpool *p;
	srlist link;
	srlist linkcopy;
};

struct slpool {
	srspinlock lock;
	slconf *conf;
	srlist list;
	int gc;
	int n;
	sriov iov;
	sr *r;
};

struct sltx {
	slpool *p;
	sl *l;
	uint64_t svp;
};

int sl_poolinit(slpool*, sr*);
int sl_poolopen(slpool*, slconf*);
int sl_poolrotate(slpool*);
int sl_poolrotate_ready(slpool*, int);
int sl_poolshutdown(slpool*);
int sl_poolgc_enable(slpool*, int);
int sl_poolgc(slpool*);
int sl_poolfiles(slpool*);
int sl_poolcopy(slpool*, char*, srbuf*);

int sl_begin(slpool*, sltx*);
int sl_prepare(slpool*, svlog*, uint64_t);
int sl_commit(sltx*);
int sl_rollback(sltx*);
int sl_write(sltx*, svlog*);

#endif
