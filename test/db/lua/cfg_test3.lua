#!/usr/bin/env bee
os = require('os')

db.cfg{
    listen              = os.getenv("LISTEN"),
    slab_alloc_arena = 0.2,
    sham = {threads = 10},
}

require('console').listen(os.getenv('ADMIN'))
