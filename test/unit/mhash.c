#include <stdint.h>
#include <stdio.h>
#include "unit.h"

#ifndef bytemap
#define bytemap 0
#endif
#define MH_SOURCE 1


#define mh_name _i32
struct mh_i32_node_t {
	int32_t key;
	int32_t val;
};
#define mh_node_t struct mh_i32_node_t
#define mh_arg_t void *
#define mh_hash(a, arg) (a->key)
#define mh_eq(a, b, arg) ((a->key) == (b->key))
#define mh_bytemap bytemap
#include "salad/mhash.h"

#define mh_name _i32_collision
struct mh_i32_collision_node_t {
	int32_t key;
	int32_t val;
};
#define mh_node_t struct mh_i32_collision_node_t
#define mh_arg_t void *
#define mh_hash(a, arg) 42
#define mh_eq(a, b, arg) ((a->key) == (b->key))

#define mh_bytemap bytemap
#include "salad/mhash.h"

#undef MH_SOURCE

static void mhash_int32_id_test()
{
	header();
	int k;
	struct mh_i32_t *h;
#define init()		({ mh_i32_new();		})
#define clear(x)	({ mh_i32_clear((x));		})
#define destroy(x)	({ mh_i32_delete((x));		})
#define get(x) ({							\
	const struct mh_i32_node_t _node = { .key = (x) };		\
	mh_i32_get(h, &_node, NULL);					\
})
#define put(x) ({							\
	const struct mh_i32_node_t _node = { .key = (x) };		\
	mh_i32_put(h, &_node, NULL, NULL);				\
})
#define key(k) (mh_i32_node(h, k)->key)
#define val(k) (mh_i32_node(h, k)->val)
#define del(k) ({							\
	mh_i32_del(h, k, NULL);						\
})

#include "mhash_body.c"
	footer();
}


static void mhash_int32_collision_test()
{
	header();
	int k;
	struct mh_i32_collision_t *h;
#define init()		({ mh_i32_collision_new();		})
#define clear(x)	({ mh_i32_collision_clear((x));	})
#define destroy(x)	({ mh_i32_collision_delete((x));	})
#define get(x) ({							\
	const struct mh_i32_collision_node_t _node = { .key = (x) };	\
	mh_i32_collision_get(h, &_node, NULL);				\
})
#define put(x) ({							\
	const struct mh_i32_collision_node_t _node = { .key = (x) };	\
	mh_i32_collision_put(h, &_node, NULL, NULL);			\
})
#define key(k) (mh_i32_collision_node(h, k)->key)
#define val(k) (mh_i32_collision_node(h, k)->val)
#define del(k) ({							\
	mh_i32_collision_del(h, k, NULL);				\
})

#include "mhash_body.c"
	footer();
}

int main(void)
{
	mhash_int32_id_test();
	mhash_int32_collision_test();
	return 0;
}
