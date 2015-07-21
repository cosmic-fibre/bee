#!/usr/bin/env bee

require('suite')

if not file_exists('lock') then
	sham_rmdir()
	sham_mkdir()
end

local sham = {
	threads = 0
}

if file_exists('mt') then
	sham.threads = 3
end

db.cfg {
    listen            = os.getenv("LISTEN"),
    slab_alloc_arena  = 0.1,
    pid_file          = "bee.pid",
    rows_per_wal      = 50,
    sham_dir        = "./sham_test",
    sham            = sham,
    custom_proc_title = "default"
}

require('console').listen(os.getenv('ADMIN'))
