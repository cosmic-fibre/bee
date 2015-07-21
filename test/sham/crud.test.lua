
-- insert

space = db.schema.space.create('test', { engine = 'sham' })
index = space:create_index('primary', { type = 'tree', parts = {1, 'num'} })
sham_dir()[1]
for key = 1, 132 do space:insert({key}) end
t = {}
for key = 1, 132 do table.insert(t, space:get({key})) end
t

-- replace/get

for key = 1, 132 do space:replace({key, key}) end
t = {}
for key = 1, 132 do table.insert(t, space:get({key})) end
t

-- update/get

for key = 1, 132 do space:update({key}, {{'+', 2, key}}) end
t = {}
for key = 1, 132 do table.insert(t, space:get({key})) end
t

-- delete/get

for key = 1, 132 do space:delete({key}) end
for key = 1, 132 do assert(space:get({key}) == nil) end

-- delete nonexistent
space:delete({1234})

-- select

for key = 1, 96 do space:insert({key}) end
index = space.index[0]
index:select({}, {iterator = db.index.ALL})
index:select({}, {iterator = db.index.GE})
index:select(4,  {iterator = db.index.GE})
index:select({}, {iterator = db.index.GT})
index:select(4,  {iterator = db.index.GT})
index:select({}, {iterator = db.index.LE})
index:select(7,  {iterator = db.index.LE})
index:select({}, {iterator = db.index.LT})
index:select(7,  {iterator = db.index.LT})

space:drop()
sham_schedule()
sham_dir()[1]
