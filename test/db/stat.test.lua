-- clear statistics
--# stop server default
--# start server default
db.stat.INSERT.total
db.stat.DELETE.total
db.stat.UPDATE.total
db.stat.REPLACE.total
db.stat.SELECT.total

space = db.schema.space.create('tweedledum')
index = space:create_index('primary', { type = 'hash' })

-- check stat_cleanup
-- add several tuples
for i=1,10 do space:insert{i, 'tuple'..tostring(i)} end
db.stat.INSERT.total
db.stat.DELETE.total
db.stat.UPDATE.total
db.stat.REPLACE.total
db.stat.SELECT.total

--# stop server default
--# start server default

-- statistics must be zero
db.stat.INSERT.total
db.stat.DELETE.total
db.stat.UPDATE.total
db.stat.REPLACE.total
db.stat.SELECT.total

-- cleanup
db.space.tweedledum:drop()
