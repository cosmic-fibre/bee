#!/usr/bin/env bee

print('Hello, World!')

--
-- Command-line argument handling
--
local script = io.open('script.lua', 'w')
script:write([[
-- Bee binary
print('arg[-1]', arg[-1]:match('bee'))
-- Script name
print('arg[0] ', arg[0])
-- Command-line arguments
print('arg', arg[1], arg[2], arg[3])
print('...', ...)
]])
script:close()

os.execute("bee ./script.lua 1 2 3")
