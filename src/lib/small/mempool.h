#ifndef INCLUDES_BEE_SMALL_MEMPOOL_H
#define INCLUDES_BEE_SMALL_MEMPOOL_H
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
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include "small/slab_cache.h"
#include "small/lifo.h"
#define RB_COMPACT 1
#include "third_party/rb.h"

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

/**
 * Pool allocator.
 *
 * Good for allocating tons of small objects of the same size.
 * Stores all objects in order-of-virtual-page-size memory blocks,
 * called slabs. Each object can be freed if necessary. There is
 * (practically) no allocation overhead. Internal fragmentation
 * may occur if lots of objects are allocated, and then many of
 * them are freed in reverse-to-allocation order.
 *
 * Under the hood, uses a slab cache of mmap()-allocated slabs.
 * Slabs of the slab cache are never released back to the
 * operating system.
 *
 * Thread-safety
 * -------------
 * Calls to alloc() and free() on the same mempool instance must
 * be externally synchronized. Use of different instances in
 * different threads is thread-safe (but they must also be based
 * on distinct slab caches).
 *
 * Exception-safety
 * ----------------
 * The only type of failure which can occur is a failure to
 * allocate memory. In case of such error, an exception
 * (OutOfMemory) is raised. _nothrow()
 * version of mempool_alloc() returns NULL rather than raises an
 * error in case of failure.
 */

/** mslab - a standard slab formatted to store objects of equal size. */
struct mslab {
	struct slab slab;
	/* Head of a list of used but freed objects */
	void *free_list;
	/** Index of an object that has never been allocated in mslab */
	uint32_t free_idx;
	/** Number of available slots in the slab. */
	uint32_t nfree;
	/** Used if this slab is a member of free_slabs tree. */
	rb_node(struct mslab) node;
	/* Reference to the owning pool. */
	struct mempool *pool;
};

/**
 * Mempool will try to allocate blocks large enough to ensure
 * the overhead from internal fragmentation is less than the
 * specified below.
 */
static const double OVERHEAD_RATIO = 0.01;

static inline uint32_t
mslab_sizeof()
{
	return small_align(sizeof(struct mslab), sizeof(intptr_t));
}

/**
 * Calculate the maximal size of an object for which it makes
 * sense to create a memory pool given the size of the slab.
 */
static inline uint32_t
mempool_objsize_max(uint32_t slab_size)
{
	/* Fit at least 4 objects in a slab, aligned by pointer size. */
	return ((slab_size - mslab_sizeof()) / 4) &
		 ~(sizeof(intptr_t) - 1);
}

typedef rb_tree(struct mslab) mslab_tree_t;

/** A memory pool. */
struct mempool
{
	/**
	 * A link in delayed free list of pools. Must be the first
	 * member in the struct.
	 * @sa smfree_delayed().
	 */
	struct lifo link;
	/** List of pointers for delayed free. */
	struct lifo delayed;
	/** The source of empty slabs. */
	struct slab_cache *cache;
	/** All slabs. */
	struct slab_list slabs;
	/**
	 * Slabs with some amount of free space available are put
	 * into this red-black tree, which is sorted by slab
	 * address. A (partially) free slab with the smallest
	 * address is chosen for allocation. This reduces internal
	 * memory fragmentation across many slabs.
	 */
	mslab_tree_t free_slabs;
	/**
	 * A completely empty slab which is not freed only to
	 * avoid the overhead of slab_cache oscillation around
	 * a single element allocation.
	 */
	struct mslab *spare;
	/**
	 * The size of an individual object. All objects
	 * allocated on the pool have the same size.
	 */
	uint32_t objsize;
	/**
	 * Mempool slabs are ordered (@sa slab_cache.h for
	 * definition of "ordered"). The order is calculated
	 * when the pool is initialized or is set explicitly.
	 * The latter is necessary for 'small' allocator,
	 * which needs to quickly find mempool containing
	 * an allocated object when the object is freed.
	 */
	uint8_t slab_order;
	/** How many objects can fit in a slab. */
	uint32_t objcount;
	/** Offset from beginning of slab to the first object */
	uint32_t objoffset;
};

/** Allocation statistics. */
struct mempool_stats
{
	/** Object size. */
	uint32_t objsize;
	/** Total objects allocated. */
	uint32_t objcount;
	/** Size of the slab. */
	uint32_t slabsize;
	/** Number of slabs. All slabs are of the same size. */
	uint32_t slabcount;
	/** Memory used and booked but passive (to see fragmentation). */
	struct small_stats totals;
};

void
mempool_stats(struct mempool *mempool, struct mempool_stats *stats);

/** @todo: struct mempool_iterator */

void
mempool_create_with_order(struct mempool *pool, struct slab_cache *cache,
			  uint32_t objsize, uint8_t order);

/**
 * Initialize a mempool. Tell the pool the size of objects
 * it will contain.
 *
 * objsize must be >= sizeof(mbitmap_t)
 * If allocated objects must be aligned, then objsize must
 * be aligned. The start of free area in a slab is always
 * uint64_t aligned.
 *
 * @sa mempool_destroy()
 */
static inline void
mempool_create(struct mempool *pool, struct slab_cache *cache,
	       uint32_t objsize)
{
	size_t overhead = (objsize > sizeof(struct mslab) ?
			   objsize : sizeof(struct mslab));
	size_t slab_size = (size_t) (overhead / OVERHEAD_RATIO);
	if (slab_size > cache->arena->slab_size)
		slab_size = cache->arena->slab_size;
	/*
	 * Calculate the amount of usable space in a slab.
	 * @note: this asserts that slab_size_min is less than
	 * SLAB_ORDER_MAX.
	 */
	uint8_t order = slab_order(cache, slab_size);
	assert(order <= cache->order_max);
	return mempool_create_with_order(pool, cache, objsize, order);
}

/**
 * Free the memory pool and release all cached memory blocks.
 * @sa mempool_create()
 */
void
mempool_destroy(struct mempool *pool);

/** Allocate an object. */
void *
mempool_alloc_nothrow(struct mempool *pool);

void
mslab_free(struct mempool *pool, struct mslab *slab, void *ptr);

/**
 * Free a single object.
 * @pre the object is allocated in this pool.
 */
static inline void
mempool_free(struct mempool *pool, void *ptr)
{
#ifndef NDEBUG
	memset(ptr, '#', pool->objsize);
#endif
	struct mslab *slab = (struct mslab *)
		slab_from_ptr(pool->cache, ptr, pool->slab_order);
	pool->slabs.stats.used -= pool->objsize;
	mslab_free(pool, slab, ptr);
}


/** How much memory is used by this pool. */
static inline size_t
mempool_used(struct mempool *pool)
{
	return pool->slabs.stats.used;
}


/** How much memory is held by this pool. */
static inline size_t
mempool_total(struct mempool *pool)
{
	return pool->slabs.stats.total;
}

#if defined(__cplusplus)
} /* extern "C" */
#include "exception.h"

static inline void *
mempool_alloc(struct mempool *pool)
{
	void *ptr = mempool_alloc_nothrow(pool);
	if (ptr == NULL)
		tnt_raise(OutOfMemory, pool->objsize,
			  "mempool", "new slab");
	return ptr;
}

static inline void *
mempool_alloc0(struct mempool *pool)
{
	return memset(mempool_alloc(pool), 0, pool->objsize);
}

#endif /* defined(__cplusplus) */

#endif /* INCLUDES_BEE_SMALL_MEMPOOL_H */
