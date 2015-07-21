#!/usr/bin/env bee
--
-- Test floating points values (too_long_treshold) with fractional part
--
db.cfg{
    listen = 3313,
    slab_alloc_arena = 0.1,
    pid_file = "db.pid",
    rows_per_wal = 50,
    too_long_threshold = 0.01,
    logger="bee.log"
}

if db.space.space1 ~= nil then
    db.space.space1:drop()
end
s = db.schema.space.create('space1')
s:create_index('primary', {type = 'hash', parts = {1, 'NUM'}})

t = {}
for k,v in pairs(db.cfg) do if type(v) ~= 'table' and type(v) ~= 'function' then table.insert(t,k..':'..tostring(v)) end end

print('db.cfg')
for k,v in pairs(t) do print(k, v) end
print('------------------------------------------------------')
x = db.cfg.too_long_threshold
print('Check that too_long_threshold = 0.01')
print(x)
t = nil
s:drop()
os.exit()

