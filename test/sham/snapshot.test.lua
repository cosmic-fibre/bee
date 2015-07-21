
-- snapshot

os.execute("rm -f *.snap")
os.execute("rm -f *.xlog")
os.execute("touch mt")

--# stop server default
--# start server default

space = db.schema.space.create('test', { engine = 'sham' })
index = space:create_index('primary')

for key = 1, 351 do space:insert({key}) end
db.snapshot()

os.execute("rm -f mt")
os.execute("touch lock")

--# stop server default
--# start server default

space = db.space['test']
t = {}
for key = 1, 351 do table.insert(t, space:get({key})) end
t
space:drop()

os.execute("rm -f lock")
