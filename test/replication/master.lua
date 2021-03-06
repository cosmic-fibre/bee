#!/usr/bin/env bee
os = require('os')
db.cfg({
    listen              = os.getenv("LISTEN"),
    slab_alloc_arena    = 0.1,
    pid_file            = "bee.pid",
    logger              = "| cat - >> bee.log",
    custom_proc_title   = "master",
})

require('console').listen(os.getenv('ADMIN'))
