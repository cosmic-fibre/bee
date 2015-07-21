--# stop server default
--# start server default
space = db.schema.space.create('tweedledum')
index = space:create_index('primary', { type = 'hash' })
--# setopt delimiter ';'
i = 1;
while true do
    space:insert{space:len(), string.rep('test', i)}
    i = i + 1
end;
space:len();
i = 1;
while true do
    space:insert{space:len(), string.rep('test', i)}
    i = i + 1
end;
space:len();
i = 1;
while true do
    space:insert{space:len(), string.rep('test', i)}
    i = i + 1
end;
--# setopt delimiter ''
space:len()
space.index['primary']:get{0}
space.index['primary']:get{5}
space.index['primary']:get{9}
space.index['primary']:get{11}
space.index['primary']:get{15}
-- check that iterators work
i = 0
t = {}
--# setopt delimiter ';'
for state, v in space:pairs() do
    table.insert(t, v)
    i = i + 1
    if i == 50 then
        break
    end
end;
--# setopt delimiter ''
t
space:truncate()
space:insert{0, 'test'}
space.index['primary']:get{0}
collectgarbage('collect')
--
-- Check that statement-level rollback does not leak tuples
--
space:truncate()
function insert(a) space:insert(a) end
--# setopt delimiter ';'
function dup_key()
    db.begin()
    space:insert{1}
    local i = 1
    while i < 2000 do
        status, _ = pcall(insert, {1, string.rep('test', i)})
        if status then
            error('Unexpected success when inserting a duplicate')
        end
        if db.error.last().code ~= db.error.TUPLE_FOUND then
            db.error.raise()
        end
        i = i + 1
    end
    db.commit()
    return i
end;
--# setopt delimiter ''
dup_key()
space:select{}
--
-- Cleanup
--
space:drop()
t = nil
