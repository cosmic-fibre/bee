-- temporary spaces
_space = db.space._space
-- not a temporary
FLAGS = 6
s = db.schema.space.create('t', { temporary = true })
s.temporary
s:drop()

-- not a temporary, too
s = db.schema.space.create('t', { temporary = false })
s.temporary
s:drop()

-- not a temporary, too
s = db.schema.space.create('t', { temporary = nil })
s.temporary
s:drop()

s = db.schema.space.create('t', { temporary = true })
index = s:create_index('primary', { type = 'hash' })

s:insert{1, 2, 3}
s:get{1}
s:len()

_ = _space:update(s.id, {{'=', FLAGS, 'temporary'}})
s.temporary
_ = _space:update(s.id, {{'=', FLAGS, ''}})
s.temporary

--# stop server default
--# start server default
FLAGS = 6
_space = db.space._space

s = db.space.t
s:len()
s.temporary

_ = _space:update(s.id, {{'=', FLAGS, 'no-temporary'}})
s.temporary
_ = _space:update(s.id, {{'=', FLAGS, ',:asfda:temporary'}})
s.temporary
_ = _space:update(s.id, {{'=', FLAGS, 'a,b,c,d,e'}})
s.temporary
_ = _space:update(s.id, {{'=', FLAGS, 'temporary'}})
s.temporary

s:get{1}
s:insert{1, 2, 3}

_ = _space:update(s.id, {{'=', FLAGS, 'temporary'}})
s.temporary
_ = _space:update(s.id, {{'=', FLAGS, 'no-temporary'}})
s.temporary

s:delete{1}
_ = _space:update(s.id, {{'=', FLAGS, 'no-temporary'}})
s:drop()
