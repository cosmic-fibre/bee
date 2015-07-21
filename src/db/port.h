#ifndef INCLUDES_BEE_DB_PORT_H
#define INCLUDES_BEE_DB_PORT_H
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
#include "trivia/util.h"

struct tuple;
struct port;

/**
 * A single port represents a destination of db_process output.
 * One such destination can be a Lua stack, or the binary
 * protocol.
 * An instance of a port is usually short lived, as it is created
 * for every server request. State of the instance is represented
 * by the tuples added to it. E.g.:
 *
 * struct port_iproto *port = port_iproto_new(...)
 * for (tuple in tuples)
 *	port_add_tuple(tuple);
 * port_eof(port);	// end of request
 *
 * Beginning with Bee 1.5, tuple can have different internal
 * structure and port_add_tuple() requires a double
 * dispatch: first, by the type of the port the tuple is being
 * added to, second, by the type of the tuple format, since the
 * format defines the internal structure of the tuple.
 */

struct port_vtab
{
	void (*add_tuple)(struct port *port, struct tuple *tuple);
	/** Must be called in the end of execution of a single request. */
	void (*eof)(struct port *port);
};

struct port
{
	struct port_vtab *vtab;
};

static inline void
port_eof(struct port *port)
{
	return (port->vtab->eof)(port);
}

static inline void
port_add_tuple(struct port *port, struct tuple *tuple)
{
	(port->vtab->add_tuple)(port, tuple);
}

/** Reused in port_lua */
void
null_port_eof(struct port *port __attribute__((unused)));

/**
 * This one does not have state currently, thus a single
 * instance is sufficient.
 */
extern struct port null_port;

#endif /* INCLUDES_BEE_DB_PORT_H */
