--
-- Access control tests which require a binary protocol
-- connection to the server
--
db.schema.user.grant('guest','read,write,execute','universe')
session = db.session
remote = require('net.db')
c = remote:new(db.cfg.listen)
c:call("dostring", "session.su('admin')")
c:call("dostring", "return session.user()")
c:close()
db.schema.user.revoke('guest', 'read,write,execute', 'universe')

-- gh-488 suid functions
--
setuid_space = db.schema.space.create('setuid_space')
index = setuid_space:create_index('primary')
setuid_func = function() return db.space.setuid_space:auto_increment{} end
db.schema.func.create('setuid_func')
db.schema.user.grant('guest', 'execute', 'function', 'setuid_func')
c = remote:new(db.cfg.listen)
c:call("setuid_func")
session.su('guest')
setuid_func()
session.su('admin')
db.schema.func.drop('setuid_func')
db.schema.func.create('setuid_func', { setuid = true })
db.schema.user.grant('guest', 'execute', 'function', 'setuid_func')
c:call("setuid_func")
session.su('guest')
setuid_func()
session.su('admin')
c:close()
-- OPENTAR-84: crash in on_replace_dd_func during recovery
-- _func space recovered after _user space, so setuid option can be
-- handled incorrectly
db.snapshot()
--# stop server default
--# start server default
remote = require('net.db')
session = db.session
setuid_func = function() return db.space.setuid_space:auto_increment{} end
c = remote:new(db.cfg.listen)
c:call("setuid_func")
session.su('guest')
setuid_func()
session.su('admin')
c:close()
db.schema.func.drop('setuid_func')
db.space.setuid_space:drop()
--
-- gh-530 "assertion failed"
-- If a user is dropped, its session should not be usable
-- any more
--
test = db.schema.space.create('test')
index = test:create_index('primary')
db.schema.user.create('test', {password='test'})
db.schema.user.grant('test', 'read,write', 'space','test')
db.schema.user.grant('test', 'read', 'space', '_space')
db.schema.user.grant('test', 'read', 'space', '_index')
net = require('net.db')
c = net.new('test:test@'..db.cfg.listen)
c.space.test:insert{1}
db.schema.user.drop('test')
c.space.test:insert{1}
c:close()
test:drop()

--
-- gh-575: User loses 'universe' grants after alter
--

db.space._priv:get{1}
u = db.space._user:get{1}
db.session.su('admin')
db.schema.user.passwd('Gx5!')
c = require('net.db').new('admin:Gx5!@'..db.cfg.listen)
c:call('dostring', 'return 2 + 2')
c:close()
db.space._user:replace(u)
--
-- Roles: test that universal access of an authenticated
-- session is not updated if grant is made from another
-- session
--
test = db.schema.space.create('test')
_ = test:create_index('primary')
test:insert{1}
db.schema.user.create('test', {password='test'})
db.schema.user.grant('test', 'read', 'space', '_space')
db.schema.user.grant('test', 'read', 'space', '_index')
net = require('net.db')
c = net.new('test:test@'..db.cfg.listen)
c.space.test:select{}
db.schema.role.grant('public', 'read', 'universe')
c.space.test:select{}
c:close()
c = net.new('test:test@'..db.cfg.listen)
c.space.test:select{}
db.schema.role.revoke('public', 'read', 'universe')
c.space.test:select{}
db.session.su('test')
test:select{}
db.session.su('admin')
c:close()
db.schema.user.drop('test')
test:drop()
--
-- gh-508 - wrong check for universal access of setuid functions
--
-- notice that guest can execute stuff, but can't read space _func
db.schema.user.grant('guest', 'execute', 'universe')
function f1() return db.space._func:get(1)[4] end
function f2() return db.space._func:get(2)[4] end
db.schema.func.create('f1')
db.schema.func.create('f2',{setuid=true})
c = net.new(db.cfg.listen)
-- should return access denied
c:call('f1')
-- should work (used to return access denied, because was not setuid
c:call('f2')
c:close()
db.schema.user.revoke('guest', 'execute', 'universe')
db.schema.func.drop('f1')
db.schema.func.drop('f2')
