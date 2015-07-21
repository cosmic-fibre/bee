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
#include "replication.h"
#include "sham_engine.h"
#include "cfg.h"
#include "xrow.h"
#include "tuple.h"
#include "scoped_guard.h"
#include "txn.h"
#include "index.h"
#include "sham_index.h"
#include "recovery.h"
#include "space.h"
#include "request.h"
#include "iproto_constants.h"
#include "replication.h"
#include "salad/rlist.h"
#include <sham.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

void sham_raise(void *env)
{
	void *c = sp_ctl(env);
	void *o = sp_get(c, "sham.error");
	char *error = (char *)sp_get(o, "value", NULL);
	auto scoped_guard =
		make_scoped_guard([=] { sp_destroy(o); });
	tnt_raise(ClientError, ER_SHAM, error);
}

void sham_info(void (*callback)(const char*, const char*, void*), void *arg)
{
	ShamEngine *engine = (ShamEngine*)engine_find("sham");
	void *env = engine->env;
	void *c = sp_ctl(env);
	void *o, *cur = sp_cursor(c);
	if (cur == NULL)
		sham_raise(env);
	while ((o = sp_get(cur))) {
		const char *k = (const char *)sp_get(o, "key", NULL);
		const char *v = (const char *)sp_get(o, "value", NULL);
		callback(k, v, arg);
	}
	sp_destroy(cur);
}

void
sham_replace(struct txn * /* txn */, struct space *space,
	       struct tuple *old_tuple, struct tuple *new_tuple,
               enum dup_replace_mode mode)
{
	Index *index = index_find(space, 0);
	(void) index->replace(old_tuple, new_tuple, mode);
}

struct ShamSpace: public Handler {
	ShamSpace(Engine*);
};

ShamSpace::ShamSpace(Engine *e)
	:Handler(e)
{
	replace = sham_replace;
}

ShamEngine::ShamEngine()
	:Engine("sham")
	 ,m_prev_checkpoint_lsn(-1)
	 ,m_checkpoint_lsn(-1)
	 ,recovery_complete(0)
{
	flags = 0;
	env = NULL;
}

void
ShamEngine::init()
{
	env = sp_env();
	if (env == NULL)
		panic("failed to create sham environment");
	void *c = sp_ctl(env);
	sp_set(c, "sham.path", cfg_gets("sham_dir"));
	sp_set(c, "sham.path_create", "0");
	sp_set(c, "scheduler.threads", cfg_gets("sham.threads"));
	sp_set(c, "memory.limit", cfg_gets("sham.memory_limit"));
	sp_set(c, "compaction.node_size", cfg_gets("sham.node_size"));
	sp_set(c, "compaction.page_size", cfg_gets("sham.page_size"));
	sp_set(c, "log.enable", "0");
	sp_set(c, "log.two_phase_recover", "1");
	sp_set(c, "log.commit_lsn", "1");
	int rc = sp_open(env);
	if (rc == -1)
		sham_raise(env);
}

Handler *
ShamEngine::open()
{
	return new ShamSpace(this);
}

static inline void
sham_send_row(Relay *relay, uint32_t space_id, char *tuple,
                uint32_t tuple_size)
{
	struct recovery_state *r = relay->r;
	struct request_replace_body body;
	body.m_body = 0x82; /* map of two elements. */
	body.k_space_id = IPROTO_SPACE_ID;
	body.m_space_id = 0xce; /* uint32 */
	body.v_space_id = mp_bswap_u32(space_id);
	body.k_tuple = IPROTO_TUPLE;
	struct xrow_header row;
	row.type = IPROTO_INSERT;
	row.server_id = 0;
	row.lsn = vclock_inc(&r->vclock, row.server_id);
	row.bodycnt = 2;
	row.body[0].iov_base = &body;
	row.body[0].iov_len = sizeof(body);
	row.body[1].iov_base = tuple;
	row.body[1].iov_len = tuple_size;
	relay_send(relay, &row);
}

/**
 * Relay all data that should be present in the snapshot
 * to the replica.
 */
void
ShamEngine::join(Relay *relay)
{
	struct vclock *res = vclockset_last(&relay->r->snap_dir.index);
	if (res == NULL)
		tnt_raise(ClientError, ER_MISSING_SNAPSHOT);
	int64_t signt = vclock_signature(res);

	/* get snapshot object */
	char id[128];
	snprintf(id, sizeof(id), "snapshot.%" PRIu64, signt);
	void *c = sp_ctl(env);
	void *snapshot = sp_get(c, id);
	assert(snapshot != NULL);

	/* iterate through a list of databases which took a
	 * part in the snapshot */
	void *db_cursor = sp_ctl(snapshot, "db_list");
	if (db_cursor == NULL)
		sham_raise(env);
	while (sp_get(db_cursor)) {
		void *db = sp_object(db_cursor);

		/* get space id */
		void *dbctl = sp_ctl(db);
		void *oid = sp_get(dbctl, "name");
		char *name = (char*)sp_get(oid, "value", NULL);
		char *pe = NULL;
		uint32_t space_id = strtoul(name, &pe, 10);
		sp_destroy(oid);

		/* send database */
		void *o = sp_object(db);
		void *cursor = sp_cursor(snapshot, o);
		if (cursor == NULL) {
			sp_destroy(db_cursor);
			sham_raise(env);
		}
		while (sp_get(cursor)) {
			o = sp_object(cursor);
			uint32_t tuple_size = 0;
			char *tuple = (char *)sp_get(o, "value", &tuple_size);
			try {
				sham_send_row(relay, space_id, tuple, tuple_size);
			} catch (...) {
				sp_destroy(cursor);
				sp_destroy(db_cursor);
				throw;
			}
		}
		sp_destroy(cursor);
	}
	sp_destroy(db_cursor);
}

