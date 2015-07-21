
-- space create/drop

space = db.schema.space.create('test', { engine = 'sham' })
sham_dir()[1]
space:drop()
sham_schedule()
sham_dir()[1]

-- index create/drop

space = db.schema.space.create('test', { engine = 'sham' })
index = space:create_index('primary')
sham_dir()[1]
space:drop()
sham_schedule()
sham_dir()[1]

-- index create/drop alter

space = db.schema.space.create('test', { engine = 'sham' })
index = space:create_index('primary')
sham_dir()[1]
_index = db.space[db.schema.INDEX_ID]
_index:delete{102, 0}
space:drop()
sham_schedule()
sham_dir()[1]

-- index create/drop tree string

space = db.schema.space.create('test', { engine = 'sham' })
index = space:create_index('primary', {type = 'tree', parts = {1, 'STR'}})
space:insert({'test'})
space:drop()
sham_schedule()
sham_dir()[1]

-- index create/drop tree num

space = db.schema.space.create('test', { engine = 'sham' })
index = space:create_index('primary', {type = 'tree', parts = {1, 'num'}})
space:insert({13})
space:drop()
sham_schedule()
sham_dir()[1]

-- index create hash 

space = db.schema.space.create('test', { engine = 'sham' })
index = space:create_index('primary', {type = 'hash'})
space:drop()
sham_schedule()
sham_dir()[1]

-- secondary index create

space = db.schema.space.create('test', { engine = 'sham' })
index1 = space:create_index('primary')
index2 = space:create_index('secondary')
space:drop()
sham_schedule()
sham_dir()[1]

-- index size

space = db.schema.space.create('test', { engine = 'sham' })
index = space:create_index('primary')
primary = space.index[0]
primary:len()
space:insert({13})
space:insert({14})
space:insert({15})
primary:len()
space:drop()
sham_schedule()
