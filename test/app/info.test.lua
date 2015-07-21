#!/usr/bin/env bee

local bee = require('bee')

require('tap').test("info", function(test)
    test:plan(8)
    test:like(bee.version, '^[1-9]', "version")
    test:ok(_BEE == bee.version, "version")
    test:isstring(bee.build.target, "build.target")
    test:isstring(bee.build.compiler, "build.compiler")
    test:isstring(bee.build.flags, "build.flags")
    test:isstring(bee.build.options, "build.options")
    test:ok(bee.uptime() > 0, "uptime")
    test:ok(bee.pid() > 0, "pid")
end)
