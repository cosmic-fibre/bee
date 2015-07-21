#!/usr/bin/env bee

print[[
--------------------------------------------------------------------------------
-- #267: Bad exception catching
--------------------------------------------------------------------------------
]]

db.cfg{
    logger="bee.log",
    slab_alloc_arena=0.1,
}
function pcalltest()
    local ERRMSG = "module 'some_invalid_module' not found"
    local status, msg = pcall(require, 'some_invalid_module')
    if status == false and msg ~= nil and msg:match(ERRMSG) ~= nil then
        return 'pcall is ok'
    else
        return 'pcall is broken'
    end
end

local status, msg = xpcall(pcalltest, function(msg)
    print('error handler:', msg)
end)
print('pcall inside xpcall:', status, msg)

local status, msg = pcall(function() error('some message') end)
print('pcall with Lua error():', status, msg:match('some message'))

local status, msg = pcall(function()
    db.error(db.error.ILLEGAL_PARAMS, 'some message')
end)
print('pcall with db.error():', status, msg)

print('pcall with no return:', select('#', pcall(function() end)))
print('pcall with multireturn:', pcall(function() return 1, 2, 3 end))
os.exit(0)
