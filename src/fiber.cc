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
#include "fiber.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "say.h"
#include "assoc.h"
#include "memory.h"
#include "trigger.h"
#include <typeinfo>

static struct cord main_cord;
__thread struct cord *cord_ptr = NULL;
pthread_t main_thread_id;

static void
update_last_stack_frame(struct fiber *fiber)
{
#ifdef ENABLE_BACKTRACE
	fiber->last_stack_frame = __builtin_frame_address(0);
#else
	(void)fiber;
#endif /* ENABLE_BACKTRACE */

}

static void
fiber_recycle(struct fiber *fiber);

void
fiber_call(struct fiber *callee)
{
	struct fiber *caller = fiber();
	struct cord *cord = cord();

	assert(cord->call_stack_depth < FIBER_CALL_STACK);
	assert(caller);

	cord->call_stack_depth++;
	cord->fiber = callee;
	callee->caller = caller;

	update_last_stack_frame(caller);

	callee->csw++;
	coro_transfer(&caller->coro.ctx, &callee->coro.ctx);
}

void
fiber_start(struct fiber *callee, ...)
{
	va_start(callee->f_data, callee);
	fiber_call(callee);
	va_end(callee->f_data);
}

bool
fiber_checkstack()
{
	if (cord()->call_stack_depth + 1 >= FIBER_CALL_STACK)
		return true;
	return false;
}

/** Interrupt a synchronous wait of a fiber inside the event loop.
 * We do so by keeping an "async" event in every fiber, solely
 * for this purpose, and raising this event here.
 */

void
fiber_wakeup(struct fiber *f)
{
	/** Remove the fiber from whatever wait list it is on. */
	rlist_del(&f->state);
	struct cord *cord = cord();
	if (rlist_empty(&cord->ready)) {
		/*
		 * ev_feed_event() is possibly faster,
		 * but EV_CUSTOM event gets scheduled in the
		 * same event loop iteration, which can
		 * produce unfair scheduling, (see the case of
		 * fiber_sleep(0))
		 */
		ev_feed_event(cord->loop, &cord->wakeup_event, EV_CUSTOM);
	}
	rlist_move_tail_entry(&cord->ready, f, state);
}

/** Cancel the subject fiber.
*
 * Note: this is not guaranteed to return immediately, since
 * requires a level of cooperation on behalf of the fiber. A fiber
 * may opt to set FIBER_IS_CANCELLABLE to false, and never test that
 * it was cancelled. Such fiber can not ever be cancelled, and
 * for such fiber this call will lead to an infinite wait.
 * However, as long as most of the cooperative code calls fiber_testcancel(),
 * most of the fibers are cancellable.
 *
 * Currently cancellation can only be synchronous: this call
 * returns only when the subject fiber has terminated.
 *
 * The fiber which is cancelled, has FiberCancelException raised
 * in it. For cancellation to work, this exception type should be
 * re-raised whenever (if) it is caught.
 */

void
fiber_cancel(struct fiber *f)
{
	assert(f->fid != 0);
	struct fiber *self = fiber();

	f->flags |= FIBER_IS_CANCELLED;

	/**
	 * Don't wait for self and for fibers which are already
	 * dead.
	 */
	if (f != self && !fiber_is_dead(f)) {
		rlist_add_tail_entry(&f->wake, self, state);
		if (f->flags & FIBER_IS_CANCELLABLE)
			fiber_wakeup(f);
		fiber_yield();
	}
	/*
	 * Check if we're ourselves cancelled.
	 * This also implements cancel for the case when
	 * f == fiber().
	 */
	fiber_testcancel();
}

bool
fiber_is_cancelled()
{
	return fiber()->flags & FIBER_IS_CANCELLED;
}

/** Test if this fiber is in a cancellable state and was indeed
 * cancelled, and raise an exception (FiberCancelException) if
 * that's the case.
 */
void
fiber_testcancel(void)
{
	/*
	 * Fiber can catch FiberCancelException using try..catch
	 * block in C or pcall()/xpcall() in Lua. However,
	 * FIBER_IS_CANCELLED flag is still set and the subject
	 * fiber will be killed by subsequent unprotected call of
	 * this function.
	 */
	if (fiber_is_cancelled())
		tnt_raise(FiberCancelException);
}


