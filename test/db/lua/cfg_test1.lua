#!/usr/bin/env bee
os = require('os')

db.cfg{
    listen              = os.getenv("LISTEN"),
}

require('console').listen(os.getenv('ADMIN'))
