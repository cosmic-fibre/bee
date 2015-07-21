#ifndef BEE_DB_TXN_H_INCLUDED
#define BEE_DB_TXN_H_INCLUDED
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
#include "index.h"
#include "trigger.h"
#include "fiber.h"

extern double too_long_threshold;
struct tuple;
struct space;

/**
 * A single statement of a multi-statement
 * transaction: undo and redo info.
 */
struct txn_stmt {
	/** Doubly linked list of all statements. */
	struct rlist next;
	/** Undo info. */
	struct space *space;
	struct tuple *old_tuple;
	struct tuple *new_tuple;
	/** Redo info: the binary log row */
	struct xrow_header *row;
};

struct txn {
	/** Pre-allocated first statement. */
	struct txn_stmt stmt;
	/** List of statements in a transaction. */
	struct rlist stmts;
	 /** Commit and rollback triggers */
	struct rlist on_commit, on_rollback;
	/** Total number of statements in this txn. */
	int n_stmts;
	/** Commit signature (LSN sum) */
	int64_t signature;
	/**
	 * True if this transaction is running in autocommit mode
	 * (statement end causes an automatic transaction commit).
	 */
	bool autocommit;
	/** Engine involved in multi-statement transaction. */
	Engine *engine;
	/** Engine-specific transaction data */
	void *engine_tx;
	/**
	 * Triggers on fiber yield and stop to abort transaction
	 * for in-memory engine.
	 */
	struct trigger fiber_on_yield, fiber_on_stop;
};

/* Pointer to the current transaction (if any) */
static inline struct txn *
in_txn()
{
	return (struct txn *) fiber_get_key(fiber(), FIBER_KEY_TXN);
}

/**
 * Start a transaction explicitly.
 * @pre no transaction is active
 */
struct txn *
txn_begin(bool autocommit);

/**
 * Commit a transaction.
 * @pre txn == in_txn()
 */
void
txn_commit(struct txn *txn);

/** Rollback a transaction, if any. */
void
txn_rollback();

/**
 * Start a new statement. If no current transaction,
 * start a new transaction with autocommit = true.
 */
struct txn *
txn_begin_stmt(struct request *request, struct space *space);

/**
 * End a statement. In autocommit mode, end
 * the current transaction as well.
 */
static inline void
txn_commit_stmt(struct txn *txn)
{
	if (txn->autocommit)
		txn_commit(txn);
}

/**
 * Rollback a statement. In autocommit mode,
 * rolls back the entire transaction.
 */
void
txn_rollback_stmt();

/**
 * Raise an error if this is a multi-statement
 * transaction: DDL can not be part of a multi-statement
 * transaction and must be run in autocommit mode.
 */
void
txn_check_autocommit(struct txn *txn, const char *where);

void
txn_replace(struct txn *txn, struct space *space,
	    struct tuple *old_tuple, struct tuple *new_tuple,
	    enum dup_replace_mode mode);

/** Last statement of the transaction. */
static inline struct txn_stmt *
txn_stmt(struct txn *txn)
{
	return rlist_last_entry(&txn->stmts, struct txn_stmt, next);
}

/**
 * FFI bindings: do not throw exceptions, do not accept extra
 * arguments
 */
extern "C" {

/**
 * @retval 0 - success
 * @retval -1 - failed, perhaps a transaction has already been
 * started
 */
int
dbffi_txn_begin();

void
dbffi_txn_rollback();

} /* extern  "C" */

#endif /* BEE_DB_TXN_H_INCLUDED */
