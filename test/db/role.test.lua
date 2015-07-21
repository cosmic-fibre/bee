db.schema.role.create('iddqd')
db.schema.role.create('iddqd')
db.schema.role.drop('iddqd')
db.schema.role.drop('iddqd')
db.schema.role.create('iddqd')
-- impossible to su to a role
db.session.su('iddqd')
-- test granting privilege to a role
db.schema.role.grant('iddqd', 'execute', 'universe')
db.schema.role.info('iddqd')
db.schema.role.revoke('iddqd', 'execute', 'universe')
db.schema.role.info('iddqd')
-- test granting a role to a user
db.schema.user.create('tester')
db.schema.user.info('tester')
db.schema.user.grant('tester', 'execute', 'role', 'iddqd')
db.schema.user.info('tester')
-- test granting user to a user
db.schema.user.grant('tester', 'execute', 'role', 'tester')
-- test granting a non-execute grant on a role - error
db.schema.user.grant('tester', 'write', 'role', 'iddqd')
db.schema.user.grant('tester', 'read', 'role', 'iddqd')
-- test granting role to a role
db.schema.role.grant('iddqd', 'execute', 'role', 'iddqd')
db.schema.role.grant('iddqd', 'iddqd')
db.schema.role.revoke('iddqd', 'iddqd')
db.schema.user.grant('tester', 'iddqd')
db.schema.user.revoke('tester', 'iddqd')
db.schema.role.drop('iddqd')
db.schema.user.revoke('tester', 'no-such-role')
db.schema.user.grant('tester', 'no-such-role')
db.schema.user.drop('tester')
-- check for loops in role grants
db.schema.role.create('a')
db.schema.role.create('b')
db.schema.role.create('c')
db.schema.role.create('d')
db.schema.role.grant('b', 'a')
db.schema.role.grant('c', 'a')
db.schema.role.grant('d', 'b')
db.schema.role.grant('d', 'c')

--check user restrictions
db.schema.user.grant('a', 'd')
db.schema.user.revoke('a', 'd')
db.schema.user.drop('a')

db.schema.role.grant('a', 'd')
db.schema.role.drop('d')
db.schema.role.drop('b')
db.schema.role.drop('c')
db.schema.role.drop('a')
-- check that when dropping a role, it's first revoked
-- from whoever it is granted
db.schema.role.create('a')
db.schema.role.create('b')
db.schema.role.grant('b', 'a')
db.schema.role.drop('a')
db.schema.role.info('b')
db.schema.role.drop('b')
-- check a grant received via a role
db.schema.user.create('test')
db.schema.user.create('grantee')
db.schema.role.create('liaison')

--check role restrictions
db.schema.role.grant('test', 'liaison')
db.schema.role.revoke('test', 'liaison')
db.schema.role.drop('test')

db.schema.user.grant('grantee', 'liaison')
db.schema.user.grant('test', 'read,write', 'universe')
db.session.su('test')
s = db.schema.space.create('test')
_ = s:create_index('i1')
db.schema.role.grant('liaison', 'read,write', 'space', 'test')
db.session.su('grantee')
db.space.test:insert{1}
db.space.test:select{1}
db.session.su('test')
db.schema.role.revoke('liaison', 'read,write', 'space', 'test')
db.session.su('grantee')
db.space.test:insert{1}
db.space.test:select{1}
db.session.su('admin')
db.schema.user.drop('test')
db.schema.user.drop('grantee')
db.schema.role.drop('liaison')


--
-- Test how privileges are propagated through a complex role graph.
-- Here's the graph:
--
-- role1 ->- role2 -->- role4 -->- role6 ->- user1
--                \               /     \
--                 \->- role5 ->-/       \->- role9 ->- role10 ->- user
--                     /     \               /
--           role3 ->-/       \->- role7 ->-/
--
-- Privilege checks verify that grants/revokes are propagated correctly
-- from the role1 to role10.
--
db.schema.user.create("user")
db.schema.role.create("role1")
db.schema.role.create("role2")
db.schema.role.create("role3")
db.schema.role.create("role4")
db.schema.role.create("role5")
db.schema.role.create("role6")
db.schema.role.create("role7")
db.schema.user.create("user1")
db.schema.role.create("role9")
db.schema.role.create("role10")

db.schema.role.grant("role2", "role1")
db.schema.role.grant("role4", "role2")
db.schema.role.grant("role5", "role2")
db.schema.role.grant("role5", "role3")
db.schema.role.grant("role6", "role4")
db.schema.role.grant("role6", "role5")
db.schema.role.grant("role7", "role5")
db.schema.user.grant("user1", "role6")
db.schema.role.grant("role9", "role6")
db.schema.role.grant("role9", "role7")
db.schema.role.grant("role10", "role9")
db.schema.user.grant("user", "role10")

-- try to create a cycle
db.schema.role.grant("role2", "role10")

--
-- test grant propagation
--
db.schema.role.grant("role1", "read", "universe")
db.session.su("user")
db.space._space.index.name:get{"_space"}[3]
db.session.su("admin")
db.schema.role.revoke("role1", "read", "universe")
db.session.su("user")
db.space._space.index.name:get{"_space"}[3]
db.session.su("admin")

--
-- space-level privileges
--
db.schema.role.grant("role1", "read", "space", "_index")
db.session.su("user")
db.space._space.index.name:get{"_space"}[3]
db.space._index:get{288, 0}[3]
db.session.su("admin")
db.schema.role.revoke("role1", "read", "space", "_index")
db.session.su("user")
db.space._space.index.name:get{"_space"}[3]
db.space._index:get{288, 0}[3]
db.session.su("admin")

