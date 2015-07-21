dofile('utils.lua')

hash = db.schema.space.create('tweedledum')
tmp = hash:create_index('primary', { type = 'hash', parts = {1, 'num', 2, 'str', 3, 'num'}, unique = true })
tmp = hash:create_index('unique', { type = 'hash', parts = {3, 'num', 5, 'num'}, unique = true })

-- insert rows
hash:insert{0, 'foo', 0, '', 1}
hash:insert{0, 'foo', 1, '', 1}
hash:insert{1, 'foo', 0, '', 2}
hash:insert{1, 'foo', 1, '', 2}
hash:insert{0, 'bar', 0, '', 3}
hash:insert{0, 'bar', 1, '', 3}
hash:insert{1, 'bar', 0, '', 4}
hash:insert{1, 'bar', 1, '', 4}

-- try to insert a row with a duplicate key
hash:insert{1, 'bar', 1, '', 5}

-- output all rows

--# setopt delimiter ';'
function db.select_all()
    local result = {}
    local tuple, v
    for tuple, v in hash:pairs() do
        table.insert(result, v)
    end
    return result
end;
--# setopt delimiter ''
db.sort(db.select_all())

-- primary index select
hash.index['primary']:get{1, 'foo', 0}
hash.index['primary']:get{1, 'bar', 0}
-- primary index select with missing part
hash.index['primary']:get{1, 'foo'}
-- primary index select with extra part
hash.index['primary']:get{1, 'foo', 0, 0}
-- primary index select with wrong type
hash.index['primary']:get{1, 'foo', 'baz'}

-- secondary index select
hash.index['unique']:get{1, 4}
-- secondary index select with no such key
hash.index['unique']:get{1, 5}
-- secondary index select with missing part
hash.index['unique']:get{1}
-- secondary index select with wrong type
hash.index['unique']:select{1, 'baz'}

-- cleanup
hash:truncate()
hash:len()
hash:drop()
