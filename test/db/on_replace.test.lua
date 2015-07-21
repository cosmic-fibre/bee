ts = db.schema.space.create('test_space')
ti = ts:create_index('primary', { type = 'hash' })

type(ts.on_replace)
ts.on_replace()
ts:on_replace()
ts:on_replace(123)

function fail(old_tuple, new_tuple) error('test') end
ts:on_replace(fail)

ts:insert{1, 'b', 'c'}
ts:get{1}

ts:on_replace(nil, fail)

ts:insert{1, 'b', 'c'}
ts:get{1}

function fail(old_tuple, new_tuple) error('abc') end
ts:on_replace(fail)

ts:insert{2, 'b', 'c'}
ts:get{2}

function save_out(told, tnew) o = told n = tnew end
ts:on_replace(save_out, fail)

ts:insert{2, 'a', 'b', 'c'}
o
n

ts:replace{2, 'd', 'e', 'f'}
o
n

ts:drop()
