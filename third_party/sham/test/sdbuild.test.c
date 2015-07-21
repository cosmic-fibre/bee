
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
	svv *v = sv_vbuild(r, &pv, 1, NULL, 0);
	v->lsn = lsn;
	v->flags = flags;
	sv vv;
	sv_init(&vv, &sv_vif, v, NULL);
	sd_buildadd(b, r, &vv, flags & SVDUP);
	sv_vfree(r->a, v);
}

static void
sdbuild_empty(stc *cx srunused)
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
	sd_buildend(&b, &r);
	sdpageheader *h = sd_buildheader(&b);
	t( h->count == 0 );

	sd_buildfree(&b, &r);
	sr_keyfree(&cmp, &a);
}

static void
sdbuild_page0(stc *cx srunused)
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
	int k = 15;
	addv(&b, &r, 3, SVSET, &i);
	addv(&b, &r, 2, SVSET, &j);
	addv(&b, &r, 1, SVSET, &k);
	sd_buildend(&b, &r);
	sdpageheader *h = sd_buildheader(&b);
	t( h->count == 3 );
	t( h->lsnmin == 1 );
	t( h->lsnmax == 3 );
	sd_buildfree(&b, &r);
	sr_keyfree(&cmp, &a);
}

static void
sdbuild_page1(stc *cx srunused)
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
	int k = 15;
	addv(&b, &r, 3, SVSET, &i);
	addv(&b, &r, 2, SVSET, &j);
	addv(&b, &r, 1, SVSET, &k);
	sd_buildend(&b, &r);
	sdpageheader *h = sd_buildheader(&b);
	t( h->count == 3 );
	t( h->lsnmin == 1 );
	t( h->lsnmax == 3 );

	srbuf buf;
	sr_bufinit(&buf);
	t( sd_commitpage(&b, &r, &buf) == 0 );
	h = (sdpageheader*)buf.s;
	sdpage page;
	sd_pageinit(&page, h);

	sdv *min = sd_pagemin(&page);
	t( *(int*)sr_formatkey( sd_pageof(&page, min), 0) == i );
	sdv *max = sd_pagemax(&page);
	t( *(int*)sr_formatkey( sd_pageof(&page, max), 0) == k );
	sd_buildcommit(&b);

	t( sd_buildbegin(&b, &r, 1, 0) == 0);
	j = 19; 
	k = 20;
	addv(&b, &r, 4, SVSET, &j);
	addv(&b, &r, 5, SVSET, &k);
	sd_buildend(&b, &r);
	h = sd_buildheader(&b);
	t( h->count == 2 );
	t( h->lsnmin == 4 );
	t( h->lsnmax == 5 );
	sr_bufreset(&buf);
	t( sd_commitpage(&b, &r, &buf) == 0 );
	h = (sdpageheader*)buf.s;
	sd_pageinit(&page, h);
	min = sd_pagemin(&page);
	t( *(int*)sr_formatkey( sd_pageof(&page, min), 0) == j );
	max = sd_pagemax(&page);
	t( *(int*)sr_formatkey( sd_pageof(&page, max), 0) == k );
	sd_buildcommit(&b);

	sd_buildfree(&b, &r);
	sr_buffree(&buf, &a);
	sr_keyfree(&cmp, &a);
}

static void
sdbuild_compression_zstd(stc *cx srunused)
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
	sr_init(&r, &error, &a, &seq, SR_FKV, &cmp, &ij, crc, &sr_zstdfilter);

	sdbuild b;
	sd_buildinit(&b);
	t( sd_buildbegin(&b, &r, 1, 1) == 0);
	int i = 7;
	int j = 8;
	int k = 15;
	addv(&b, &r, 3, SVSET, &i);
	addv(&b, &r, 2, SVSET, &j);
	addv(&b, &r, 1, SVSET, &k);
	sd_buildend(&b, &r);
	sdpageheader *h = sd_buildheader(&b);
	t( h->count == 3 );
	t( h->lsnmin == 1 );
	t( h->lsnmax == 3 );
	sd_buildcommit(&b);

	t( sd_buildbegin(&b, &r, 1, 1) == 0);
	addv(&b, &r, 3, SVSET, &i);
	addv(&b, &r, 2, SVSET, &j);
	addv(&b, &r, 1, SVSET, &k);
	sd_buildend(&b, &r);
	h = sd_buildheader(&b);
	t( h->count == 3 );
	t( h->lsnmin == 1 );
	t( h->lsnmax == 3 );
	sd_buildcommit(&b);

	sd_buildfree(&b, &r);
	sr_keyfree(&cmp, &a);
}

static void
sdbuild_compression_lz4(stc *cx srunused)
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
	sr_init(&r, &error, &a, &seq, SR_FKV, &cmp, &ij, crc, &sr_lz4filter);

	sdbuild b;
	sd_buildinit(&b);
	t( sd_buildbegin(&b, &r, 1, 1) == 0);
	int i = 7;
	int j = 8;
	int k = 15;
	addv(&b, &r, 3, SVSET, &i);
	addv(&b, &r, 2, SVSET, &j);
	addv(&b, &r, 1, SVSET, &k);
	sd_buildend(&b, &r);
	sdpageheader *h = sd_buildheader(&b);
	t( h->count == 3 );
	t( h->lsnmin == 1 );
	t( h->lsnmax == 3 );
	sd_buildcommit(&b);

	t( sd_buildbegin(&b, &r, 1, 1) == 0);
	addv(&b, &r, 3, SVSET, &i);
	addv(&b, &r, 2, SVSET, &j);
	addv(&b, &r, 1, SVSET, &k);
	sd_buildend(&b, &r);
	h = sd_buildheader(&b);
	t( h->count == 3 );
	t( h->lsnmin == 1 );
	t( h->lsnmax == 3 );
	sd_buildcommit(&b);

	sd_buildfree(&b, &r);
	sr_keyfree(&cmp, &a);
}

stgroup *sdbuild_group(void)
{
	stgroup *group = st_group("sdbuild");
	st_groupadd(group, st_test("empty", sdbuild_empty));
	st_groupadd(group, st_test("page0", sdbuild_page0));
	st_groupadd(group, st_test("page1", sdbuild_page1));
	st_groupadd(group, st_test("compression_zstd", sdbuild_compression_zstd));
	st_groupadd(group, st_test("compression_lz4", sdbuild_compression_lz4));
	return group;
}
