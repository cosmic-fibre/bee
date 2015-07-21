db.cfg.too_long_threshold
-- good
db.cfg{too_long_threshold=0.2}
db.cfg.too_long_threshold
-- good 
db.cfg{snap_io_rate_limit=10}
db.cfg.snap_io_rate_limit
db.cfg.io_collect_interval
db.cfg{io_collect_interval=0.001}
db.cfg.io_collect_interval

-- A test case for http://bugs.launchpad.net/bugs/712447:
-- Valgrind reports use of not initialized memory after 'reload
-- configuration'
--
space = db.schema.space.create('tweedledum')
index = space:create_index('primary')
space:insert{1, 'tuple'}
db.snapshot()
db.cfg{}

space:insert{2, 'tuple2'}
db.snapshot()
space:insert{3, 'tuple3'}
db.snapshot()

-- A test case for https://github.com/bee/bee/issues/112:
-- Bee crashes with SIGSEGV during reload configuration
--
-- log level
db.cfg{log_level=5}
-- constants
db.cfg{custom_proc_title="custom proc title"}
db.cfg{wal_dir="dynamic"}
db.cfg{snap_dir="dynamic"}
db.cfg{logger="new logger"}
-- bad1
db.cfg{slab_alloc_arena=0.2}
db.cfg.slab_alloc_arena

space:drop()
db.cfg{snap_io_rate_limit=0}
db.cfg{io_collect_interval=0}
db.cfg{too_long_threshold=0.5}
