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
#include "user.h"
#include "user_def.h"
#include "session.h"

static char zero_hash[SCRAMBLE_SIZE];

void
authenticate(const char *user_name, uint32_t len,
	     const char *tuple, const char * /* tuple_end */)
{
	struct user *user = user_cache_find_by_name(user_name, len);
	struct session *session = current_session();
	uint32_t part_count;
	uint32_t scramble_len;
	const char *scramble;
	/*
	 * Allow authenticating back to GUEST user without
	 * checking a password. This is useful for connection
	 * pooling.
	 */
	part_count = mp_decode_array(&tuple);
	if (part_count == 0 && user->uid == GUEST &&
	    memcmp(user->hash2, zero_hash, SCRAMBLE_SIZE) == 0) {
		/* No password is set for GUEST, OK. */
		goto ok;
	}

	if (part_count < 2) {
		/* Expected at least: authentication mechanism and data. */
		tnt_raise(ClientError, ER_INVALID_MSGPACK,
			   "authentication request body");
	}
	mp_next(&tuple); /* Skip authentication mechanism. */
	scramble = mp_decode_str(&tuple, &scramble_len);
	if (scramble_len != SCRAMBLE_SIZE) {
		/* Authentication mechanism, data. */
		tnt_raise(ClientError, ER_INVALID_MSGPACK,
			   "invalid scramble size");
	}

	if (scramble_check(scramble, session->salt, user->hash2))
		tnt_raise(ClientError, ER_PASSWORD_MISMATCH, user->name);

ok:
	credentials_init(&session->credentials, user);
}
