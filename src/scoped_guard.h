#ifndef BEE_SCOPED_GUARD_H_INCLUDED
#define BEE_SCOPED_GUARD_H_INCLUDED

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

#include "object.h"

template <typename Functor>
struct ScopedGuard {
	Functor f;
	bool is_active;

	explicit ScopedGuard(const Functor& fun)
		: f(fun), is_active(true) {
		/* nothing */
	}

	ScopedGuard(ScopedGuard&& guard)
		: f(guard.f), is_active(true) {
		guard.is_active = false;
		abort();
	}

	~ScopedGuard()
	{
		if (is_active)
			f();
	}

private:
	explicit ScopedGuard(const ScopedGuard&) = delete;
	ScopedGuard& operator=(const ScopedGuard&) = delete;
};

template <typename Functor>
inline ScopedGuard<Functor>
make_scoped_guard(Functor guard)
{
	return ScopedGuard<Functor>(guard);
}

#endif /* BEE_SCOPED_GUARD_H_INCLUDED */
