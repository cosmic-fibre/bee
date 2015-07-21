s = db.schema.space.create('select', { temporary = true })
index1 = s:create_index('primary', { type = 'tree' })
index2 = s:create_index('second', { type = 'tree', unique = true,  parts = {2, 'num', 1, 'num'}})
for i = 1, 20 do s:insert({ i, 1, 2, 3 }) end

--------------------------------------------------------------------------------
-- get tests
--------------------------------------------------------------------------------

s.index[0]:get()
s.index[0]:get({})
s.index[0]:get(nil)
s.index[0]:get(1)
s.index[0]:get({1})
s.index[0]:get({1, 2})
s.index[0]:get(0)
s.index[0]:get({0})
s.index[0]:get("0")
s.index[0]:get({"0"})

s.index[1]:get(1)
s.index[1]:get({1})
s.index[1]:get({1, 2})

--------------------------------------------------------------------------------
-- select tests
--------------------------------------------------------------------------------

s.index[0]:select()
s.index[0]:select({})
s.index[0]:select(nil)
s.index[0]:select({}, {iterator = 'ALL'})
s.index[0]:select(nil, {iterator = db.index.ALL })
s.index[0]:select({}, {iterator = db.index.ALL, limit = 10})
s.index[0]:select(nil, {iterator = db.index.ALL, limit = 0})
s.index[0]:select({}, {iterator = 'ALL', limit = 1, offset = 15})
s.index[0]:select(nil, {iterator = 'ALL', limit = 20, offset = 15})

s.index[0]:select(nil, {iterator = db.index.EQ})
s.index[0]:select({}, {iterator = 'EQ'})
s.index[0]:select(nil, {iterator = 'REQ'})
s.index[0]:select({}, {iterator = db.index.REQ})

s.index[0]:select(nil, {iterator = 'EQ', limit = 2, offset = 1})
s.index[0]:select({}, {iterator = db.index.REQ, limit = 2, offset = 1})

s.index[0]:select(1)
s.index[0]:select({1})
s.index[0]:select({1, 2})
s.index[0]:select(0)
s.index[0]:select({0})
s.index[0]:select("0")
s.index[0]:select({"0"})

s.index[1]:select(1)
s.index[1]:select({1})
s.index[1]:select({1}, {limit = 2})
s.index[1]:select(1, {iterator = 'EQ'})
s.index[1]:select({1}, {iterator = db.index.EQ, offset = 16, limit = 2})
s.index[1]:select({1}, {iterator = db.index.REQ, offset = 16, limit = 2 })
s.index[1]:select({1, 2}, {iterator = 'EQ'})
s.index[1]:select({1, 2}, {iterator = db.index.REQ})
s.index[1]:select({1, 2})

s.index[0]:select(nil, { iterator = 'ALL', offset = 0, limit = 4294967295 })
s.index[0]:select({}, { iterator = 'ALL', offset = 0, limit = 4294967295 })

s.index[0]:select(1)
s.index[0]:select(1, { iterator = db.index.EQ })
s.index[0]:select(1, { iterator = 'EQ' })
s.index[0]:select(1, { iterator = 'GE' })
s.index[0]:select(1, { iterator = 'GE', limit = 2 })
s.index[0]:select(1, { iterator = 'LE', limit = 2 })
s.index[0]:select(1, { iterator = 'GE', offset = 10, limit = 2 })

s:select(2)

s:drop()

s = db.schema.space.create('select', { temporary = true })
index = s:create_index('primary', { type = 'tree' })
local a s:insert{0}
lots_of_links = {}
ref_count = 0
while (true) do table.insert(lots_of_links, s:get{0}) ref_count = ref_count + 1 end
ref_count
lots_of_links = {}
s:drop()