/** Change the current cancellation state of a fiber. This is not
 * a cancellation point.
 */
bool
fiber_set_cancellable(bool yesno)
{
	bool prev = fiber()->flags & FIBER_IS_CANCELLABLE;
	if (yesno == true)
		fiber()->flags |= FIBER_IS_CANCELLABLE;
	else
		fiber()->flags &= ~FIBER_IS_CANCELLABLE;
	return prev;
}

void
fiber_set_joinable(struct fiber *fiber, bool yesno)
{
	if (yesno == true)
		fiber->flags |= FIBER_IS_JOINABLE;
	else
		fiber->flags &= ~FIBER_IS_JOINABLE;
}

void
fiber_join(struct fiber *fiber)
{
	assert(fiber->flags & FIBER_IS_JOINABLE);

	if (! fiber_is_dead(fiber)) {
		rlist_add_tail_entry(&fiber->wake, fiber(), state);
		fiber_yield();
	}
	assert(fiber_is_dead(fiber));
	/* The fiber is already dead. */
	fiber_recycle(fiber);

	Exception::move(&fiber->exception, &fiber()->exception);
	if (fiber()->exception &&
	    typeid(*fiber()->exception) != typeid(FiberCancelException)) {
		fiber()->exception->raise();
	}
	fiber_testcancel();
}
/**
 * @note: this is not a cancellation point (@sa fiber_testcancel())
 * but it is considered good practice to call testcancel()
 * after each yield.
 */
void
fiber_yield(void)
{
	struct cord *cord = cord();
	struct fiber *caller = cord->fiber;
	struct fiber *callee = caller->caller;
	cord->call_stack_depth--;
	caller->caller = &cord->sched;

	/** By convention, these triggers must not throw. */
	if (! rlist_empty(&caller->on_yield))
		trigger_run(&caller->on_yield, NULL);

	cord->fiber = callee;
	update_last_stack_frame(caller);

	callee->csw++;
	coro_transfer(&caller->coro.ctx, &callee->coro.ctx);
}

struct fiber_watcher_data {
	struct fiber *f;
	bool timed_out;
};

static void
fiber_schedule_timeout(ev_loop * /* loop */,
		       ev_timer *watcher, int revents)
{
	(void) revents;

	assert(fiber() == &cord()->sched);
	struct fiber_watcher_data *state =
			(struct fiber_watcher_data *) watcher->data;
	state->timed_out = true;
	fiber_call(state->f);
}

/**
 * @brief yield & check timeout
 * @return true if timeout exceeded
 */
bool
fiber_yield_timeout(ev_tstamp delay)
{
	struct ev_timer timer;
	ev_timer_init(&timer, fiber_schedule_timeout, delay, 0);
	struct fiber_watcher_data state = { fiber(), false };
	timer.data = &state;
	ev_timer_start(loop(), &timer);
	fiber_yield();
	ev_timer_stop(loop(), &timer);
	return state.timed_out;
}

/**
 * @note: this is a cancellation point (@sa fiber_testcancel())
 */
void
fiber_sleep(ev_tstamp delay)
{
	/*
	 * We don't use fiber_wakeup() here to ensure there is
	 * no infinite wakeup loop in case of fiber_sleep(0).
	 */
	fiber_yield_timeout(delay);
	fiber_testcancel();
}

void
fiber_schedule_cb(ev_loop * /* loop */, ev_watcher *watcher, int /* revents */)
{
	assert(fiber() == &cord()->sched);
	fiber_call((struct fiber *) watcher->data);
}

static inline void
fiber_schedule_list(struct rlist *list)
{
	/** Don't schedule both lists at the same time. */
	struct rlist copy;
	rlist_create(&copy);
	rlist_swap(list, &copy);
	while (! rlist_empty(&copy)) {
		struct fiber *f;
		f = rlist_shift_entry(&copy, struct fiber, state);
		fiber_call(f);
	}
}

static void
fiber_schedule_wakeup(ev_loop * /* loop */, ev_async *watcher, int revents)
{
	(void) watcher;
	(void) revents;
	struct cord *cord = cord();
	fiber_schedule_list(&cord->ready);
}

