import bee

admin("function f() db.schema.space.create('test', { id = 0 }) end")
admin("db.schema.user.create('test', { password = 'test' })")
admin("db.schema.func.create('f')")
admin("db.schema.user.grant('test', 'Write', 'space', '_space')")
admin("db.schema.user.grant('test', 'Execute', 'function', 'f')")
sql.authenticate('test', 'test')
# call from sql to have the right owner
sql("call f()")
admin("index = db.space.test:create_index('primary', { type = 'hash' })")
admin("db.schema.func.drop('f')")
sql("ping")
# xxx: bug -- currently selects no rows
sql("select * from t0")
sql("insert into t0 values (1, 'I am a tuple')")
sql("select * from t0 where k0 = 1")
# currently there is no way to find out how many records
# a space contains 
sql("select * from t0 where k0 = 0")
sql("select * from t0 where k0 = 2")
server.restart()
sql("select * from t0 where k0 = 1")
admin("db.snapshot()")
sql("select * from t0 where k0 = 1")
server.restart()
sql("select * from t0 where k0 = 1")
sql("delete from t0 where k0 = 1")
sql("select * from t0 where k0 = 1")
# xxx: update comes through, returns 0 rows affected 
sql("update t0 set k1 = 'I am a new tuple' where k0=1")
# nothing is selected, since nothing was there
sql("select * from t0 where k0=1")
sql("insert into t0 values (1, 'I am a new tuple')")
sql("select * from t0 where k0 = 1")
sql("update t0 set k1 = 'I am the newest tuple' where k0=1")
sql("select * from t0 where k0 = 1")
# this is correct, can append field to tuple
sql("update t0 set k1 = 'Huh', k2 = 'I am a new field! I was added via append' where k0=1")
sql("select * from t0 where k0 = 1")
# this is illegal
sql("update t0 set k1 = 'Huh', k1000 = 'invalid field' where k0=1")
sql("select * from t0 where k0 = 1")
sql("replace into t0 values (1, 'I am a new tuple', 'stub')")
sql("update t0 set k1 = 'Huh', k2 = 'Oh-ho-ho' where k0=1")
sql("select * from t0 where k0 = 1")
# check empty strings
sql("update t0 set k1 = '', k2 = '' where k0=1")
sql("select * from t0 where k0 = 1")
# check type change 
sql("update t0 set k1 = 2, k2 = 3 where k0=1")
sql("select * from t0 where k0 = 1")
# check limits
sql("insert into t0 values (0)")
sql("select * from t0 where k0=0")
sql("insert into t0 values (4294967295)")
sql("select * from t0 where k0=4294967295")
# cleanup 
sql("delete from t0 where k0=0")
sql("delete from t0 where k0=4294967295")

print """#
# A test case for: http://bugs.launchpad.net/bugs/712456
# Verify that when trying to access a non-existing or
# very large space id, no crash occurs.
#
"""
sql("select * from t1 where k0 = 0")
sql("select * from t65537 where k0 = 0")
sql("select * from t4294967295 where k0 = 0")
admin("db.space[0]:drop()")
admin("db.schema.user.drop('test')")

print """#
# A test case for: http://bugs.launchpad.net/bugs/716683
# Admin console should not stall on unknown command.
"""
admin("show status", simple=True)
