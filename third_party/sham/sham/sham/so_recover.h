#ifndef SO_RECOVER_H_
#define SO_RECOVER_H_

/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

int so_recoverbegin(sodb*);
int so_recoverend(sodb*);
int so_recover(so*);
int so_recover_repository(so*);

#endif
