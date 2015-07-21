
os.execute("rm -f *.snap")
os.execute("rm -f *.xlog")
os.execute("touch mt")

--# stop server default
--# start server default

space = db.schema.create_space('test', { engine = 'sham' })
index = space:create_index('primary')

for key = 1, 351 do space:insert({key}) end
db.snapshot()
space:drop()
sham_schedule()
sham_dir()[1]

-- ensure that previous space has been garbage collected
space = db.schema.create_space('test', { engine = 'sham' })
index = space:create_index('primary')
for key = 1, 351 do space:insert({key}) end
sham_dir()[1] -- 2
db.snapshot()
space:drop()
sham_schedule()
sham_dir()[1] -- 1

space = db.schema.create_space('test', { engine = 'sham' })
index = space:create_index('primary')
for key = 1, 351 do space:insert({key}) end
sham_dir()[1] -- 2
db.snapshot()
space:drop()
sham_schedule()
sham_dir()[1] -- 1

os.execute("rm -f *.snap")
os.execute("rm -f *.xlog")
os.execute("rm -f mt")
os.execute("rm -f lock")

--# stop server default
--# start server default
