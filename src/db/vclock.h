#ifndef INCLUDES_BEE_VCLOCK_H
#define INCLUDES_BEE_VCLOCK_H
/*
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>

#define RB_COMPACT 1
#include <third_party/rb.h>

#include "bit/bit.h"

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

enum { VCLOCK_MAX = 16 };

/** Cluster vector clock */
struct vclock {
	/** Map of used components in lsn array */
	unsigned int map;
	/** Sum of all components of vclock. */
	int64_t signature;
	int64_t lsn[VCLOCK_MAX];
	/** To order binary logs by vector clock. */
	rb_node(struct vclock) link;
};

/* Server id, coordinate */
struct vclock_c {
	uint32_t id;
	int64_t lsn;
};

struct vclock_iterator
{
	struct bit_iterator it;
	const struct vclock *vclock;
};

static inline void
vclock_iterator_init(struct vclock_iterator *it, const struct vclock *vclock)
{
	it->vclock = vclock;
	bit_iterator_init(&it->it, &vclock->map, sizeof(vclock->map), true);
}

static inline struct vclock_c
vclock_iterator_next(struct vclock_iterator *it)
{
	struct vclock_c c;
	size_t id = bit_iterator_next(&it->it);
	c.id = id == SIZE_MAX ? (int) VCLOCK_MAX : id;
	if (c.id < VCLOCK_MAX)
		c.lsn = it->vclock->lsn[c.id];
	return c;
}


#define vclock_foreach(it, var) \
	for (struct vclock_c (var) = vclock_iterator_next(it); \
	     (var).id < VCLOCK_MAX; (var) = vclock_iterator_next(it))

static inline void
vclock_create(struct vclock *vclock)
{
	memset(vclock, 0, sizeof(*vclock));
}

static inline bool
vclock_has(const struct vclock *vclock, uint32_t server_id)
{
	return server_id < VCLOCK_MAX && (vclock->map & (1 << server_id));
}

static inline int64_t
vclock_get(const struct vclock *vclock, uint32_t server_id)
{
	return vclock_has(vclock, server_id) ? vclock->lsn[server_id] : -1;
}

static inline int64_t
vclock_inc(struct vclock *vclock, uint32_t server_id)
{
	assert(vclock_has(vclock, server_id));
	vclock->signature++;
	return ++vclock->lsn[server_id];
}

static inline void
vclock_copy(struct vclock *dst, const struct vclock *src)
{
	*dst = *src;
}

static inline uint32_t
vclock_size(const struct vclock *vclock)
{
	return __builtin_popcount(vclock->map);
}

static inline int64_t
vclock_sum(const struct vclock *vclock)
{
	int64_t sum = 0;
	struct vclock_iterator it;
	vclock_iterator_init(&it, vclock);
	vclock_foreach(&it, server)
		sum += server.lsn;
	return sum;
}

static inline int64_t
vclock_signature(const struct vclock *vclock)
{
	return vclock->signature;
}

static inline void
vclock_add_server_nothrow(struct vclock *vclock, uint32_t server_id)
{
	vclock->map |= 1 << server_id;
}

int64_t
vclock_follow(struct vclock *vclock, uint32_t server_id, int64_t lsn);

/**
 * \brief Format vclock to YAML-compatible string representation:
 * { server_id: lsn, server_id:lsn })
 * \param vclock vclock
 * \return fomatted string. This pointer should be passed to free(3) to
 * release the allocated storage when it is no longer needed.
 */
char *
vclock_to_string(const struct vclock *vclock);

/**
 * \brief Fill vclock from string representation.
 * \param vclock vclock
 * \param str string to parse
 * \retval 0 on sucess
 * \retval error offset on error (indexed from 1)
 * \sa vclock_to_string()
 */
size_t
vclock_from_string(struct vclock *vclock, const char *str);

enum { VCLOCK_ORDER_UNDEFINED = INT_MAX };

/**
 * \brief Compare vclocks
 * \param a vclock
 * \param b vclock
 * \retval 1 if \a vclock is ordered after \a other
 * \retval -1 if \a vclock is ordered before than \a other
 * \retval 0 if vclocks are equal
 * \retval VCLOCK_ORDER_UNDEFINED if vclocks are concurrent
 */
static inline int
vclock_compare(const struct vclock *a, const struct vclock *b)
{
	bool le = true, ge = true;
	unsigned int map = a->map | b->map;
	struct bit_iterator it;
	bit_iterator_init(&it, &map, sizeof(map), true);

	for (size_t server_id = bit_iterator_next(&it); server_id < VCLOCK_MAX;
	     server_id = bit_iterator_next(&it)) {

		int64_t lsn_a = a->lsn[server_id];
		int64_t lsn_b = b->lsn[server_id];
		le = le && lsn_a <= lsn_b;
		ge = ge && lsn_a >= lsn_b;
		if (!ge && !le)
			return VCLOCK_ORDER_UNDEFINED;
	}
	if (ge && !le)
		return 1;
	if (le && !ge)
		return -1;
	return 0;
}

/**
 * @brief vclockset - a set of vclocks
 */
typedef rb_tree(struct vclock) vclockset_t;
rb_proto(, vclockset_, vclockset_t, struct vclock);

/**
 * A proximity search in a set of vclock objects.
 *
 * The set is normally the index of vclocks in the binary
 * log files of the current directory. The task of the search is
 * to find the first log,
 *
 * @return a vclock that <= than \a key
 */
static inline struct vclock *
vclockset_match(vclockset_t *set, struct vclock *key,
		bool panic_if_error)
{
	struct vclock *match = vclockset_psearch(set, key);
	/**
	 * vclockset comparator returns 0 for
	 * incomparable keys. So the match doesn't have to be
	 * strictly preceding the search key, it may be
	 * incomparable. If this is the case, unwind until
	 * we get to a key which is strictly below the search
	 * pattern.
	 */
	while (match != NULL) {
		if (vclock_compare(match, key) <= 0)
			return match;
		/* The order is undefined, try the previous vclock. */
		match = vclockset_prev(set, match);
	}
	/*
	 * There is no xlog which is strictly less than the search
	 * pattern. Return the fist successor log - it is either
	 * strictly greater, or incomparable with the key.
	 */
	return panic_if_error ? NULL: vclockset_first(set);
}

#if defined(__cplusplus)
} /* extern "C" */

#include "error.h"

/**
 * Allocate a new vclock object and initialize it
 *
 * @param src source vclock to use for initialization (can be
 *            NULL, in that case a new object is created).
 */
static inline struct vclock *
vclock_dup(struct vclock *src)
{
	struct vclock *vclock = (struct vclock *) malloc(sizeof(*vclock));
	if (vclock == NULL) {
		tnt_raise(ClientError, ER_MEMORY_ISSUE,
			  sizeof(*vclock), "vclock", "malloc");
	}
	vclock_copy(vclock, src);
	return vclock;
}

static inline void
vclock_add_server(struct vclock *vclock, uint32_t server_id)
{
	if (server_id >= VCLOCK_MAX)
		tnt_raise(ClientError, ER_REPLICA_MAX, server_id);
	assert(! vclock_has(vclock, server_id));
	vclock_add_server_nothrow(vclock, server_id);
}

static inline void
vclock_del_server(struct vclock *vclock, uint32_t server_id)
{
	assert(vclock_has(vclock, server_id));
	vclock->lsn[server_id] = 0;
	vclock->map &= ~(1 << server_id);
	vclock->signature = vclock_sum(vclock);
}

#endif /* defined(__cplusplus) */

#endif /* INCLUDES_BEE_VCLOCK_H */
