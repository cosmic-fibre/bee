#ifndef SI_RECOVER_H_
#define SI_RECOVER_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

sinode *si_bootstrap(si*, sr*, uint32_t);
int si_recover(si*, sr*);

#endif
