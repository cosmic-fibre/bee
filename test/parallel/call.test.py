import sys
import uuid
import random

login = 'u'+str(uuid.uuid4())[0:8]
passw = 'p'+str(uuid.uuid4())[0:8]

sys.stdout.push_filter('db.schema.user.create.*', 'db.schema.user.create()')
sys.stdout.push_filter('db.schema.user.grant.*', 'db.schema.user.grant()')

admin("db.schema.user.create('%s', { password = '%s' })" % (login, passw))
admin("db.schema.user.grant('%s', 'execute,read,write', 'universe')" % (login))
sql.authenticate(login, passw)
admin("function f1() return 'testing', 1, false, -1, 1.123, 1e123, nil end")
admin("f1()")
sql("call f1()")
admin("f1=nil")
sql("call f1()")
admin("function f1() return f1 end")
sql("call f1()")

# A test case for https://github.com/bee/bee/issues/44
# IPROTO required!
sql("call dostring('db.error(33333, \"Hey!\")')")

print """
# A test case for Bug#103491
# server CALL processing bug with name path longer than two
# https://bugs.launchpad.net/bee/+bug/1034912
"""
admin("f = function() return 'OK' end")
admin("test = {}")
admin("test.f = f")
admin("test.test = {}")
admin("test.test.f = f")
sql("call f()")
sql("call test.f()")
sql("call test.test.f()")

print """
# Test for Bug #955226
# Lua Numbers are passed back wrongly as strings
#
"""
admin("function foo() return 1, 2, '1', '2' end")
sql("call foo()")

#
# check how well we can return tables
#
admin("function f1(...) return {...} end")
admin("function f2(...) return f1({...}) end")
sql("call f1('test_', 'test_')")
sql("call f2('test_', 'test_')")
sql("call f1()")
sql("call f2()")
#
# check multi-tuple return
#
admin("function f3() return {{'hello'}, {'world'}} end")
sql("call f3()")
admin("function f3() return {'hello', {'world'}} end")
sql("call f3()")
admin("function f3() return 'hello', {{'world'}, {'canada'}} end")
sql("call f3()")
admin("function f3() return {}, '123', {{}, {}} end")
sql("call f3()")
admin("function f3() return { {{'hello'}} } end")
sql("call f3()")
admin("function f3() return { db.tuple.new('hello'), {'world'} } end")
sql("call f3()")
admin("function f3() return { {'world'}, db.tuple.new('hello') } end")
sql("call f3()")

sql("call f1('jason')")
sql("call f1('jason', 1, 'test', 2, 'stewart')")

admin("space = db.schema.create_space('tweedledum', { id = 0 })")
admin("space:create_index('primary', { type = 'hash' })")

admin("function myreplace(...) return space:replace{...} end")
admin("function myinsert(...) return space:insert{...} end")
sql("insert into t0 values (1, 'test db delete')")
sql("call space:delete(1)")
sql("insert into t0 values (1, 'test db delete')")
sql("call space:delete(1)")
sql("call space:delete(1)")
sql("insert into t0 values (2, 'test db delete')")
sql("call space:delete(1)")
sql("call space:delete(2)")
sql("call space:delete(2)")
admin("space:delete{2}")
sql("insert into t0 values (2, 'test db delete')")
sql("call space:get(2)")
admin("space:delete{2}")
sql("call space:get(2)")
sql("insert into t0 values (2, 'test db.select()')")
sql("call space:get(2)")
sql("call space:select(2)")
admin("space:get{2}")
admin("space:select{2}")
admin("space:get{1}")
admin("space:select{1}")
sql("call myreplace(2, 'hello', 'world')")
sql("call myreplace(2, 'goodbye', 'universe')")
sql("call space:get(2)")
sql("call space:select(2)")
admin("space:get{2}")
admin("space:select{2}")
sql("call myreplace(2)")
sql("call space:get(2)")
sql("call space:select(2)")
sql("call space:delete(2)")
sql("call space:delete(2)")
sql("call myinsert(3, 'old', 2)")
# test that insert produces a duplicate key error
sql("call myinsert(3, 'old', 2)")
admin("space:update({3}, {{'=', 1, 4}, {'=', 2, 'new'}})")
sql("call space:get(4)")
sql("call space:select(4)")
admin("space:update({4}, {{'+', 3, 1}})")
admin("space:update({4}, {{'-', 3, 1}})")
sql("call space:get(4)")
sql("call space:select(4)")
admin("function field_x(key, field_index) return space:get(key)[field_index] end")
sql("call field_x(4, 1)")
sql("call field_x(4, 2)")
sql("call space:delete(4)")

admin("space:drop()")
sys.stdout.push_filter('db.schema.user.drop(.*)', 'db.schema.user.drop()')
admin("db.schema.user.drop('%s')" % login)
