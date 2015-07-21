
/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <libsr.h>
#include <libst.h>

static void
srrq_test0(stc *cx srunused)
{
	sra a;
	sr_aopen(&a, &sr_stda);
	srrq q;
	t( sr_rqinit(&q, &a, 1, 10) == 0 );

	sr_rqfree(&q, &a);
}

static void
srrq_test1(stc *cx srunused)
{
	sra a;
	sr_aopen(&a, &sr_stda);
	srrq q;
	t( sr_rqinit(&q, &a, 1, 10) == 0 );

	srrqnode *an = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *bn = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *cn = sr_malloc(&a, sizeof(srrqnode));
	t( an != NULL );
	t( bn != NULL );
	t( cn != NULL );
	sr_rqinitnode(an);
	sr_rqinitnode(bn);
	sr_rqinitnode(cn);

	sr_rqadd(&q, an, 1);
	sr_rqadd(&q, bn, 2);
	sr_rqadd(&q, cn, 3);
	int i = 3;
	srrqnode *n = NULL;
	while ((n = sr_rqprev(&q, n))) {
		t( i == n->v );
		i--;
	}
	sr_rqfree(&q, &a);

	sr_free(&a, an);
	sr_free(&a, bn);
	sr_free(&a, cn);
}

static void
srrq_test2(stc *cx srunused)
{
	sra a;
	sr_aopen(&a, &sr_stda);
	srrq q;
	t( sr_rqinit(&q, &a, 1, 10) == 0 );

	srrqnode *an = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *bn = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *cn = sr_malloc(&a, sizeof(srrqnode));
	t( an != NULL );
	t( bn != NULL );
	t( cn != NULL );
	sr_rqinitnode(an);
	sr_rqinitnode(bn);
	sr_rqinitnode(cn);

	sr_rqadd(&q, an, 1);
	sr_rqadd(&q, bn, 2);
	sr_rqadd(&q, cn, 3);
	sr_rqdelete(&q, cn);
	int i = 2;
	srrqnode *n = NULL;
	while ((n = sr_rqprev(&q, n))) {
		t( i == n->v );
		i--;
	}
	sr_rqfree(&q, &a);
	sr_free(&a, an);
	sr_free(&a, bn);
	sr_free(&a, cn);
}

static void
srrq_test3(stc *cx srunused)
{
	sra a;
	sr_aopen(&a, &sr_stda);
	srrq q;
	t( sr_rqinit(&q, &a, 1, 10) == 0 );

	srrqnode *an = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *bn = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *cn = sr_malloc(&a, sizeof(srrqnode));
	t( an != NULL );
	t( bn != NULL );
	t( cn != NULL );
	sr_rqinitnode(an);
	sr_rqinitnode(bn);
	sr_rqinitnode(cn);

	sr_rqadd(&q, an, 1);
	sr_rqadd(&q, bn, 2);
	sr_rqdelete(&q, bn);
	sr_rqadd(&q, cn, 3);
	int i = 3;
	srrqnode *n = NULL;
	while ((n = sr_rqprev(&q, n))) {
		t( i == n->v );
		i = 1;
	}
	sr_rqfree(&q, &a);
	sr_free(&a, an);
	sr_free(&a, bn);
	sr_free(&a, cn);
}

static void
srrq_test4(stc *cx srunused)
{
	sra a;
	sr_aopen(&a, &sr_stda);
	srrq q;
	t( sr_rqinit(&q, &a, 1, 10) == 0 );

	srrqnode *an = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *bn = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *cn = sr_malloc(&a, sizeof(srrqnode));
	t( an != NULL );
	t( bn != NULL );
	t( cn != NULL );
	sr_rqinitnode(an);
	sr_rqinitnode(bn);
	sr_rqinitnode(cn);

	sr_rqadd(&q, an, 1);
	sr_rqdelete(&q, an);
	sr_rqadd(&q, bn, 2);
	sr_rqadd(&q, cn, 3);
	int i = 3;
	srrqnode *n = NULL;
	while ((n = sr_rqprev(&q, n))) {
		t( i == n->v );
		i--;
	}
	sr_rqfree(&q, &a);
	sr_free(&a, an);
	sr_free(&a, bn);
	sr_free(&a, cn);
}

