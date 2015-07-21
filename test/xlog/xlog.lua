#!/usr/bin/env bee
os = require('os')

db.cfg{
    listen              = os.getenv("LISTEN"),
    slab_alloc_arena    = 0.1,
    pid_file            = "bee.pid",
    panic_on_wal_error  = false,
    rows_per_wal        = 50
}

require('console').listen(os.getenv('ADMIN'))
