--# push filter 'listen: .*' to 'primary: <uri>'
--# push filter 'admin: .*' to 'admin: <uri>'
db.cfg.nosuchoption = 1
t = {} for k,v in pairs(db.cfg) do if type(v) ~= 'table' and type(v) ~= 'function' then table.insert(t, k..': '..tostring(v)) end end
t
-- must be read-only
db.cfg()
t = {} for k,v in pairs(db.cfg) do if type(v) ~= 'table' and type(v) ~= 'function' then table.insert(t, k..': '..tostring(v)) end end
t

-- check that cfg with unexpected parameter fails.
db.cfg{sherlock = 'holmes'}

-- check that cfg with unexpected type of parameter failes
db.cfg{listen = {}}
db.cfg{wal_dir = 0}
db.cfg{coredump = 'true'}


--------------------------------------------------------------------------------
-- Test of hierarchical cfg type check
--------------------------------------------------------------------------------

db.cfg{slab_alloc_arena = "100500"}
db.cfg{sham = "sham"}
db.cfg{sham = {threads = "threads"}}


--------------------------------------------------------------------------------
-- Test of default cfg options
--------------------------------------------------------------------------------

--# create server cfg_tester1 with script = "db/lua/cfg_test1.lua"
--# start server cfg_tester1
--# create connection cfg_tester_con to cfg_tester1
--# set connection cfg_tester_con
db.cfg.slab_alloc_arena, db.cfg.slab_alloc_factor, db.cfg.sham.threads, db.cfg.sham.page_size
--# set connection default
--# drop connection cfg_tester_con
--# stop server cfg_tester1
--# cleanup server cfg_tester1

--# create server cfg_tester2 with script = "db/lua/cfg_test2.lua"
--# start server cfg_tester2
--# create connection cfg_tester_con to cfg_tester2
--# set connection cfg_tester_con
db.cfg.slab_alloc_arena, db.cfg.slab_alloc_factor, db.cfg.sham.threads, db.cfg.sham.page_size
--# set connection default
--# drop connection cfg_tester_con
--# stop server cfg_tester2
--# cleanup server cfg_tester2

--# create server cfg_tester3 with script = "db/lua/cfg_test3.lua"
--# start server cfg_tester3
--# create connection cfg_tester_con to cfg_tester3
--# set connection cfg_tester_con
db.cfg.slab_alloc_arena, db.cfg.slab_alloc_factor, db.cfg.sham.threads, db.cfg.sham.page_size
--# set connection default
--# drop connection cfg_tester_con
--# stop server cfg_tester3
--# cleanup server cfg_tester3

--# create server cfg_tester4 with script = "db/lua/cfg_test4.lua"
--# start server cfg_tester4
--# create connection cfg_tester_con to cfg_tester4
--# set connection cfg_tester_con
db.cfg.slab_alloc_arena, db.cfg.slab_alloc_factor, db.cfg.sham.threads, db.cfg.sham.page_size
--# set connection default
--# drop connection cfg_tester_con
--# stop server cfg_tester4
--# cleanup server cfg_tester4

--# clear filter
