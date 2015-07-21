
session = db.session
-- user id for a Lua session is admin - 1
session.uid()
-- extra arguments are ignored
session.uid(nil)
-- admin
session.user()
-- extra argumentes are ignored
session.user(nil)
-- password() is a function which returns base64(sha1(sha1(password))
-- a string to store in _user table
db.schema.user.password('test')
db.schema.user.password('test1')
-- admin can create any user
db.schema.user.create('test', { password = 'test' })
-- su() let's you change the user of the session
-- the user will be unabe to change back unless he/she
-- is granted access to 'su'
session.su('test')
-- you can't create spaces unless you have a write access on
-- system space _space
-- in future we may  introduce a separate privilege
db.schema.space.create('test')
-- su() goes through because called from admin
-- console, and it has no access checks
-- for functions
session.su('admin')
db.schema.user.grant('test', 'write', 'space', '_space')

--# setopt delimiter ';'
function usermax()
    local i = 1
    while true do
        db.schema.user.create('user'..i)
        i = i + 1
    end
end;
usermax();
function usermax()
    local i = 1
    while true do
        db.schema.user.drop('user'..i)
        i = i + 1
    end
end;
usermax();
--# setopt delimiter ''
db.schema.user.create('rich')
db.schema.user.grant('rich', 'read,write', 'universe')
session.su('rich')
uid = session.uid()
db.schema.func.create('dummy')
session.su('admin')
db.space['_user']:delete{uid}
db.schema.func.drop('dummy')
db.space['_user']:delete{uid}
db.schema.user.revoke('rich', 'read,write', 'universe')
db.schema.user.revoke('rich', 'public')
db.space['_user']:delete{uid}
db.schema.user.drop('test')

-- sudo
db.schema.user.create('tester')
-- admin -> user
session.user()
session.su('tester', function() return session.user() end)
session.user()

-- user -> admin
session.su('tester')
session.user()
session.su('admin', function() return session.user() end)
session.user()

-- drop current user
session.su('admin', function() return db.schema.user.drop('tester') end)
session.user()
session.su('admin')
session.user()

--------------------------------------------------------------------------------
-- #198: names like '' and 'x.y' and 5 and 'primary ' are legal
--------------------------------------------------------------------------------
-- invalid identifiers
db.schema.user.create('invalid.identifier')
db.schema.user.create('invalid identifier')
db.schema.user.create('user ')
db.schema.user.create('5')
db.schema.user.create(' ')

-- valid identifiers
db.schema.user.create('Петя_Иванов')
db.schema.user.drop('Петя_Иванов')

-- gh-300: misleading error message if a function does not exist
LISTEN = require('uri').parse(db.cfg.listen)
LISTEN ~= nil
c = (require 'net.db'):new(LISTEN.host, LISTEN.service)

c:call('nosuchfunction')
function nosuchfunction() end
c:call('nosuchfunction')
nosuchfunction = nil
c:call('nosuchfunction')
c:close()
-- Dropping a space recursively drops all grants - it's possible to 
-- restore from a snapshot
db.schema.user.create('testus')
s = db.schema.space.create('admin_space')
index = s:create_index('primary', {type = 'hash', parts = {1, 'NUM'}})
db.schema.user.grant('testus', 'write', 'space', 'admin_space')
s:drop()
db.snapshot()
--# stop server default
--# start server default
db.schema.user.drop('testus')
-- ------------------------------------------------------------
-- a test case for gh-289
-- db.schema.user.drop() with cascade doesn't work
-- ------------------------------------------------------------
session = db.session
db.schema.user.create('uniuser')
db.schema.user.grant('uniuser', 'read, write, execute', 'universe')
session.su('uniuser')
us = db.schema.space.create('uniuser_space')
session.su('admin')
db.schema.user.drop('uniuser')
-- ------------------------------------------------------------
-- A test case for gh-253
-- A user with universal grant has no access to drop oneself
-- ------------------------------------------------------------
-- This behaviour is expected, since an object may be destroyed
-- only by its creator at the moment
-- ------------------------------------------------------------
db.schema.user.create('grantor')
db.schema.user.grant('grantor', 'read, write, execute', 'universe')  
session.su('grantor')
db.schema.user.create('grantee')
db.schema.user.grant('grantee', 'read, write, execute', 'universe')  
session.su('grantee')
-- fails - can't suicide - ask the creator to kill you
db.schema.user.drop('grantee')
session.su('grantor')
db.schema.user.drop('grantee')
-- fails, can't kill oneself
db.schema.user.drop('grantor')
session.su('admin')
db.schema.user.drop('grantor')
-- ----------------------------------------------------------
-- A test case for gh-299
-- It appears to be too easy to read all fields in _user
-- table
-- guest can't read _user table, add a test case
-- ----------------------------------------------------------
session.su('guest')
db.space._user:select{0}
db.space._user:select{1}
session.su('admin')
-- ----------------------------------------------------------
-- A test case for gh-358 Change user does not work from lua
-- Correct the update syntax in schema.lua
-- ----------------------------------------------------------
db.schema.user.create('user1')
db.space._user.index.name:select{'user1'}
session.su('user1')
db.schema.user.passwd('new_password')
session.su('admin')
db.space._user.index.name:select{'user1'}
db.schema.user.passwd('user1', 'extra_new_password')
db.space._user.index.name:select{'user1'}
db.schema.user.passwd('invalid_user', 'some_password')
db.schema.user.passwd()
session.su('user1')
-- permission denied
db.schema.user.passwd('admin', 'xxx')
session.su('admin')
db.schema.user.drop('user1')
db.space._user.index.name:select{'user1'}
-- ----------------------------------------------------------
-- A test case for gh-421 Granting a privilege revokes an
-- existing grant
-- ----------------------------------------------------------
db.schema.user.create('user')
id = db.space._user.index.name:get{'user'}[1]
db.schema.user.grant('user', 'read,write', 'universe')
db.space._priv:select{id}
db.schema.user.grant('user', 'read', 'universe')
db.space._priv:select{id}
db.schema.user.revoke('user', 'write', 'universe')
db.space._priv:select{id}
db.schema.user.revoke('user', 'read', 'universe')
db.space._priv:select{id}
db.schema.user.grant('user', 'write', 'universe')
db.space._priv:select{id}
db.schema.user.grant('user', 'read', 'universe')
db.space._priv:select{id}
db.schema.user.drop('user')
db.space._priv:select{id}
-- -----------------------------------------------------------
-- Be a bit more rigorous in what is accepted in space _user
-- -----------------------------------------------------------
db.space._user:insert{10, 1, 'name'}
db.space._user:insert{10, 1, 'name', 'strange-object-type'}
db.space._user:insert{10, 1, 'name', 'user', 'password'}
db.space._user:insert{10, 1, 'name', 'role', 'password'}
session = nil
-- -----------------------------------------------------------
-- admin can't manage grants on not owned objects
-- -----------------------------------------------------------
db.schema.user.create('twostep')
db.schema.user.grant('twostep', 'read,write,execute', 'universe')
db.session.su('twostep')
twostep = db.schema.space.create('twostep')
index2 = twostep:create_index('primary')
db.schema.func.create('test')
db.session.su('admin')
db.schema.user.revoke('twostep', 'execute,read,write', 'universe')
db.schema.user.create('twostep_client')
db.schema.user.grant('twostep_client', 'execute', 'function', 'test')
db.schema.user.drop('twostep')
db.schema.user.drop('twostep_client')
-- the space is dropped when the user is dropped
-- 
-- db.schema.user.exists()
db.schema.user.exists('guest')
db.schema.user.exists(nil)
db.schema.user.exists(0)
db.schema.user.exists(1)
db.schema.user.exists(100500)
db.schema.user.exists('admin')
db.schema.user.exists('nosuchuser')
db.schema.user.exists{}
-- gh-671: db.schema.func.exists()
db.schema.func.exists('nosuchfunc')
db.schema.func.exists('guest')
db.schema.func.exists(1)
db.schema.func.exists(2)
db.schema.func.exists('db.schema.user.info')
db.schema.func.exists()
db.schema.func.exists(nil)
-- gh-665: user.exists() should nto be true for roles
db.schema.user.exists('public')
db.schema.role.exists('public')
db.schema.role.exists(nil)
-- test if_exists/if_not_exists in grant/revoke
db.schema.user.grant('guest', 'read,write,execute', 'universe')
db.schema.user.grant('guest', 'read,write,execute', 'universe')
db.schema.user.grant('guest', 'read,write,execute', 'universe', '', { if_not_exists = true })
db.schema.user.revoke('guest', 'read,write,execute', 'universe')
db.schema.user.revoke('guest', 'read,write,execute', 'universe')
db.schema.user.revoke('guest', 'read,write,execute', 'universe', '', { if_exists = true })
db.schema.func.create('dummy', { if_not_exists = true })
db.schema.func.create('dummy', { if_not_exists = true })
db.schema.func.drop('dummy')

-- gh-664 roles: accepting bad syntax for create
db.schema.user.create('user', 'blah')
db.schema.user.drop('user', 'blah')

-- gh-664 roles: accepting bad syntax for create
db.schema.func.create('func', 'blah')
db.schema.func.drop('blah', 'blah')
-- gh-758 attempt to set password for user guest
db.schema.user.passwd('guest', 'sesame')
