
a = db.schema.space.create('test', { engine = 'sham' })
index = a:create_index('primary', { type = 'tree', parts = {1, 'num'} })

b = db.schema.space.create('test_tmp', { engine = 'sham' })
index = b:create_index('primary', { type = 'tree', parts = {1, 'num'} })

-- begin/rollback

db.begin()
for key = 1, 10 do a:insert({key}) end
t = {}
for key = 1, 10 do table.insert(t, a:select({key})[1]) end
t
for key = 1, 10 do b:insert({key}) end
t = {}
for key = 1, 10 do table.insert(t, b:select({key})[1]) end
t
db.rollback()

t = {}
for key = 1, 10 do assert(#a:select({key}) == 0) end
t
for key = 1, 10 do assert(#b:select({key}) == 0) end
t

-- begin/commit insert

db.begin()
t = {}
for key = 1, 10 do a:insert({key}) end
t = {}
for key = 1, 10 do b:insert({key}) end
t = {}
db.commit()

t = {}
for key = 1, 10 do table.insert(t, a:select({key})[1]) end
t
t = {}
for key = 1, 10 do table.insert(t, b:select({key})[1]) end
t

-- begin/commit delete

db.begin()
for key = 1, 10 do a:delete({key}) end
for key = 1, 10 do b:delete({key}) end
db.commit()

t = {}
for key = 1, 10 do assert(#a:select({key}) == 0) end
t
for key = 1, 10 do assert(#b:select({key}) == 0) end
t

a:drop()
sham_schedule()
b:drop()
sham_schedule()
