#ifndef BEE_COEIO_H_INCLUDED
#define BEE_COEIO_H_INCLUDED
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

#include "trivia/config.h"

#include <sys/types.h> /* ssize_t */
#include <stdarg.h>

#include "third_party/bee_eio.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Asynchronous IO Tasks (libeio wrapper)
 *
 * Yield the current fiber until a created task is complete.
 */

void coeio_init(void);
void coeio_reinit(void);

struct coeio_task;

typedef ssize_t (*coio_task_cb)(struct coio_task *task); /* like eio_req */
typedef ssize_t (*coio_call_cb)(va_list ap);
typedef void (*coio_task_timeout_cb)(struct coio_task *task); /* like eio_req */

/**
 * A single task context.
 */
struct coio_task {
	struct eio_req base; /* eio_task - must be first */
	/** The calling fiber. */
	struct fiber *fiber;
	/** Callbacks. */
	union {
		struct { /* coio_task() */
			coio_task_cb task_cb;
			coio_task_timeout_cb timeout_cb;
		};
		struct { /* coio_call() */
			coio_call_cb call_cb;
			va_list ap;
		};
	};
	/** Callback results. */
	int complete;
};

ssize_t
coio_task(struct coio_task *task, coio_task_cb func,
	  coio_task_timeout_cb on_timeout, double timeout);

/** \cond public */
ssize_t
coio_call(ssize_t (*func)(va_list ap), ...);

struct addrinfo;

int
coio_getaddrinfo(const char *host, const char *port,
		 const struct addrinfo *hints, struct addrinfo **res,
		 double timeout);
/** \endcond public */

#if defined(__cplusplus)
}
#endif

struct cord;
/**
 * \brief Yield until \a cord has terminated.
 * If \a cord has terminated with an uncaught exception
 * **re-throws** this exception in the calling cord/fiber.
 * \param cord cord
 * \sa pthread_join()
 * \return 0 on success
 */
int
cord_cojoin(struct cord *cord);

#endif /* BEE_COEIO_H_INCLUDED */
