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
#include <stdint.h>
#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#if !MH_SOURCE
#define MH_UNDEF
#endif

/*
 * Map: (i32) => (void *)
 */
#define mh_name _i32ptr
#define mh_key_t uint32_t
struct mh_i32ptr_node_t {
	mh_key_t key;
	void *val;
};

#define mh_node_t struct mh_i32ptr_node_t
#define mh_arg_t void *
#define mh_hash(a, arg) (a->key)
#define mh_hash_key(a, arg) (a)
#define mh_eq(a, b, arg) ((a->key) == (b->key))
#define mh_eq_key(a, b, arg) ((a) == (b->key))
#include "salad/mhash.h"

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */
