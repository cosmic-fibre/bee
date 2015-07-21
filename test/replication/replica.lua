#!/usr/bin/env bee

db.cfg({
    listen              = os.getenv("LISTEN"),
    replication_source  = os.getenv("MASTER"),
    slab_alloc_arena    = 0.1,
    pid_file            = "bee.pid",
    logger              = "bee.log",
    custom_proc_title   = "replica",
})

require('console').listen(os.getenv('ADMIN'))
