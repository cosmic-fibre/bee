-- Test Lua from admin console. Whenever producing output,
-- make sure it's a valid YAML
db.info.unknown_variable
db.info[23]
db.info['unknown_variable']
string.match(db.info.version, '^[1-9]') ~= nil
string.match(db.info.pid, '^[1-9][0-9]*$') ~= nil
#db.info.server > 0
db.info.replication.status
db.info.status
string.len(db.info.uptime) > 0
string.match(db.info.uptime, '^[1-9][0-9]*$') ~= nil
t = {}
for k, _ in pairs(db.info()) do table.insert(t, k) end
table.sort(t)
t
db.info.snapshot_pid
