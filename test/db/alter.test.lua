_space = db.space[db.schema.SPACE_ID]
_index = db.space[db.schema.INDEX_ID]
ADMIN = 1
--
-- Test insertion into a system space - verify that
-- mandatory fields are required.
--
_space:insert{_space.id, ADMIN, 'test', 5 }
--
-- Bad space id
--
_space:insert{'hello', 'world', 'test'}
--
-- Can't create a space which has wrong field count - field_count must be NUM
--
_space:insert{_space.id, ADMIN, 'test', 'world'}
--
-- There is already a tuple for the system space
--
_space:insert{_space.id, ADMIN, '_space', 'memtx', 0}
_space:replace{_space.id, ADMIN, '_space', 'memtx', 0}
_space:insert{_index.id, ADMIN, '_index', 'memtx', 0}
_space:replace{_index.id, ADMIN, '_index', 'memtx', 0}
--
-- Can't change properties of a space
--
_space:replace{_space.id, ADMIN, '_space', 'memtx', 0}
--
-- Can't drop a system space
--
_space:delete{_space.id}
_space:delete{_index.id}
--
-- Can't change properties of a space
--
_space:update({_space.id}, {{'-', 1, 1}})
_space:update({_space.id}, {{'-', 1, 2}})
--
-- Create a space
--
t = _space:auto_increment{ADMIN, 'hello', 'memtx', 0}
-- Check that a space exists
space = db.space[t[1]]
space.id
space.field_count
space.index[0]
--
-- check dml - the space has no indexes yet, but must not crash on DML
--
space:select{0}
space:insert{0, 0}
space:replace{0, 0}
space:update({0}, {{'+', 1, 1}})
space:delete{0}
t = _space:delete{space.id}
space_deleted = db.space[t[1]]
space_deleted
space:replace{0}
_index:insert{_space.id, 0, 'primary', 'tree', 1, 1, 0, 'num'}
_index:replace{_space.id, 0, 'primary', 'tree', 1, 1, 0, 'num'}
_index:insert{_index.id, 0, 'primary', 'tree', 1, 2, 0, 'num', 1, 'num'}
_index:replace{_index.id, 0, 'primary', 'tree', 1, 2, 0, 'num', 1, 'num'}
_index:select{}
-- modify indexes of a system space
_index:delete{_index.id, 0}
_space:insert{1000, ADMIN, 'hello', 'memtx', 0}
_index:insert{1000, 0, 'primary', 'tree', 1, 1, 0, 'num'}
db.space[1000]:insert{0, 'hello, world'}
db.space[1000]:drop()
db.space[1000]
-- test that after disabling triggers on system spaces we still can
-- get a correct snapshot
_index:run_triggers(false)
_space:run_triggers(false)
db.snapshot()
--# stop server default
--# start server default
ADMIN = 1
db.space['_space']:insert{1000, ADMIN, 'test', 'memtx', 0}
db.space[1000].id
db.space['_space']:delete{1000}
db.space[1000]

--------------------------------------------------------------------------------
-- #197: db.space.space0:len() returns an error if there is no index
--------------------------------------------------------------------------------

space = db.schema.space.create('gh197')
space:len()
space:truncate()
space:pairs():totable()
space:drop()

--------------------------------------------------------------------------------
-- #198: names like '' and 'x.y' and 5 and 'primary ' are legal
--------------------------------------------------------------------------------

-- invalid identifiers
db.schema.space.create('invalid.identifier')
db.schema.space.create('invalid identifier')
db.schema.space.create('primary ')
db.schema.space.create('5')
db.schema.space.create('')

-- valid identifiers
db.schema.space.create('_Abcde'):drop()
db.schema.space.create('_5'):drop()
db.schema.space.create('valid_identifier'):drop()
db.schema.space.create('ынтыпрайзный_空間'):drop() -- unicode
db.schema.space.create('utf8_наше_Фсё'):drop() -- unicode

space = db.schema.space.create('test')

-- invalid identifiers
space:create_index('invalid.identifier')
space:create_index('invalid identifier')
space:create_index('primary ')
space:create_index('5')
space:create_index('')

space:drop()
-- gh-57 Confusing error message when trying to create space with a
-- duplicate id
auto = db.schema.space.create('auto_original')
auto2 = db.schema.space.create('auto', {id = auto.id})
db.schema.space.drop('auto')
auto2
db.schema.space.create('auto_original', {id = auto.id})
auto:drop()

-- ------------------------------------------------------------------
-- gh-281 Crash after rename + replace + delete with multi-part index
-- ------------------------------------------------------------------
s = db.schema.space.create('space')
index = s:create_index('primary', {unique = true, parts = {1, 'NUM', 2, 'STR'}})
s:insert{1, 'a'}
db.space.space.index.primary:rename('secondary')
db.space.space:replace{1,'The rain in Spain'}
db.space.space:delete{1,'The rain in Spain'}
db.space.space:select{}
s:drop()

-- ------------------------------------------------------------------
-- gh-362 Appropriate error messages in create_index
-- ------------------------------------------------------------------
s = db.schema.space.create(42)
s = db.schema.space.create("test", "bug")
s = db.schema.space.create("test", {unknown = 'param'})
s = db.schema.space.create("test")
index = s:create_index('primary', {unique = true, parts = {0, 'NUM', 1, 'STR'}})
index = s:create_index('primary', {unique = true, parts = {'NUM', 1, 'STR', 2}})
index = s:create_index('primary', {unique = true, parts = 'bug'})
s:drop()


-- ------------------------------------------------------------------
-- gh-155 Bee failure on simultaneous space:drop()
-- ------------------------------------------------------------------

--# setopt delimiter ';'
local fiber = require('fiber')
local W = 4
local N = 50
local ch = fiber.channel(W)
for i=1,W do
    fiber.create(function()
        for k=1,N do
            local space_id = math.random(2147483647)
            local space = db.schema.space.create(string.format('space_%d', space_id))
            space:create_index('pk', { type = 'tree' })
            space:drop()
        end
        ch:put(true)
    end)
end
for i=1,W do
    ch:get()
end
--# setopt delimiter ''
