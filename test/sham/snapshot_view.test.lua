
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

-- remove bee xlogs
os.execute("rm -f *.xlog")

os.execute("rm -f mt")
os.execute("touch lock")

--# stop server default
--# start server default

space = db.space['test']
space:len()
sham_dir()[1]
space:drop()
sham_schedule()
sham_dir()[1]

os.execute("rm -f *.snap")
os.execute("rm -f *.xlog")
os.execute("rm -f lock")

--# stop server default
--# start server default
