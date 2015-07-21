
-- gh-283: Sham: hang after three creates and drops

s = db.schema.space.create('space0', {engine='sham'})
i = s:create_index('space0', {type = 'tree', parts = {1, 'STR'}})
s:insert{'a', 'b', 'c'}
s:drop()
sham_schedule()

s = db.schema.space.create('space0', {engine='sham'})
i = s:create_index('space0', {type = 'tree', parts = {1, 'STR'}})
s:insert{'a', 'b', 'c'}
t = s.index[0]:select({}, {iterator = db.index.ALL})
t
s:drop()
sham_schedule()

s = db.schema.space.create('space0', {engine='sham'})
i = s:create_index('space0', {type = 'tree', parts = {1, 'STR'}})
s:insert{'a', 'b', 'c'}
t = s.index[0]:select({}, {iterator = db.index.ALL})
t
s:drop()
sham_schedule()

-- gh-280: Sham: crash if insert without index

s = db.schema.space.create('test', {engine='sham'})
s:insert{'a'}
s:drop()
sham_schedule()

-- gh-436: No error when creating temporary sham space

s = db.schema.space.create('tester',{engine='sham', temporary=true})

-- gh-432: Sham: ignored limit

s = db.schema.space.create('tester',{engine='sham'})
i = s:create_index('sham_index', {})
for v=1, 100 do s:insert({v}) end
t = s:select({''},{iterator='GT', limit =1})
t
t = s:select({},{iterator='GT', limit =1})
t
s:drop()
sham_schedule()

s = db.schema.space.create('tester', {engine='sham'})
i = s:create_index('sham_index', {type = 'tree', parts = {1, 'STR'}})
for v=1, 100 do s:insert({tostring(v)}) end
t = s:select({''},{iterator='GT', limit =1})
t
t = s:select({},{iterator='GT', limit =1})
t
s:drop()
sham_schedule()

-- gh-680: Sham: assertion on update
s = db.schema.space.create('tester', {engine='sham'})
i = s:create_index('primary',{type = 'tree', parts = {2, 'STR'}})
s:insert{1,'X'}
s:update({'X'}, {{'=', 2, 'Y'}})
s:select{'X'}
s:select{'Y'}
s:update({'X'}, {{'=', 3, 'Z'}})
s:select{'X'}
s:select{'Y'}
s:drop()
sham_schedule()
