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
#include "coeio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>

#include "fiber.h"
#include "exception.h"
#include "third_party/bee_ev.h"

/*
 * Asynchronous IO Tasks (libeio wrapper).
 * ---------------------------------------
 *
 * libeio request processing is designed in edge-trigger
 * manner, when libeio is ready to process some requests it
 * calls coeio_poller callback.
 *
 * Due to libeio design, want_poll callback is called while
 * locks are being held, so it's not possible to call any libeio
 * function inside this callback. Thus coeio_want_poll raises an
 * async event which will be dealt with normally as part of the
 * main Bee event loop.
 *
 * The async event handler, in turn, performs eio_poll(), which
 * will run on_complete callback for all ready eio tasks.
 * In case if some of the requests are not complete by the time
 * eio_poll() has been called, coeio_idle watcher is started, which
 * would periodically invoke eio_poll() until all requests are
 * complete.
 *
 * See for details:
 * http://pod.tst.eu/http://cvs.schmorp.de/libeio/eio.pod
*/

struct coeio_manager {
	ev_loop *loop;
	ev_idle coeio_idle;
	ev_async coeio_async;
} coeio_manager;

static void
coeio_idle_cb(ev_loop *loop, struct ev_idle *w, int /* events */)
{
	if (eio_poll() != -1) {
		/* nothing to do */
		ev_idle_stop(loop, w);
	}
}

static void
coeio_async_cb(ev_loop *loop, struct ev_async *w __attribute__((unused)),
	       int events __attribute__((unused)))
{
	if (eio_poll() == -1) {
		/* not all tasks are complete. */
		ev_idle_start(loop, &coeio_manager.coeio_idle);
	}
}

static void
coeio_want_poll_cb(void)
{
	ev_async_send(coeio_manager.loop, &coeio_manager.coeio_async);
}

/**
 * Init coeio subsystem.
 *
 * Create idle and async watchers, init eio.
 */
void
coeio_init(void)
{
	eio_init(coeio_want_poll_cb, NULL);
	coeio_manager.loop = loop();

	ev_idle_init(&coeio_manager.coeio_idle, coeio_idle_cb);
	ev_async_init(&coeio_manager.coeio_async, coeio_async_cb);

	ev_async_start(loop(), &coeio_manager.coeio_async);
}

/**
 * ReInit coeio subsystem (for example after 'fork')
 */
void
coeio_reinit(void)
{
	eio_init(coeio_want_poll_cb, NULL);
}

static void
coio_on_exec(eio_req *req)
{
	struct coio_task *task = (struct coio_task *) req;
	req->result = task->task_cb(task);
}

/**
 * A callback invoked by eio_poll when associated
 * eio_request is complete.
 */
static int
coio_on_finish(eio_req *req)
{
	struct coio_task *task = (struct coio_task *) req;
	if (task->fiber == NULL) {
		/* timed out (only coio_task() )*/
		if (task->timeout_cb != NULL) {
			task->timeout_cb(task);
		}
		return 0;
	}

	task->complete = 1;
	fiber_wakeup(task->fiber);
	return 0;
}

ssize_t
coio_task(struct coio_task *task, coio_task_cb func,
	  coio_task_timeout_cb on_timeout, double timeout)
{
	/* from eio.c: REQ() definition */
	memset(&task->base, 0, sizeof(task->base));
	task->base.type = EIO_CUSTOM;
	task->base.feed = coio_on_exec;
	task->base.finish = coio_on_finish;
	/* task->base.destroy = NULL; */
	/* task->base.pri = 0; */

	task->fiber = fiber();
	task->task_cb = func;
	task->timeout_cb = on_timeout;
	task->complete = 0;

	eio_submit(&task->base);

	if (fiber_yield_timeout(timeout) && !task->complete) {
		/* timed out. */
		task->fiber = NULL;
		errno = ETIMEDOUT;
		return -1;
	}

	return task->base.result;
}

static void
coio_on_call(eio_req *req)
{
	struct coio_task *task = (struct coio_task *) req;
	req->result = task->call_cb(task->ap);
}

/**
 * Create new eio task with specified function and
 * arguments. Yield and wait until the task is complete
 * or a timeout occurs.
 *
 * This function doesn't throw exceptions to avoid double error
 * checking: in most cases it's also necessary to check the return
 * value of the called function and perform necessary actions. If
 * func sets errno, the errno is preserved across the call.
 *
 * @retval -1 and errno = ENOMEM if failed to create a task
 * @retval the function return (errno is preserved).
 *
 * @code
 *	static ssize_t openfile_cb(va_list ap)
 *	{
 *	         const char *filename = va_arg(ap);
 *	         int flags = va_arg(ap);
 *	         return open(filename, flags);
 *	}
 *
 *	 if (coio_call(openfile_cb, 0.10, "/tmp/file", 0) == -1)
 *		// handle errors.
 *	...
 */
