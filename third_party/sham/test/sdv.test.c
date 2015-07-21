
/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <libsr.h>
#include <libsv.h>
#include <libsd.h>
#include <libst.h>
#include <sham.h>

static void
addv(sdbuild *b, sr *r, uint64_t lsn, uint8_t flags, int *key)
{
	srformatv pv;
	pv.key = (char*)key;
	pv.r.size = sizeof(uint32_t);
	pv.r.offset = 0;
	svv *v = sv_vbuild(r, &pv, 1, (char*)key, sizeof(uint32_t));
	v->lsn = lsn;
	v->flags = flags;
	sv vv;
	sv_init(&vv, &sv_vif, v, NULL);
	sd_buildadd(b, r, &vv, flags & SVDUP);
	sv_vfree(r->a, v);
}

static void
sdv_test(stc *cx srunused)
{
	sra a;
	sr_aopen(&a, &sr_stda);
	srkey cmp;
	sr_keyinit(&cmp);
	srkeypart *part = sr_keyadd(&cmp, &a);
	t( sr_keypart_setname(part, &a, "key") == 0 );
	t( sr_keypart_set(part, &a, "u32") == 0 );
	srinjection ij;
	memset(&ij, 0, sizeof(ij));
	srerror error;
	sr_errorinit(&error);
	srseq seq;
	sr_seqinit(&seq);
	srcrcf crc = sr_crc32c_function();
	sr r;
	sr_init(&r, &error, &a, &seq, SR_FKV, &cmp, &ij, crc, NULL);

	sdbuild b;
	sd_buildinit(&b);
	t( sd_buildbegin(&b, &r, 1, 0) == 0);
	int i = 7;
	int j = 8;
	addv(&b, &r, 3, SVSET, &i);
	addv(&b, &r, 4, SVSET, &j);
	sd_buildend(&b, &r);

	srbuf buf;
	sr_bufinit(&buf);
	t( sd_commitpage(&b, &r, &buf) == 0 );
	sdpageheader *h = (sdpageheader*)buf.s;
	sdpage page;
	sd_pageinit(&page, h);

	sriter it;
	sr_iterinit(sd_pageiter, &it, &r);
	sr_iteropen(sd_pageiter, &it, &page, SR_GTE, NULL, 0, UINT64_MAX);
	t( sr_iteratorhas(&it) != 0 );
	sv *v = sr_iteratorof(&it);
	t( v != NULL );
	t( v->i == &sd_vif );

	t( *(int*)sv_key(v, &r, 0) == i );
	sr_iteratornext(&it);
	t( sr_iteratorhas(&it) != 0 );

	v = sr_iteratorof(&it);
	t( v != NULL );
	t( v->i == &sd_vif );
	
	t( *(int*)sv_key(v, &r, 0) == j );
	t( sv_lsn(v) == 4 );
	t( sv_flags(v) == SVSET );

	sd_buildfree(&b, &r);
	sr_buffree(&buf, &a);
	sr_keyfree(&cmp, &a);
}

stgroup *sdv_group(void)
{
	stgroup *group = st_group("sdv");
	st_groupadd(group, st_test("test", sdv_test));
	return group;
}