struct fiber *
fiber_find(uint32_t fid)
{
	struct mh_i32ptr_t *fiber_registry = cord()->fiber_registry;
	mh_int_t k = mh_i32ptr_find(fiber_registry, fid, NULL);

	if (k == mh_end(fiber_registry))
		return NULL;
	return (struct fiber *) mh_i32ptr_node(fiber_registry, k)->val;
}

static void
register_fid(struct fiber *fiber)
{
	struct mh_i32ptr_node_t node = { fiber->fid, fiber };
	mh_i32ptr_put(cord()->fiber_registry, &node, NULL, NULL);
}

static void
unregister_fid(struct fiber *fiber)
{
	struct mh_i32ptr_node_t node = { fiber->fid, NULL };
	mh_i32ptr_remove(cord()->fiber_registry, &node, NULL);
}

void
fiber_gc(void)
{
	if (region_used(&fiber()->gc) < 128 * 1024) {
		region_reset(&fiber()->gc);
		return;
	}

	region_free(&fiber()->gc);
}

/** Common part of fiber_new() and fiber_recycle(). */
static void
fiber_reset(struct fiber *fiber)
{
	rlist_create(&fiber->on_yield);
	rlist_create(&fiber->on_stop);
	fiber->flags = FIBER_DEFAULT_FLAGS;
}

/** Destroy an active fiber and prepare it for reuse. */
static void
fiber_recycle(struct fiber *fiber)
{
	fiber_reset(fiber);
	rlist_del(&fiber->state);               /* safety */
	fiber->gc.name[0] = '\0';
	fiber->f = NULL;
	memset(fiber->fls, 0, sizeof(fiber->fls));
	unregister_fid(fiber);
	fiber->fid = 0;
	region_free(&fiber->gc);
	rlist_move_entry(&cord()->dead, fiber, link);
}

static void
fiber_loop(void *data __attribute__((unused)))
{
	for (;;) {
		struct fiber *fiber = fiber();

		assert(fiber != NULL && fiber->f != NULL && fiber->fid != 0);
		try {
			fiber->f(fiber->f_data);
			/*
			 * Make sure a leftover exception does not
			 * propagate up to the joiner.
			 */
			Exception::clear(&fiber->exception);
		} catch (FiberCancelException *e) {
			say_info("fiber `%s' has been cancelled",
				 fiber_name(fiber));
			say_info("fiber `%s': exiting", fiber_name(fiber));
		} catch (Exception *e) {
			e->log();
		} catch (...) {
			/* This can only happen in case of a server bug. */
			say_error("fiber `%s': unknown exception",
				fiber_name(fiber));
			panic("fiber `%s': exiting", fiber_name(fiber));
		}
		fiber->flags |= FIBER_IS_DEAD;
		while (! rlist_empty(&fiber->wake)) {
		       struct fiber *f;
		       f = rlist_shift_entry(&fiber->wake, struct fiber,
					     state);
		       fiber_wakeup(f);
	        }
		if (! rlist_empty(&fiber->on_stop))
			trigger_run(&fiber->on_stop, fiber);
		if (! (fiber->flags & FIBER_IS_JOINABLE))
			fiber_recycle(fiber);
		fiber_yield();	/* give control back to scheduler */
	}
}

/** Set fiber name.
 *
 * @param[in] name the new name of the fiber. Truncated to
 * FIBER_NAME_MAXLEN.
*/

void
fiber_set_name(struct fiber *fiber, const char *name)
{
	assert(name != NULL);
	region_set_name(&fiber->gc, name);
}

extern inline void
fiber_set_key(struct fiber *fiber, enum fiber_key key, void *value);

extern inline void *
fiber_get_key(struct fiber *fiber, enum fiber_key key);

/**
 * Create a new fiber.
 *
 * Takes a fiber from fiber cache, if it's not empty.
 * Can fail only if there is not enough memory for
 * the fiber structure or fiber stack.
 *
 * The created fiber automatically returns itself
 * to the fiber cache when its "main" function
 * completes.
 */
