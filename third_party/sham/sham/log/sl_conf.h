#ifndef SL_CONF_H_
#define SL_CONF_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct slconf slconf;

struct slconf {
	int   enable;
	char *path;
	int   sync_on_rotate;
	int   sync_on_write;
	int   rotatewm;
};

#endif
