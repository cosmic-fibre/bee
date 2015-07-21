#ifndef BEE_DB_SYSVIEW_ENGINE_H_INCLUDED
#define BEE_DB_SYSVIEW_ENGINE_H_INCLUDED
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
#include "engine.h"

struct SysviewEngine: public Engine {
public:
	SysviewEngine();
	virtual Handler *open();
	virtual Index *createIndex(struct key_def *key_def);
	virtual void dropIndex(Index *index);
	virtual void keydefCheck(struct space *space, struct key_def *key_def);
	virtual void beginJoin();
	virtual void recoverToCheckpoint(int64_t lsn);
	virtual void endRecovery();
	virtual bool needToBuildSecondaryKey(struct space *space);
	virtual void join(Relay*);
	virtual int beginCheckpoint(int64_t);
	virtual int waitCheckpoint();
	virtual void commitCheckpoint();
	virtual void abortCheckpoint();
};

#endif /* BEE_DB_SYSVIEW_ENGINE_H_INCLUDED */
