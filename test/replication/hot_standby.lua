#!/usr/bin/env bee

require('console').listen(os.getenv('ADMIN'))
db.cfg({
    listen              = os.getenv("MASTER"),
    slab_alloc_arena    = 0.1,
    pid_file            = "bee.pid",
    logger              = "bee.log",
    custom_proc_title   = "hot_standby",
    wal_dir             = "..",
    snap_dir            = "..",
})

