#ifndef INCLUDES_BEE_DB_LUA_INDEX_H
#define INCLUDES_BEE_DB_LUA_INDEX_H
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
#include <stdint.h>
#include <sys/types.h> /* ssize_t */

struct lua_State;
struct iterator;

void
db_lua_index_init(struct lua_State *L);

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

size_t
dbffi_index_len(uint32_t space_id, uint32_t index_id);

struct tuple *
dbffi_index_random(uint32_t space_id, uint32_t index_id, uint32_t rnd);

struct tuple *
dbffi_index_get(uint32_t space_id, uint32_t index_id, const char *key);

struct tuple *
dbffi_index_min(uint32_t space_id, uint32_t index_id, const char *key);

struct tuple *
dbffi_index_max(uint32_t space_id, uint32_t index_id, const char *key);

ssize_t
dbffi_index_count(uint32_t space_id, uint32_t index_id, int type,
		   const char *key);

struct iterator *
dbffi_index_iterator(uint32_t space_id, uint32_t index_id, int type,
		      const char *key);

size_t
dbffi_index_bsize(uint32_t space_id, uint32_t index_id);

struct tuple*
dbffi_iterator_next(struct iterator *itr);

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */

#endif /* INCLUDES_BEE_DB_LUA_INDEX_H */
