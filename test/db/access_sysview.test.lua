session = db.session

--
-- Basic tests
--

#db.space._vspace:select{} == #db.space._space:select{}
#db.space._vindex:select{} == #db.space._index:select{}
#db.space._vuser:select{} == #db.space._user:select{}
#db.space._vpriv:select{} == #db.space._priv:select{}
#db.space._vfunc:select{} == #db.space._func:select{}

-- error: sysview does not support replace()
db.space._vspace:replace({1, 1, 'test'})

session.su('guest')

--
-- _vspace + _vindex
--
-- _vXXXX views are visible for 'public' role
#db.space._vspace.index[2]:select('_vspace') ~= 0
#db.space._vspace.index[2]:select('_vindex') ~= 0
#db.space._vspace.index[2]:select('_vuser') ~= 0
#db.space._vspace.index[2]:select('_vfunc') ~= 0
#db.space._vspace.index[2]:select('_vpriv') ~= 0

#db.space._vindex:select(db.space._vspace.id) > 0
#db.space._vindex:select(db.space._vindex.id) > 0
#db.space._vindex:select(db.space._vuser.id) > 0
#db.space._vindex:select(db.space._vfunc.id) > 0
#db.space._vindex:select(db.space._vpriv.id) > 0

db.session.su('admin')
db.schema.user.revoke('guest', 'public')
db.session.su('guest')

#db.space._vspace:select{}
#db.space._vindex:select{}
#db.space._vuser:select{}
#db.space._vpriv:select{}
#db.space._vfunc:select{}

db.session.su('admin')
db.schema.user.grant('guest', 'public')
db.session.su('guest')

#db.space._vspace:select{}
#db.space._vindex:select{}

db.session.su('admin')
s = db.schema.space.create('test')
s = db.space.test:create_index('primary')
db.schema.role.grant('public', 'read', 'space', 'test')
db.session.su('guest')

db.space._vspace.index[2]:get('test') ~= nil
#db.space._vindex:select(db.space.test.id) == 1

db.session.su('admin')
db.schema.role.revoke('public', 'read', 'space', 'test')
db.session.su('guest')

db.space._vspace.index[2]:get('test') == nil
#db.space._vindex:select(db.space.test.id) == 0

db.session.su('admin')
db.schema.user.grant('guest', 'read', 'space', 'test')
db.session.su('guest')

db.space._vspace.index[2]:get('test') ~= nil
#db.space._vindex:select(db.space.test.id) == 1

db.session.su('admin')
db.schema.user.revoke('guest', 'read', 'space', 'test')
db.session.su('guest')

db.space._vspace.index[2]:get('test') == nil
#db.space._vindex:select(db.space.test.id) == 0

-- check universe permissions
db.session.su('admin')
db.schema.user.grant('guest', 'read', 'universe')
db.session.su('guest')

#db.space._vspace:select{}
#db.space._vindex:select{}
#db.space._vuser:select{}
#db.space._vpriv:select{}
#db.space._vfunc:select{}

db.session.su('admin')
db.schema.user.revoke('guest', 'read', 'universe')
db.schema.user.grant('guest', 'write', 'universe')
db.session.su('guest')

#db.space._vindex:select{}
#db.space._vuser:select{}
#db.space._vpriv:select{}
#db.space._vfunc:select{}

db.session.su('admin')
db.schema.user.revoke('guest', 'write', 'universe')
db.space.test:drop()
db.session.su('guest')

-- read access to original space also allow to read a view
db.session.su('admin')
space_cnt = #db.space._space:select{}
index_cnt = #db.space._index:select{}
db.schema.user.grant('guest', 'read', 'space', '_space')
db.schema.user.grant('guest', 'read', 'space', '_index')
db.session.su('guest')
#db.space._vspace:select{} == space_cnt
#db.space._vindex:select{} == index_cnt
db.session.su('admin')
db.schema.user.revoke('guest', 'read', 'space', '_space')
db.schema.user.revoke('guest', 'read', 'space', '_index')
db.session.su('guest')
#db.space._vspace:select{} < space_cnt
#db.space._vindex:select{} < index_cnt