void
ShamEngine::endRecovery()
{
	if (recovery_complete)
		return;
	/* complete two-phase recovery */
	int rc = sp_open(env);
	if (rc == -1)
		sham_raise(env);
	recovery_complete = 1;
}

Index*
ShamEngine::createIndex(struct key_def *key_def)
{
	switch (key_def->type) {
	case TREE: return new ShamIndex(key_def);
	default:
		assert(false);
		return NULL;
	}
}

void
ShamEngine::dropIndex(Index *index)
{
	ShamIndex *i = (ShamIndex*)index;
	/* schedule asynchronous drop */
	int rc = sp_drop(i->db);
	if (rc == -1)
		sham_raise(env);
	/* unref db object */
	rc = sp_destroy(i->db);
	if (rc == -1)
		sham_raise(env);
	/* maybe start asynchronous database
	 * shutdown: last snapshot might hold a
	 * db pointer. */
	rc = sp_destroy(i->db);
	if (rc == -1)
		sham_raise(env);
	i->db  = NULL;
	i->env = NULL;
}

void
ShamEngine::keydefCheck(struct space *space, struct key_def *key_def)
{
	switch (key_def->type) {
	case TREE:
		if (! key_def->is_unique) {
			tnt_raise(ClientError, ER_MODIFY_INDEX,
				  key_def->name,
				  space_name(space),
				  "Sham TREE index must be unique");
		}
		if (key_def->iid != 0) {
			tnt_raise(ClientError, ER_MODIFY_INDEX,
				  key_def->name,
				  space_name(space),
				  "Sham TREE secondary indexes are not supported");
		}
		if (key_def->part_count != 1) {
			tnt_raise(ClientError, ER_MODIFY_INDEX,
				  key_def->name,
				  space_name(space),
				  "Sham TREE index key can not be multipart");
		}
		if (key_def->parts[0].type != NUM &&
		    key_def->parts[0].type != STRING) {
			tnt_raise(ClientError, ER_MODIFY_INDEX,
				  key_def->name,
				  space_name(space),
				  "Sham TREE index field type must be STR or NUM");
		}
		break;
	default:
		tnt_raise(ClientError, ER_INDEX_TYPE,
			  key_def->name,
			  space_name(space));
		break;
	}
}

void
ShamEngine::beginStatement(struct txn *txn)
{
	assert(txn->engine_tx == NULL || txn->n_stmts != 1);
	if (txn->n_stmts == 1) {
		txn->engine_tx = sp_begin(env);
		if (txn->engine_tx == NULL)
			sham_raise(env);
	}
}

void
ShamEngine::prepare(struct txn *txn)
{
	int rc = sp_prepare(txn->engine_tx);
	switch (rc) {
	case 1: /* rollback */
		txn->engine_tx = NULL;
	case 2: /* lock */
		tnt_raise(ClientError, ER_TRANSACTION_CONFLICT);
		break;
	case -1:
		sham_raise(env);
		break;
	}
}

void
ShamEngine::commit(struct txn *txn)
{
	if (txn->engine_tx == NULL)
		return;
	/* commit transaction using transaction
	 * commit signature */
	assert(txn->signature >= 0);
	int rc = sp_commit(txn->engine_tx, txn->signature);
	if (rc == -1) {
		panic("sham commit failed: txn->signature = %"
		      PRIu64, txn->signature);
	}
	txn->engine_tx = NULL;
}

void
ShamEngine::rollbackStatement(struct txn_stmt *)
{
	panic("not implemented");
}

void
ShamEngine::rollback(struct txn *txn)
{
	if (txn->engine_tx) {
		sp_destroy(txn->engine_tx);
		txn->engine_tx = NULL;
	}
}

void
ShamEngine::beginJoin()
{
	/* put engine to recovery-complete state to
	 * correctly support join */
	endRecovery();
}

