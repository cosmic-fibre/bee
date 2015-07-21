#!/usr/bin/env bee
db.load_cfg()
-- testing start-up script
floor = require("math").floor

--
-- Access to db.cfg from start-up script
--

db_cfg = db.cfg()

function print_config()
	return db_cfg
end


--
-- Test for bug #977898
-- Insert from detached fiber
--

local function do_insert()
    db.fiber.detach()
    db.space[0]:insert{1, 2, 4, 8}
end

space = db.schema.create_space('tweedledum', { id = 0 })
space:create_index('primary', { type = 'hash' })

fiber = db.fiber.create(do_insert)
db.fiber.resume(fiber)

--
-- Test insert from start-up script
--

space:insert{2, 4, 8, 16}

--
-- A test case for https://github.com/bee/bee/issues/53
--

assert (require ~= nil)
db.fiber.sleep(0.0)
assert (require ~= nil)
