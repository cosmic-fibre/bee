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
#include "sysview_engine.h"
#include "sysview_index.h"
#include "schema.h"
#include "space.h"

static void
sysview_replace(struct txn *txn, struct space *space,
		struct tuple *old_tuple, struct tuple *new_tuple,
		enum dup_replace_mode mode)
{
	(void) txn;
	(void) space;
	(void) old_tuple;
	(void) new_tuple;
	(void) mode;
	tnt_raise(ClientError, ER_UNSUPPORTED, "sysview", "replace()");
}

struct SysviewSpace: public Handler {
	SysviewSpace(Engine *e)
		: Handler(e)
	{
		replace = sysview_replace;
	}

	virtual ~SysviewSpace()
	{
	}
};

SysviewEngine::SysviewEngine()
	:Engine("sysview")
{
}

void
SysviewEngine::recoverToCheckpoint(int64_t /* lsn */)
{
}

void
SysviewEngine::endRecovery()
{
}

Handler *SysviewEngine::open()
{
	return new SysviewSpace(this);
}

Index *
SysviewEngine::createIndex(struct key_def *key_def)
{
	assert(key_def->type == TREE);
	switch (key_def->space_id) {
	case SC_VSPACE_ID:
		return new SysviewVspaceIndex(key_def);
	case SC_VINDEX_ID:
		return new SysviewVindexIndex(key_def);
	case SC_VUSER_ID:
		return new SysviewVuserIndex(key_def);
	case SC_VFUNC_ID:
		return new SysviewVfuncIndex(key_def);
	case SC_VPRIV_ID:
		return new SysviewVprivIndex(key_def);
	default:
		struct space *space = space_cache_find(key_def->space_id);
		tnt_raise(ClientError, ER_MODIFY_INDEX, key_def->name,
			  space_name(space), "unknown space for system view");
		return NULL;
	}
}

void
SysviewEngine::dropIndex(Index *index)
{
	(void) index;
}

void
SysviewEngine::keydefCheck(struct space *space, struct key_def *key_def)
{
	(void) space;
	(void) key_def;
	/*
	 * Don't bother checking key_def to match the view requirements.
	 * Index::initIterator() must check key on each call.
	 */
}

void
SysviewEngine::beginJoin()
{
}

int
SysviewEngine::beginCheckpoint(int64_t lsn)
{
	(void) lsn;
	return 0;
}

int
SysviewEngine::waitCheckpoint()
{
	return 0;
}

void
SysviewEngine::commitCheckpoint()
{
}

void
SysviewEngine::abortCheckpoint()
{
}

bool
SysviewEngine::needToBuildSecondaryKey(struct space * /* space */)
{
	return false;
}

void
SysviewEngine::join(Relay *relay)
{
	(void) relay;
}