struct fiber *
fiber_new(const char *name, void (*f) (va_list))
{
	struct cord *cord = cord();
	struct fiber *fiber = NULL;

	if (! rlist_empty(&cord->dead)) {
		fiber = rlist_first_entry(&cord->dead,
					  struct fiber, link);
		rlist_move_entry(&cord->alive, fiber, link);
	} else {
		fiber = (struct fiber *) mempool_alloc0(&cord->fiber_pool);

		bee_coro_create(&fiber->coro, &cord->slabc,
				      fiber_loop, NULL);

		region_create(&fiber->gc, &cord->slabc);

		rlist_create(&fiber->state);
		rlist_create(&fiber->wake);
		fiber_reset(fiber);

		rlist_add_entry(&cord->alive, fiber, link);
	}

	fiber->f = f;
	/* fids from 0 to 100 are reserved */
	if (++cord->max_fid < 100)
		cord->max_fid = 101;
	fiber->fid = cord->max_fid;
	fiber_set_name(fiber, name);
	register_fid(fiber);

	return fiber;
}

/**
 * Free as much memory as possible taken by the fiber.
 *
 * Sic: cord()->sched needs manual destruction in
 * cord_destroy().
 */
void
fiber_destroy(struct cord *cord, struct fiber *f)
{
	if (f == fiber()) {
		/** End of the application. */
		assert(cord == &main_cord);
		return;
	}
	assert(f != &cord->sched);

	trigger_destroy(&f->on_yield);
	trigger_destroy(&f->on_stop);
	rlist_del(&f->state);
	region_destroy(&f->gc);
	bee_coro_destroy(&f->coro, &cord->slabc);
	Exception::clear(&f->exception);
}

void
fiber_destroy_all(struct cord *cord)
{
	struct fiber *f;
	rlist_foreach_entry(f, &cord->alive, link)
		fiber_destroy(cord, f);
	rlist_foreach_entry(f, &cord->dead, link)
		fiber_destroy(cord, f);
}

void
cord_create(struct cord *cord, const char *name)
{
	cord->id = pthread_self();
	cord->loop = cord->id == main_thread_id ?
		ev_default_loop(EVFLAG_AUTO) : ev_loop_new(EVFLAG_AUTO);
	slab_cache_create(&cord->slabc, &runtime);
	mempool_create(&cord->fiber_pool, &cord->slabc,
		       sizeof(struct fiber));
	rlist_create(&cord->alive);
	rlist_create(&cord->ready);
	rlist_create(&cord->dead);
	cord->fiber_registry = mh_i32ptr_new();

	/* sched fiber is not present in alive/ready/dead list. */
	cord->call_stack_depth = 0;
	cord->sched.fid = 1;
	fiber_reset(&cord->sched);
	Exception::init(&cord->sched.exception);
	region_create(&cord->sched.gc, &cord->slabc);
	fiber_set_name(&cord->sched, "sched");
	cord->fiber = &cord->sched;

	cord->max_fid = 100;

	ev_async_init(&cord->wakeup_event, fiber_schedule_wakeup);
	ev_async_start(cord->loop, &cord->wakeup_event);
	snprintf(cord->name, sizeof(cord->name), "%s", name);
}

void
cord_destroy(struct cord *cord)
{
	slab_cache_set_thread(&cord->slabc);
	ev_async_stop(cord->loop, &cord->wakeup_event);
	/* Only clean up if initialized. */
	if (cord->fiber_registry) {
		fiber_destroy_all(cord);
		mh_i32ptr_delete(cord->fiber_registry);
	}
	region_destroy(&cord->sched.gc);
	Exception::clear(&cord->sched.exception);
	slab_cache_destroy(&cord->slabc);
	ev_loop_destroy(cord->loop);
}

struct cord_thread_arg
{
	struct cord *cord;
	const char *name;
	void *(*f)(void *);
	void *arg;
	bool is_started;
	pthread_mutex_t start_mutex;
	pthread_cond_t start_cond;
};

