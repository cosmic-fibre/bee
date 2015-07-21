#ifndef BEE_COIO_H_INCLUDED
#define BEE_COIO_H_INCLUDED
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
#include "evio.h"
#include "fiber.h"

/**
 * Co-operative I/O
 * Yield the current fiber until IO is ready.
 */
struct coio_service
{
	struct evio_service evio_service;
	/* Fiber function. */
	void (*handler)(va_list ap);
	/** Passed to the created fiber. */
	void *handler_param;
};

typedef void (*ev_io_cb)(ev_loop *, ev_io *, int);

int
coio_connect_timeout(struct ev_io *coio, struct uri *uri, struct sockaddr *addr,
		     socklen_t *addr_len, ev_tstamp timeout);

static inline int
coio_connect(struct ev_io *coio, struct uri *uri, struct sockaddr *addr,
		socklen_t *addr_len)
{
	return coio_connect_timeout(coio, uri, addr, addr_len, TIMEOUT_INFINITY);
}

void
coio_bind(struct ev_io *coio, struct sockaddr *addr,
	  socklen_t addrlen);

int
coio_accept(struct ev_io *coio, struct sockaddr *addr, socklen_t addrlen,
	    ev_tstamp timeout);

void
coio_init(struct ev_io *coio);

ssize_t
coio_read_ahead_timeout(struct ev_io *coio, void *buf, size_t sz, size_t bufsiz,
		        ev_tstamp timeout);

static inline void
coio_timeout_init(ev_tstamp *start, ev_tstamp *delay,
		  ev_tstamp timeout)
{
	return evio_timeout_init(loop(), start, delay, timeout);
}

static inline void
coio_timeout_update(ev_tstamp start, ev_tstamp *delay)
{
	return evio_timeout_update(loop(), start, delay);
}

/**
 * Reat at least sz bytes, with readahead.
 *
 * Returns 0 in case of EOF.
 */
static inline ssize_t
coio_read_ahead(struct ev_io *coio, void *buf, size_t sz, size_t bufsiz)
{
	return coio_read_ahead_timeout(coio, buf, sz, bufsiz, TIMEOUT_INFINITY);
}

ssize_t
coio_readn_ahead(struct ev_io *coio, void *buf, size_t sz, size_t bufsiz);

static inline ssize_t
coio_read(struct ev_io *coio, void *buf, size_t sz)
{
	return coio_read_ahead(coio, buf, sz, sz);
}

static inline ssize_t
coio_read_timeout(struct ev_io *coio, void *buf, size_t sz, ev_tstamp timeout)
{
	return coio_read_ahead_timeout(coio, buf, sz, sz, timeout);
}

static inline ssize_t
coio_readn(struct ev_io *coio, void *buf, size_t sz)
{
	return coio_readn_ahead(coio, buf, sz, sz);
}

ssize_t
coio_readn_ahead_timeout(struct ev_io *coio, void *buf, size_t sz, size_t bufsiz,
		         ev_tstamp timeout);

ssize_t
coio_write_timeout(struct ev_io *coio, const void *buf, size_t sz,
		   ev_tstamp timeout);

static inline void
coio_write(struct ev_io *coio, const void *buf, size_t sz)
{
	coio_write_timeout(coio, buf, sz, TIMEOUT_INFINITY);
}

ssize_t
coio_writev_timeout(struct ev_io *coio, struct iovec *iov, int iovcnt,
		    size_t size, ev_tstamp timeout);

static inline ssize_t
coio_writev(struct ev_io *coio, struct iovec *iov, int iovcnt, size_t size)
{
	return coio_writev_timeout(coio, iov, iovcnt, size, TIMEOUT_INFINITY);
}

ssize_t
coio_sendto_timeout(struct ev_io *coio, const void *buf, size_t sz, int flags,
		    const struct sockaddr *dest_addr, socklen_t addrlen,
		    ev_tstamp timeout);

ssize_t
coio_recvfrom_timeout(struct ev_io *coio, void *buf, size_t sz, int flags,
		      struct sockaddr *src_addr, socklen_t addrlen,
		      ev_tstamp timeout);

void
coio_service_init(struct coio_service *service, const char *name,
		  void (*handler)(va_list ap), void *handler_param);

/** Wait until the service binds to the port. */
void
coio_service_start(struct evio_service *service, const char *uri);

void
coio_stat_init(ev_stat *stat, const char *path);

void
coio_stat_stat_timeout(ev_stat *stat, ev_tstamp delay);

/**
 * Wait for a child to end.
 * @note this is a cancellation point (can throw
 * FiberCancelException).
 *
 * @retval exit status of the child.
 *
 * This call only works in the main thread.
 */
int
coio_waitpid(pid_t pid);


#endif /* BEE_COIO_H_INCLUDED */
