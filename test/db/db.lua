#!/usr/bin/env bee
os = require('os')

db.cfg{
    listen              = os.getenv("LISTEN"),
    slab_alloc_arena    = 0.1,
    pid_file            = "bee.pid",
    rows_per_wal        = 50
}

require('console').listen(os.getenv('ADMIN'))
