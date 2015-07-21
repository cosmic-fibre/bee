-- wal is off, good opportunity to test something more CPU intensive:
spaces = {}
db.schema.FORMAT_ID_MAX
--# setopt delimiter ';'
-- too many formats
for k = 1, db.schema.FORMAT_ID_MAX, 1 do
    local s = db.schema.space.create('space'..k)
    table.insert(spaces, s)
end;
#spaces;
-- cleanup
for k, v in pairs(spaces) do
    v:drop()
end;
