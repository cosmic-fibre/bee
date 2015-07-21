s = db.schema.space.create('spatial')

-- rtree index as primary key must be forbidden (unique)
i = s:create_index('spatial', { type = 'rtree', unique = true, parts = {1, 'array'}})

-- any non-unique index as primary key must be forbidden
i = s:create_index('spatial', { type = 'hash', unique = false, parts = {1, 'num'}})
i = s:create_index('spatial', { type = 'tree', unique = false, parts = {1, 'num'}})
i = s:create_index('spatial', { type = 'rtree', unique = false, parts = {1, 'array'}})

-- tree and hash indexes over array field is not possible
i = s:create_index('primary', { type = 'tree', parts = {1, 'array'}})
i = s:create_index('primary', { type = 'hash', parts = {1, 'array'}})

-- normal indexes
i = s:create_index('primary', { type = 'tree', parts = {1, 'num'}})
i = s:create_index('secondary', { type = 'hash', parts = {2, 'num'}})

-- adding a tuple with array instead of num will fail
i = s:insert{{1, 2, 3}, 4}
i = s:insert{1, {2, 3, 4}}

-- rtree index must be one-part
i = s:create_index('spatial', { type = 'rtree', unique = false, parts = {1, 'array', 2, 'array'}})

-- unique rtree index is not possible
i = s:create_index('spatial', { type = 'rtree', unique = true, parts = {3, 'array'}})

-- num rtree index is not possible
i = s:create_index('spatial', { type = 'rtree', unique = false, parts = {3, 'num'}})

-- str rtree index is not possible
i = s:create_index('spatial', { type = 'rtree', unique = false, parts = {3, 'str'}})


-- normal rtree index
i = s:create_index('spatial', { type = 'rtree', unique = false, parts = {3, 'array'}})

-- inserting wrong values (should fail)
s:insert{1, 2, 3}
s:insert{1, 2, "3"}
s:insert{1, 2, nil, 3}
s:insert{1, 2, {}}
s:insert{1, 2, {"3", "4", "5", "6"}}
s:insert{1, 2, {nil, 4, 5, 6}}
s:insert{1, 2, {3, {4}, 5, 6}}
s:insert{1, 2, {3, 4, {}, 6}}
s:insert{1, 2, {3, 4, 5, "6"}}
s:insert{1, 2, {3}}
s:insert{1, 2, {3, 4, 5}}

-- inserting good value
s:insert{1, 2, {3, 4, 5, 6}}

-- invalid alters
s.index.spatial:alter({unique = true})
s.index.spatial:alter({type = 'tree'})
db.space[db.schema.SPACE_ID]:update({s.id}, {{"=", 4, 'sham'}})

-- chech that truncate works
s.index.spatial:select({0, 0, 10, 10}, {iterator = 'le'})
s:truncate()
s.index.spatial:select({0, 0, 10, 10}, {iterator = 'le'})

-- inserting lots of equvalent records
for i = 1,500 do s:insert{i, i, {3, 4, 5, 6}} end
-- and some records for chaos
for i = 1,10 do for j = 1,10 do s:insert{500+i+j*20, 500+i*20+j, {i, j, i, j}} end end
s.index.spatial:count()
#s.index.spatial:select({3, 4, 5, 6})
for i = 1,500,2 do s:delete{i} end
s.index.spatial:count()
#s.index.spatial:select({3, 4, 5, 6})

s.index.spatial:min()
s.index.spatial:max()

-- seems that drop can't fail
s.index.spatial:drop()
s.index.spatial:select({})

s:drop()

s = db.schema.space.create('sham', {engine = 'sham'})
-- rtree indexes are not enabled in sham
i = s:create_index('spatial', { type = 'rtree', unique = true, parts = {3, 'array'}})
i = s:create_index('primary', { type = 'tree', parts = {1, 'num'}})
-- ... even secondary
i = s:create_index('spatial', { type = 'rtree', unique = true, parts = {3, 'array'}})
s:drop()

-- rtree in temp space must work fine
s = db.schema.space.create('spatial', {temporary = true})
i = s:create_index('primary', { type = 'tree', parts = {1, 'num'}})
i = s:create_index('spatial', { type = 'rtree', unique = false, parts = {3, 'array'}})
s:insert{1, 2, {3, 4, 5, 6}}
s.index.spatial:select({0, 0, 10, 10}, {iterator = 'le'})
s:drop()

-- snapshot test
s = db.schema.space.create('spatial')
i = s:create_index('primary', { type = 'tree', parts = {1, 'num'}})
i = s:create_index('spatial', { type = 'rtree', unique = false, parts = {3, 'array'}})
for i = 1,10 do s:insert{i, i, {i, i, i + 1, i + 1}} end
db.snapshot()
for i = 11,20 do s:insert{i, i, i + 1, i + 1}
i:select({0, 0}, {iterator = 'neighbor'})
--# stop server default
--# start server default
s = db.space.spatial
i = s.index.spatial
i:select({0, 0}, {iterator = 'neighbor'})
s:drop()