ssize_t
coio_call(ssize_t (*func)(va_list ap), ...)
{
	struct coio_task *task = (struct coio_task *) calloc(1, sizeof(*task));
	if (task == NULL)
		return -1; /* errno = ENOMEM */
	/* from eio.c: REQ() definition */
	task->base.type = EIO_CUSTOM;
	task->base.feed = coio_on_call;
	task->base.finish = coio_on_finish;
	/* task->base.destroy = NULL; */
	/* task->base.pri = 0; */

	task->fiber = fiber();
	task->call_cb = func;
	task->complete = 0;

	bool cancellable = fiber_set_cancellable(false);

	va_start(task->ap, func);
	eio_submit(&task->base);

	fiber_yield();
	assert(task->complete);
	va_end(task->ap);

	fiber_set_cancellable(cancellable);

	ssize_t result = task->base.result;
	int save_errno = errno;
	free(task);
	errno = save_errno;
	return result;
}

struct async_getaddrinfo_task {
	struct coio_task base;
	struct addrinfo *result;
	int rc;
	char *host;
	char *port;
	struct addrinfo hints;
};

#ifndef EAI_ADDRFAMILY
#define EAI_ADDRFAMILY EAI_BADFLAGS /* EAI_ADDRFAMILY is deprecated on BSD */
#endif

/*
 * Resolver function, run in separate thread by
 * coeio (libeio).
*/
static ssize_t
getaddrinfo_cb(struct coio_task *ptr)
{
	struct async_getaddrinfo_task *task =
		(struct async_getaddrinfo_task *) ptr;

	task->rc = getaddrinfo(task->host, task->port, &task->hints,
			     &task->result);

	/* getaddrinfo can return EAI_ADDRFAMILY on attempt
	 * to resolve ::1, if machine has no public ipv6 addresses
	 * configured. Retry without AI_ADDRCONFIG flag set.
	 *
	 * See for details: https://bugs.launchpad.net/bee/+bug/1160877
	 */
	if ((task->rc == EAI_BADFLAGS || task->rc == EAI_ADDRFAMILY) &&
	    (task->hints.ai_flags & AI_ADDRCONFIG)) {
		task->hints.ai_flags &= ~AI_ADDRCONFIG;
		task->rc = getaddrinfo(task->host, task->port, &task->hints,
			     &task->result);
	}
	return 0;
}

static void
getaddrinfo_free_cb(struct coio_task *ptr)
{
	struct async_getaddrinfo_task *task =
		(struct async_getaddrinfo_task *) ptr;
	free(task->host);
	free(task->port);
	if (task->result != NULL)
		freeaddrinfo(task->result);
	free(task);
}

int
coio_getaddrinfo(const char *host, const char *port,
		 const struct addrinfo *hints, struct addrinfo **res,
		 double timeout)
{
	int rc = EAI_SYSTEM;
	int save_errno = 0;

	struct async_getaddrinfo_task *task =
		(struct async_getaddrinfo_task *) calloc(1, sizeof(*task));
	if (task == NULL)
		return rc;

	/* Fill hinting information for use by connect(2) or bind(2). */
	memcpy(&task->hints, hints, sizeof(task->hints));
	/* make no difference between empty string and NULL for host */
	if (host != NULL && *host) {
		task->host = strdup(host);
		if (task->host == NULL) {
			save_errno = errno;
			goto cleanup_task;
		}
	}
	if (port != NULL) {
		task->port = strdup(port);
		if (task->port == NULL) {
			save_errno = errno;
			goto cleanup_host;
		}
	}
	/* do resolving */
	/* coio_task() don't throw. */
	if (coio_task(&task->base, getaddrinfo_cb, getaddrinfo_free_cb,
		       timeout) == -1) {
		/* timed out */
		errno = ETIMEDOUT;
		return rc;
	}

	rc = task->rc;
	*res = task->result;
	free(task->port);
cleanup_host:
	free(task->host);
cleanup_task:
	free(task);
	errno = save_errno;
	return rc;
}

static ssize_t
cord_cojoin_cb(va_list ap)
{
	struct cord *cord = va_arg(ap, struct cord *);
	void *retval = NULL;
	int res = tt_pthread_join(cord->id, &retval);
	return res;
}

int
cord_cojoin(struct cord *cord)
{
	assert(cord() != cord); /* Can't join self. */
	int rc = coio_call(cord_cojoin_cb, cord);
	if (rc == 0 && cord->fiber->exception) {
		Exception::move(&cord->fiber->exception, &fiber()->exception);
		cord_destroy(cord);
		 /* re-throw exception in this fiber */
		fiber()->exception->raise();
	}
	cord_destroy(cord);
	return rc;
}