--
-- _vuser
--

-- a guest user can read information about itself
t = db.space._vuser:select(); return #t == 1 and t[1][3] == 'guest'

-- read access to original space also allow to read a view
db.session.su('admin')
user_cnt = #db.space._user:select{}
db.schema.user.grant('guest', 'read', 'space', '_user')
db.session.su('guest')
#db.space._vuser:select{} == user_cnt
db.session.su('admin')
db.schema.user.revoke('guest', 'read', 'space', '_user')
db.session.su('guest')
#db.space._vuser:select{} < user_cnt

db.session.su('admin')
db.schema.user.grant('guest', 'read,write', 'universe')
db.session.su('guest')

db.schema.user.create('tester')

db.session.su('admin')
db.schema.user.revoke('guest', 'read,write', 'universe')
db.session.su('guest')

#db.space._vuser.index[2]:select('tester') > 0

db.session.su('admin')
db.schema.user.drop('tester')
db.session.su('guest')

--
-- _vpriv
--

-- a guest user can see granted 'public' role
db.space._vpriv.index[2]:select('role')[1][2] == session.uid()

-- read access to original space also allow to read a view
db.session.su('admin')
db.schema.user.grant('guest', 'read', 'space', '_priv')
priv_cnt = #db.space._priv:select{}
db.session.su('guest')
#db.space._vpriv:select{} == priv_cnt
db.session.su('admin')
db.schema.user.revoke('guest', 'read', 'space', '_priv')
db.session.su('guest')
cnt = #db.space._vpriv:select{}

cnt < priv_cnt

db.session.su('admin')
db.schema.user.grant('guest', 'read,write', 'space', '_schema')
db.session.su('guest')

#db.space._vpriv:select{} == cnt + 1

db.session.su('admin')
db.schema.user.revoke('guest', 'read,write', 'space', '_schema')
db.session.su('guest')

#db.space._vpriv:select{} == cnt

--
-- _vfunc
--

db.session.su('admin')
db.schema.func.create('test')

-- read access to original space also allow to read a view
func_cnt = #db.space._func:select{}
db.schema.user.grant('guest', 'read', 'space', '_func')
db.session.su('guest')
#db.space._vfunc:select{} == func_cnt
db.session.su('admin')
db.schema.user.revoke('guest', 'read', 'space', '_func')
db.session.su('guest')
cnt = #db.space._vfunc:select{}

cnt < func_cnt

db.session.su('admin')
db.schema.user.grant('guest', 'execute', 'function', 'test')
db.session.su('guest')

#db.space._vfunc:select{} = cnt + 1

db.session.su('admin')
db.schema.user.revoke('guest', 'execute', 'function', 'test')
db.session.su('guest')

#db.space._vfunc:select{} == cnt

db.session.su('admin')
db.schema.user.grant('guest', 'execute', 'universe')
db.session.su('guest')

#db.space._vfunc:select{} == cnt + 1

db.session.su('admin')
db.schema.user.revoke('guest', 'execute', 'universe')
db.schema.func.drop('test')
db.session.su('guest')

#db.space._vfunc:select{} == cnt

--
-- view:alter() tests
--

session.su('admin')
db.space._vspace.index[1]:alter({parts = { 2, 'str' }})
db.space._vspace.index[1]:select('xxx')
db.space._vspace.index[1]:select(1)
db.space._vspace.index[1]:alter({parts = { 2, 'num' }})
db.space._space.index[1]:drop()
db.space._vspace.index[1]:select(1)
s = db.space._space:create_index('owner', {parts = { 2, 'num' }, id = 1, unique = false})
#db.space._vspace.index[1]:select(1) > 0

session = nil
