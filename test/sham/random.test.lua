
-- random

space = db.schema.space.create('test', { engine = 'sham'})
index = space:create_index('primary', { type = 'tree', parts = {1, 'num'} })

dofile('index_random_test.lua')
index_random_test(space, 'primary')

space:drop()
sham_schedule()
