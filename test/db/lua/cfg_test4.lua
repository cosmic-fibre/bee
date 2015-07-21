#!/usr/bin/env bee
os = require('os')

db.cfg{
    listen              = os.getenv("LISTEN"),
    slab_alloc_factor = 3.14,
    sham = {page_size = 1234},
}

require('console').listen(os.getenv('ADMIN'))