--
-- grant to a non-leaf branch
--
db.schema.role.grant("role5", "read", "space", "_index")
db.session.su("user")
db.space._space.index.name:get{"_space"}[3]
db.space._index:get{288, 0}[3]
db.session.su("admin")
db.schema.role.revoke("role5", "read", "space", "_index")
db.session.su("user")
db.space._space.index.name:get{"_space"}[3]
db.space._index:get{288, 0}[3]
db.session.su("admin")

--
-- grant via two branches
--
db.schema.role.grant("role3", "read", "space", "_index")
db.schema.role.grant("role4", "read", "space", "_index")
db.schema.role.grant("role9", "read", "space", "_index")

db.session.su("user")
db.space._index:get{288, 0}[3]
db.session.su("user1")
db.space._index:get{288, 0}[3]

db.session.su("admin")
db.schema.role.revoke("role3", "read", "space", "_index")

db.session.su("user")
db.space._index:get{288, 0}[3]
db.session.su("user1")
db.space._index:get{288, 0}[3]

db.session.su("admin")
db.schema.role.revoke("role4", "read", "space", "_index")

db.session.su("user")
db.space._index:get{288, 0}[3]
db.session.su("user1")
db.space._index:get{288, 0}[3]

db.session.su("admin")
db.schema.role.revoke("role9", "read", "space", "_index")

db.session.su("user")
db.space._index:get{288, 0}[3]
db.session.su("user1")
db.space._index:get{288, 0}[3]
db.session.su("admin")

--
-- check diamond-shaped grant graph
--

db.schema.role.grant("role5", "read", "space", "_space")

db.session.su("user")
db.space._space.index.name:get{"_space"}[3]
db.session.su("user1")
db.space._space.index.name:get{"_space"}[3]

db.session.su("admin")
db.schema.role.revoke("role5", "read", "space", "_space")

db.session.su("user")
db.space._space.index.name:get{"_space"}[3]
db.session.su("user1")
db.space._space.index.name:get{"_space"}[3]

db.session.su("admin")

db.schema.user.drop("user")
db.schema.user.drop("user1")
db.schema.role.drop("role1")
db.schema.role.drop("role2")
db.schema.role.drop("role3")
db.schema.role.drop("role4")
db.schema.role.drop("role5")
db.schema.role.drop("role6")
db.schema.role.drop("role7")
db.schema.role.drop("role9")
db.schema.role.drop("role10")


--
-- only the creator of the role can grant it (or a superuser)
-- There is no grant option.
-- the same applies for privileges
--
db.schema.user.create('user')
db.schema.user.create('grantee')

db.schema.user.grant('user', 'read,write,execute', 'universe')
db.session.su('user')
db.schema.role.create('role')
db.session.su('admin')
db.schema.user.grant('grantee', 'role')
db.schema.user.revoke('grantee', 'role')
db.schema.user.create('john')
db.session.su('john')
-- error
db.schema.user.grant('grantee', 'role')
--
db.session.su('admin')
_ = db.schema.space.create('test')
db.schema.user.grant('john', 'read,write,execute', 'universe')
db.session.su('john')
db.schema.user.grant('grantee', 'role')
db.schema.user.grant('grantee', 'read', 'space', 'test')
--
-- granting 'public' is however an exception - everyone
-- can grant 'public' role, it's implicitly granted with
-- a grant option.
--
db.schema.user.grant('grantee', 'public')
--
-- revoking role 'public' is another deal - only the
-- superuser can do that, and even that would be useless,
-- since one can still re-grant it back to oneself.
--
db.schema.user.revoke('grantee', 'public')

db.session.su('admin')
db.schema.user.drop('john')
db.schema.user.drop('user')
db.schema.user.drop('grantee')
db.schema.role.drop('role')
db.space.test:drop()

--
-- grant a privilege through a role, but
-- the user has another privilege either granted
-- natively (one case) or via another role.
-- Check that privileges actually OR, but
-- not replace each other.
--
_ = db.schema.space.create('test')
_ = db.space.test:create_index('primary')
db.schema.user.create('john')
db.schema.user.grant('john', 'read', 'space', 'test')
db.session.su('john')
db.space.test:select{}
db.space.test:insert{1}
db.session.su('admin')
db.schema.role.grant('public', 'write', 'space', 'test')
db.session.su('john')
db.space.test:select{}
db.space.test:insert{2}
db.session.su('admin')
db.schema.role.revoke('public', 'write', 'space', 'test')
db.session.su('john')
db.space.test:select{}
db.space.test:insert{1}
db.session.su('admin')
db.space.test:drop()
db.schema.user.drop('john')

-- test ER_GRANT
db.space._priv:insert{1, 0, 'universe', 0, 0}
-- role.exists()
--
-- true if the role is present
db.schema.role.exists('public')
-- for if there is no such role
db.schema.role.exists('nosuchrole')
-- false for users
db.schema.role.exists('guest')
-- false for users
db.schema.role.exists('admin')
-- role id is ok
db.schema.role.exists(3)
-- user id 
db.schema.role.exists(0)
db.schema.role.create('public', { if_not_exists = true})
db.schema.user.create('admin', { if_not_exists = true})
db.schema.user.create('guest', { if_not_exists = true})
db.schema.user.create('test', { if_not_exists = true})
db.schema.user.create('test', { if_not_exists = true})
db.schema.role.drop('test', { if_not_exists = true})
db.schema.role.drop('test', { if_exists = true})
db.schema.role.create('test', { if_not_exists = true})
db.schema.role.create('test', { if_not_exists = true})
db.schema.user.drop('test', { if_not_exists = true})
-- gh-664 roles: accepting bad syntax for create
db.schema.role.create('role', 'role')
db.schema.role.drop('role', 'role')
db.schema.user.drop('test', { if_exists = true})