static inline void
sham_snapshot(void *env, int64_t lsn)
{
	/* start asynchronous checkpoint */
	void *c = sp_ctl(env);
	int rc = sp_set(c, "scheduler.checkpoint");
	if (rc == -1)
		sham_raise(env);
	char snapshot[128];
	snprintf(snapshot, sizeof(snapshot), "snapshot.%" PRIu64, lsn);
	/* ensure snapshot is not already exists */
	void *o = sp_get(c, snapshot);
	if (o) {
		return;
	}
	snprintf(snapshot, sizeof(snapshot), "%" PRIu64, lsn);
	rc = sp_set(c, "snapshot", snapshot);
	if (rc == -1)
		sham_raise(env);
}

static inline void
sham_reference_checkpoint(void *env, int64_t lsn)
{
	/* recovered snapshot lsn is >= then last
	 * engine lsn */
	char checkpoint_id[32];
	snprintf(checkpoint_id, sizeof(checkpoint_id), "%" PRIu64, lsn);
	void *c = sp_ctl(env);
	int rc = sp_set(c, "snapshot", checkpoint_id);
	if (rc == -1)
		sham_raise(env);
	char snapshot[128];
	snprintf(snapshot, sizeof(snapshot), "snapshot.%" PRIu64 ".lsn", lsn);
	rc = sp_set(c, snapshot, checkpoint_id);
	if (rc == -1)
		sham_raise(env);
}

static inline int
sham_snapshot_ready(void *env, int64_t lsn)
{
	/* get sham lsn associated with snapshot */
	char snapshot[128];
	snprintf(snapshot, sizeof(snapshot), "snapshot.%" PRIu64 ".lsn", lsn);
	void *c = sp_ctl(env);
	void *o = sp_get(c, snapshot);
	if (o == NULL) {
		if (sp_error(env))
			sham_raise(env);
		panic("sham snapshot %" PRIu64 " does not exist", lsn);
	}
	char *pe;
	char *p = (char *)sp_get(o, "value", NULL);
	int64_t snapshot_start_lsn = strtoull(p, &pe, 10);
	sp_destroy(o);

	/* compare with a latest completed checkpoint lsn */
	o = sp_get(c, "scheduler.checkpoint_lsn_last");
	if (o == NULL)
		sham_raise(env);
	p = (char *)sp_get(o, "value", NULL);
	int64_t last_lsn = strtoull(p, &pe, 10);
	sp_destroy(o);
	return last_lsn >= snapshot_start_lsn;
}

static inline void
sham_delete_checkpoint(void *env, int64_t lsn)
{
	char snapshot[128];
	snprintf(snapshot, sizeof(snapshot), "snapshot.%" PRIu64, lsn);
	void *c = sp_ctl(env);
	void *s = sp_get(c, snapshot);
	if (s == NULL) {
		if (sp_error(env))
			sham_raise(env);
		panic("sham snapshot %" PRIu64 " does not exist", lsn);
	}
	int rc = sp_destroy(s);
	if (rc == -1)
		sham_raise(env);
}

void
ShamEngine::recoverToCheckpoint(int64_t checkpoint_id)
{
	/*
	 * Create a reference to the "current" snapshot,
	 * to ensure correct reference counting when a new
	 * snapshot is created.
	 * Sham doesn't store snapshot references persistently,
	 * so, after recovery, we remember the reference to the
	 * "previous" snapshot, so that when the next snapshot is
	 * taken, this reference is garbage collected. This
	 * will also prevent this snapshot from accidental
	 * garbage collection before a new snapshot is created,
	 * and thus ensure correct crash recovery in case of crash
	 * in the period between startup and creation of the first
	 * snapshot.
	 */
	sham_reference_checkpoint(env, checkpoint_id);
	m_prev_checkpoint_lsn = checkpoint_id;
}

int
ShamEngine::beginCheckpoint(int64_t lsn)
{
	assert(m_checkpoint_lsn == -1);
	if (lsn != m_prev_checkpoint_lsn) {
		sham_snapshot(env, lsn);
		m_checkpoint_lsn = lsn;
		return 0;
	}
	errno = EEXIST;
	return -1;
}

int
ShamEngine::waitCheckpoint()
{
	assert(m_checkpoint_lsn != -1);
	while (! sham_snapshot_ready(env, m_checkpoint_lsn))
		fiber_yield_timeout(.020);
	return 0;
}

void
ShamEngine::commitCheckpoint()
{
	if (m_prev_checkpoint_lsn >= 0)
		sham_delete_checkpoint(env, m_prev_checkpoint_lsn);
	m_prev_checkpoint_lsn = m_checkpoint_lsn;
	m_checkpoint_lsn = -1;
}

void
ShamEngine::abortCheckpoint()
{
	if (m_checkpoint_lsn >= 0) {
		sham_delete_checkpoint(env, m_checkpoint_lsn);
		m_checkpoint_lsn = -1;
	}
}

int sham_schedule(void)
{
	ShamEngine *engine = (ShamEngine *)engine_find("sham");
	assert(engine->env != NULL);
	void *c = sp_ctl(engine->env);
	return sp_set(c, "scheduler.run");
}
