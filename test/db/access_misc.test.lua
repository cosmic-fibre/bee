session = db.session
--
-- Check a double create space
--
s = db.schema.space.create('test')
s = db.schema.space.create('test')
--
-- Check a double drop space
--
s:drop()
s:drop()
--
-- Check double create user
--
db.schema.user.create('testus')
db.schema.user.create('testus')

s = db.schema.space.create('admin_space')
index = s:create_index('primary', {type = 'hash', parts = {1, 'NUM'}})
s:insert({1})
s:insert({2})
--
-- Check double grant and read access
--
db.schema.user.grant('testus', 'read', 'space', 'admin_space')
db.schema.user.grant('testus', 'read', 'space', 'admin_space')

session.su('testus')
s:select(1)
s:insert({3})
s:delete(1)
s:drop()
--
-- Check double revoke
--
session.su('admin')
db.schema.user.revoke('testus', 'read', 'space', 'admin_space')
db.schema.user.revoke('testus', 'read', 'space', 'admin_space')

session.su('testus')
s:select(1)
session.su('admin')
--
-- Check write access on space
-- 
db.schema.user.grant('testus', 'write', 'space', 'admin_space')

session.su('testus')
s:select(1)
s:delete(1)
s:insert({3})
s:drop()
session.su('admin')
--
-- Check double drop user
--
db.schema.user.drop('testus')
db.schema.user.drop('testus')
--
-- Check 'guest' user
--
session.su('guest')
session.uid()
db.space._user:select(1)
s:select(1)
s:insert({4})
s:delete({3})
s:drop()
gs = db.schema.space.create('guest_space')
db.schema.func.create('guest_func')

session.su('admin')
s:select()
--
-- Create user with universe read&write grants
-- and create this user session
--
db.schema.user.create('uniuser')
db.schema.user.grant('uniuser', 'read, write, execute', 'universe')
session.su('uniuser')
uid = session.uid()
--
-- Check universal user
-- Check delete currently authenticated user
--
db.schema.user.drop('uniuser')
--
--Check create, call and drop function
--
db.schema.func.create('uniuser_func')
function uniuser_func() return 'hello' end
uniuser_func()
db.schema.func.drop('uniuser_func')
--
-- Check create and drop space
--
us = db.schema.space.create('uniuser_space')
us:drop()
--
-- Check create and drop user
--
db.schema.user.create('uniuser_testus')
db.schema.user.drop('uniuser_testus')
--
-- Check access system and any spaces
--
db.space.admin_space:select()
db.space._user:select(1)
db.space._space:select(280)

us = db.schema.space.create('uniuser_space')
db.schema.func.create('uniuser_func')

session.su('admin')
db.schema.user.create('someuser')
db.schema.user.grant('someuser', 'read, write, execute', 'universe')
session.su('someuser')
--
-- Check drop objects of another user
--
s:drop()
us:drop()
db.schema.func.drop('uniuser_func')
db.schema.user.drop('uniuser_testus')

session.su('admin')
db.schema.func.drop('uniuser_func')
db.schema.user.drop('someuser')
db.schema.user.drop('uniuser_testus')
db.schema.user.drop('uniuser')
db.space._user:delete(uid)
s:drop()
--
-- Check write grant on _user
--
db.schema.user.create('testuser')
maxuid = db.space._user.index.primary:max()[1]

db.schema.user.grant('testuser', 'write', 'space', '_user')
session.su('testuser')
testuser_uid = session.uid()
db.space._user:delete(2)
db.space._user:select(1)
uid = db.space._user:insert{maxuid+1, session.uid(), 'someone', 'user'}[1]
db.space._user:delete(uid)

session.su('admin')
db.space._user:select(1)
db.space._user:delete(testuser_uid)
db.schema.user.revoke('testuser', 'write', 'space', '_user')
--
-- Check read grant on _user
--
db.schema.user.grant('testuser', 'read', 'space', '_user')
session.su('testuser')
db.space._user:delete(2)
db.space._user:select(1)
db.space._user:insert{uid, session.uid(), 'someone2', 'user'}

session.su('admin')
--
-- Check read grant on _index
--
db.schema.user.grant('testuser', 'read', 'space', '_index')
session.su('testuser')
db.space._index:select(272)
db.space._index:insert{512, 1,'owner','tree', 1, 1, 0,'num'}



session.su('admin')
db.schema.user.revoke('testuser', 'read, write, execute', 'universe')
--
-- Check that itertors check privileges
--
s = db.schema.space.create('glade') 
db.schema.user.grant('testuser', 'read', 'space', 'glade')
index = s:create_index('primary', {unique = true, parts = {1, 'NUM', 2, 'STR'}})
s:insert({1, 'A'})
s:insert({2, 'B'})
s:insert({3, 'C'})
s:insert({4, 'D'})

t = {}
for key, v in s.index.primary:pairs(3, {iterator = 'GE'}) do table.insert (t, v) end 
t
t = {}
session.su('testuser')
s:select()
for key, v in s.index.primary:pairs(3, {iterator = 'GE'}) do table.insert (t, v) end 
t
t = {}
session.su('admin')
db.schema.user.revoke('testuser', 'read', 'space', 'glade')
db.schema.user.grant('testuser', 'write', 'space', 'glade')
session.su('testuser')
s:select()
for key, v in s.index.primary:pairs(1, {iterator = 'GE'}) do table.insert (t, v) end 
t
t = {}
session.su('admin')
db.schema.user.grant('testuser', 'read, write, execute', 'space', 'glade')
session.su('testuser')
s:select()
for key, v in s.index.primary:pairs(3, {iterator = 'GE'}) do table.insert (t, v) end 
t
t = {}

session.su('guest')
s:select()
for key, v in s.index.primary:pairs(3, {iterator = 'GE'}) do table.insert (t, v) end 
t
t = {}

session.su('guest')
s:select()
for key, v in s.index.primary:pairs(3, {iterator = 'GE'}) do table.insert (t, v) end 
t

session.su('admin')
db.schema.user.drop('testuser')

s:drop()

db.space._user:select()
db.space._space:select()
db.space._func:select()

session = nil
