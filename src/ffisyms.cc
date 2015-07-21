
#include <bit/bit.h>
#include <lib/msgpuck/msgpuck.h>
#include "scramble.h"
#include <db/db.h>
#include <db/tuple.h>
#include <db/lua/index.h>
#include <db/lua/tuple.h>
#include <db/lua/call.h>
#include <db/sham_engine.h>
#include <lua/init.h>
#include "main.h"
#include "lua/bsdsocket.h"
#include "lua/digest.h"
#include "fiber.h"
#include "base64.h"
#include "random.h"
#include <lib/salad/guava.h>

/*
 * A special hack to cc/ld to keep symbols in an optimized binary.
 * Please add your symbols to this array if you plan to use it from
 * LuaJIT FFI.
 */
void *ffi_symbols[] = {
	(void *) bswap_u32,
	(void *) bswap_u64,
	(void *) mp_bswap_float,
	(void *) mp_bswap_double,
	(void *) tuple_field_count,
	(void *) tuple_field,
	(void *) tuple_rewind,
	(void *) tuple_seek,
	(void *) tuple_next,
	(void *) tuple_unref,
	(void *) dbffi_index_len,
	(void *) dbffi_index_bsize,
	(void *) dbffi_index_random,
	(void *) dbffi_index_get,
	(void *) dbffi_index_min,
	(void *) dbffi_index_max,
	(void *) dbffi_index_count,
	(void *) dbffi_index_iterator,
	(void *) dbffi_tuple_update,
	(void *) dbffi_iterator_next,
	(void *) port_ffi_create,
	(void *) port_ffi_destroy,
	(void *) dbffi_select,
	(void *) password_prepare,
	(void *) bee_error_message,
	(void *) load_cfg,
	(void *) db_set_listen,
	(void *) db_set_replication_source,
	(void *) db_set_wal_mode,
	(void *) db_set_log_level,
	(void *) db_set_io_collect_interval,
	(void *) db_set_snap_io_rate_limit,
	(void *) db_set_too_long_threshold,
	(void *) bsdsocket_local_resolve,
	(void *) bsdsocket_nonblock,
	(void *) base64_decode,
	(void *) base64_encode,
	(void *) base64_bufsize,
	(void *) SHA1internal,
	(void *) guava,
	(void *) random_bytes,
	(void *) fiber_time,
	(void *) fiber_time64,
	(void *) sham_schedule
};