void *cord_thread_func(void *p)
{
	struct cord_thread_arg *ct_arg = (struct cord_thread_arg *) p;
	cord() = ct_arg->cord;
	slab_cache_set_thread(&cord()->slabc);
	struct cord *cord = cord();
	cord_create(cord, ct_arg->name);
	/** Can't possibly be the main thread */
	assert(cord->id != main_thread_id);
	tt_pthread_mutex_lock(&ct_arg->start_mutex);
	void *(*f)(void *) = ct_arg->f;
	void *arg = ct_arg->arg;
	ct_arg->is_started = true;
	tt_pthread_cond_signal(&ct_arg->start_cond);
	tt_pthread_mutex_unlock(&ct_arg->start_mutex);
	void *res;
	try {
		res = f(arg);
		/*
		 * Clear a possible leftover exception object
		 * to not confuse the invoker of the thread.
		 */
		Exception::clear(&cord->fiber->exception);
	} catch (Exception *) {
		/*
		 * The exception is now available to the caller
		 * via cord->exception.
		 */
		res = NULL;
	}
	return res;
}


int
cord_start(struct cord *cord, const char *name, void *(*f)(void *), void *arg)
{
	int res = -1;
	struct cord_thread_arg ct_arg = { cord, name, f, arg, false,
		PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER };
	tt_pthread_mutex_lock(&ct_arg.start_mutex);
	if (tt_pthread_create(&cord->id, NULL, cord_thread_func, &ct_arg))
		goto end;
	res = 0;
	while (! ct_arg.is_started)
		tt_pthread_cond_wait(&ct_arg.start_cond, &ct_arg.start_mutex);
end:
	tt_pthread_mutex_unlock(&ct_arg.start_mutex);
	tt_pthread_mutex_destroy(&ct_arg.start_mutex);
	tt_pthread_cond_destroy(&ct_arg.start_cond);
	return res;
}

int
cord_join(struct cord *cord)
{
	assert(cord() != cord); /* Can't join self. */
	void *retval = NULL;
	int res = tt_pthread_join(cord->id, &retval);
	if (res == 0 && cord->fiber->exception) {
		/*
		 * cord_thread_func guarantees that
		 * cord->exception is only set if the subject cord
		 * has terminated with an uncaught exception,
		 * transfer it to the caller.
		 */
		Exception::move(&cord->fiber->exception, &fiber()->exception);
		cord_destroy(cord);
		fiber()->exception->raise();
	}
	cord_destroy(cord);
	return res;
}

void
break_ev_loop_f(struct trigger * /* trigger */, void * /* event */)
{
	ev_break(loop(), EVBREAK_ALL);
}

struct costart_ctx
{
	fiber_func run;
	void *arg;
};

/** Replication acceptor fiber handler. */
static void *
cord_costart_thread_func(void *arg)
{
	struct costart_ctx ctx = *(struct costart_ctx *) arg;
	free(arg);

	struct fiber *f = fiber_new("main", ctx.run);

	struct trigger break_ev_loop = {
		rlist_nil, break_ev_loop_f, NULL, NULL
	};
	/*
	 * Got to be in a trigger, to break the loop even
	 * in case of an exception.
	 */
	trigger_add(&f->on_stop, &break_ev_loop);
	fiber_start(f, ctx.arg);
	if (f->fid > 0) {
		/* The fiber hasn't died right away at start. */
		ev_run(loop(), 0);
	}
	if (f->exception &&
	    typeid(f->exception) != typeid(FiberCancelException)) {
		Exception::move(&f->exception, &fiber()->exception);
		fiber()->exception->raise();
	}

	return NULL;
}

int
cord_costart(struct cord *cord, const char *name, fiber_func f, void *arg)
{
	/** Must be allocated to avoid races. */
	struct costart_ctx *ctx = (struct costart_ctx *) malloc(sizeof(*ctx));
	if (ctx == NULL)
		return -1;
	ctx->run = f;
	ctx->arg = arg;
	if (cord_start(cord, name, cord_costart_thread_func, ctx) == -1) {
		free(ctx);
		return -1;
	}
	return 0;
}

bool
cord_is_main()
{
	return cord() == &main_cord;
}

void
fiber_init(void)
{
	main_thread_id = pthread_self();
	cord() = &main_cord;
	cord_create(cord(), "main");
}

void
fiber_free(void)
{
	cord_destroy(cord());
}

int fiber_stat(fiber_stat_cb cb, void *cb_ctx)
{
	struct fiber *fiber;
	struct cord *cord = cord();
	int res;
	rlist_foreach_entry(fiber, &cord->alive, link) {
		res = cb(fiber, cb_ctx);
		if (res != 0)
			return res;
	}
	return 0;
}
