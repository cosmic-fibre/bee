#!/usr/bin/env bee

require('suite')

os.execute("rm -rf sham_test")
os.execute("mkdir -p sham_test")

local sham = {
	threads = 5
}

db.cfg {
    listen            = os.getenv("LISTEN"),
    slab_alloc_arena  = 0.1,
    pid_file          = "bee.pid",
    rows_per_wal      = 500000,
    sham_dir        = "./sham_test",
    sham            = sham,
    custom_proc_title = "default"
}

require('console').listen(os.getenv('ADMIN'))