static void
srrq_test5(stc *cx srunused)
{
	sra a;
	sr_aopen(&a, &sr_stda);
	srrq q;
	t( sr_rqinit(&q, &a, 1, 10) == 0 );

	srrqnode *an = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *bn = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *cn = sr_malloc(&a, sizeof(srrqnode));
	t( an != NULL );
	t( bn != NULL );
	t( cn != NULL );
	sr_rqinitnode(an);
	sr_rqinitnode(bn);
	sr_rqinitnode(cn);

	sr_rqadd(&q, an, 1);
	sr_rqadd(&q, bn, 2);
	sr_rqadd(&q, cn, 3);
	sr_rqupdate(&q, cn, 8);
	int i = 8;
	srrqnode *n = NULL;
	while ((n = sr_rqprev(&q, n))) {
		t( i == n->v );
		if (i == 8)
			i = 2;
		else
			i = 1;
	}
	sr_rqfree(&q, &a);
	sr_free(&a, an);
	sr_free(&a, bn);
	sr_free(&a, cn);
}

static void
srrq_test6(stc *cx srunused)
{
	sra a;
	sr_aopen(&a, &sr_stda);
	srrq q;
	t( sr_rqinit(&q, &a, 1, 10) == 0 );

	srrqnode *an = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *bn = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *cn = sr_malloc(&a, sizeof(srrqnode));
	t( an != NULL );
	t( bn != NULL );
	t( cn != NULL );
	sr_rqinitnode(an);
	sr_rqinitnode(bn);
	sr_rqinitnode(cn);

	sr_rqadd(&q, an, 1);
	sr_rqadd(&q, bn, 2);
	sr_rqadd(&q, cn, 3);
	sr_rqupdate(&q, cn, 8);
	sr_rqdelete(&q, bn);
	sr_rqdelete(&q, an);
	int i = 8;
	srrqnode *n = NULL;
	while ((n = sr_rqprev(&q, n))) {
		t( i == n->v );
		i--;
	}
	sr_rqfree(&q, &a);
	sr_free(&a, an);
	sr_free(&a, bn);
	sr_free(&a, cn);
}

static void
srrq_test7(stc *cx srunused)
{
	sra a;
	sr_aopen(&a, &sr_stda);
	srrq q;
	t( sr_rqinit(&q, &a, 1, 10) == 0 );

	srrqnode *an = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *bn = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *cn = sr_malloc(&a, sizeof(srrqnode));
	t( an != NULL );
	t( bn != NULL );
	t( cn != NULL );
	sr_rqinitnode(an);
	sr_rqinitnode(bn);
	sr_rqinitnode(cn);

	sr_rqadd(&q, an, 1);
	sr_rqadd(&q, bn, 2);
	sr_rqadd(&q, cn, 3);
	sr_rqdelete(&q, bn);
	sr_rqdelete(&q, cn);
	sr_rqdelete(&q, an);
	int i = 0;
	srrqnode *n = NULL;
	while ((n = sr_rqprev(&q, n))) {
		i++;
	}
	t( i == 0 );
	sr_rqfree(&q, &a);
	sr_free(&a, an);
	sr_free(&a, bn);
	sr_free(&a, cn);
}

static void
srrq_test8(stc *cx srunused)
{
	sra a;
	sr_aopen(&a, &sr_stda);
	srrq q;
	t( sr_rqinit(&q, &a, 10, 100) == 0 );

	srrqnode *an = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *bn = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *cn = sr_malloc(&a, sizeof(srrqnode));
	srrqnode *dn = sr_malloc(&a, sizeof(srrqnode));
	t( an != NULL );
	t( bn != NULL );
	t( cn != NULL );
	t( dn != NULL );
	sr_rqinitnode(an);
	sr_rqinitnode(bn);
	sr_rqinitnode(cn);
	sr_rqinitnode(dn);

	sr_rqadd(&q, an, 0);
	sr_rqadd(&q, bn, 10);
	sr_rqadd(&q, cn, 20);
	sr_rqadd(&q, dn, 30);
	int i = 30;
	srrqnode *n = NULL;
	while ((n = sr_rqprev(&q, n)))
		i -= 10;
	sr_rqfree(&q, &a);
	sr_free(&a, an);
	sr_free(&a, bn);
	sr_free(&a, cn);
	sr_free(&a, dn);
}

stgroup *srrq_group(void)
{
	stgroup *group = st_group("srrq");
	st_groupadd(group, st_test("test0", srrq_test0));
	st_groupadd(group, st_test("test1", srrq_test1));
	st_groupadd(group, st_test("test2", srrq_test2));
	st_groupadd(group, st_test("test3", srrq_test3));
	st_groupadd(group, st_test("test4", srrq_test4));
	st_groupadd(group, st_test("test5", srrq_test5));
	st_groupadd(group, st_test("test6", srrq_test6));
	st_groupadd(group, st_test("test7", srrq_test7));
	st_groupadd(group, st_test("test8", srrq_test8));
	return group;
}
