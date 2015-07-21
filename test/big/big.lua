#!/usr/bin/env bee

db.cfg{
    listen              = os.getenv("LISTEN"),
    slab_alloc_arena    = 0.1,
    pid_file            = "bee.pid",
    rows_per_wal        = 500000
}

require('console').listen(os.getenv('ADMIN'))
