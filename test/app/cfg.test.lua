#!/usr/bin/env bee

local tap = require('tap')
local test = tap.test('cfg')
local socket = require('socket')
local fio = require('fio')
test:plan(29)

--------------------------------------------------------------------------------
-- Invalid values
--------------------------------------------------------------------------------

test:is(type(db.cfg), 'function', 'db is not started')

local function invalid(name, val)
    local status, result = pcall(db.cfg, {[name]=val})
    test:ok(not status and result:match('Incorrect'), 'invalid '..name)
end

invalid('replication_source', '//guest@localhost:3301')
invalid('wal_mode', 'invalid')
invalid('rows_per_wal', -1)
invalid('listen', '//!')

test:is(type(db.cfg), 'function', 'db is not started')

--------------------------------------------------------------------------------
-- All db members must raise an exception on access if db.cfg{} wasn't called
--------------------------------------------------------------------------------

local db = require('db')
local function testfun()
    return type(db.space)
end

local status, result = pcall(testfun)

test:ok(not status and result:match('Please call db.cfg{}'),
    'exception on unconfigured db')

os.execute("rm -rf sham")
db.cfg{
    logger="bee.log",
    slab_alloc_arena=0.1,
    wal_mode = "", -- "" means default value
}

-- gh-678: sham engine creates sham dir with empty 'snapshot' file
test:isnil(io.open("sham", 'r'), 'sham_dir is not auto-created')

status, result = pcall(testfun)
test:ok(status and result == 'table', 'configured db')

--------------------------------------------------------------------------------
-- gh-534: Segmentation fault after two bad wal_mode settings
--------------------------------------------------------------------------------

test:is(db.cfg.wal_mode, "write", "wal_mode default value")
db.cfg{wal_mode = ""}
test:is(db.cfg.wal_mode, "write", "wal_mode default value")
db.cfg{wal_mode = "none"}
test:is(db.cfg.wal_mode, "none", "wal_mode change")
-- "" or NULL resets option to default value
db.cfg{wal_mode = ""}
test:is(db.cfg.wal_mode, "write", "wal_mode default value")
db.cfg{wal_mode = "none"}
test:is(db.cfg.wal_mode, "none", "wal_mode change")
db.cfg{wal_mode = require('msgpack').NULL}
test:is(db.cfg.wal_mode, "write", "wal_mode default value")

test:is(db.cfg.panic_on_wal_error, true, "panic_on_wal_mode default value")
db.cfg{panic_on_wal_error=false}
test:is(db.cfg.panic_on_wal_error, false, "panic_on_wal_mode new value")

test:is(db.cfg.wal_dir_rescan_delay, 0.1, "wal_dir_rescan_delay default value")
db.cfg{wal_dir_rescan_delay=0.2}
test:is(db.cfg.wal_dir_rescan_delay, 0.2, "wal_dir_rescan_delay new value")

local bee_bin = arg[-1]
local PANIC = 256
function run_script(code)
    local dir = fio.tempdir()
    local script_path = fio.pathjoin(dir, 'script.lua')
    local script = fio.open(script_path, {'O_CREAT', 'O_WRONLY', 'O_APPEND'},
        tonumber('0777', 8))
    script:write(code)
    script:write("\nos.exit(0)")
    script:close()
    local cmd = [[/bin/sh -c 'cd "%s" && "%s" ./script.lua 2> /dev/null']]
    local res = os.execute(string.format(cmd, dir, bee_bin))
    fio.rmdir(dir)
    return res
end

-- gh-715: Cannot switch to/from 'fsync'
code = [[ db.cfg{ logger="bee.log", wal_mode = 'fsync' }; ]]
test:is(run_script(code), 0, 'wal_mode fsync')

code = [[ db.cfg{ wal_mode = 'fsync' }; db.cfg { wal_mode = 'fsync' }; ]]
test:is(run_script(code), 0, 'wal_mode fsync -> fsync')

code = [[ db.cfg{ wal_mode = 'fsync' }; db.cfg { wal_mode = 'none'} ]]
test:is(run_script(code), PANIC, 'wal_mode fsync -> write is not supported')

code = [[ db.cfg{ wal_mode = 'write' }; db.cfg { wal_mode = 'fsync'} ]]
test:is(run_script(code), PANIC, 'wal_mode write -> fsync is not supported')

-- gh-684: Inconsistency with db.cfg and directories
local code;
code = [[ db.cfg{ work_dir='invalid' } ]]
test:is(run_script(code), PANIC, 'work_dir is invalid')

code = [[ db.cfg{ sham_dir='invalid' } ]]
test:is(run_script(code), PANIC, 'sham_dir is invalid')

code = [[ db.cfg{ snap_dir='invalid' } ]]
test:is(run_script(code), PANIC, 'snap_dir is invalid')

code = [[ db.cfg{ wal_dir='invalid' } ]]
test:is(run_script(code), PANIC, 'wal_dir is invalid')

-- db.cfg { listen = xx }
local path = './bee.sock'
os.remove(path)
db.cfg{ listen = 'unix/:'..path }
s = socket.tcp_connect('unix/', path)
test:isnt(s, nil, "dynamic listen")
if s then s:close() end
db.cfg{ listen = '' }
s = socket.tcp_connect('unix/', path)
test:isnil(s, 'dynamic listen')
if s then s:close() end
os.remove(path)

test:check()
os.exit(0)
