--# setopt delimiter ';'
-- empty transaction - ok
db.begin() db.commit();
-- double begin
db.begin() db.begin();
-- no active transaction since exception rolled it back
db.commit();
-- double commit - implicit start of transaction
db.begin() db.commit() db.commit();
-- commit if not started - implicit start of transaction
db.commit();
-- rollback if not started - ok
db.rollback()
-- double rollback - ok
db.begin() db.rollback() db.rollback();
-- rollback of an empty trans - ends transaction
db.begin() db.rollback();
-- no current transaction - implicit begin
db.commit();
fiber = require('fiber');
function sloppy()
    db.begin()
end;
f = fiber.create(sloppy);
-- when the sloppy fiber ends, its session has an active transction
-- ensure it's rolled back automatically
f:status();
fiber.sleep(0);
fiber.sleep(0);
f:status();
-- transactions and system spaces
db.begin() db.schema.space.create('test');
db.rollback();
db.begin() db.schema.func.create('test');
db.rollback();
db.begin() db.schema.user.create('test');
db.rollback();
db.begin() db.schema.user.grant('guest', 'read', 'space', '_priv');
db.rollback();
db.begin() db.space._schema:insert{'test'};
db.rollback();
db.begin() db.space._cluster:insert{123456789, 'abc'};
db.rollback();
s = db.schema.space.create('test');
db.begin() index = s:create_index('primary');
db.rollback();
index = s:create_index('primary');
function multi()
    db.begin()
    s:auto_increment{'first row'}
    s:auto_increment{'second row'}
    t = s:select{}
    db.commit()
end;
multi();
t;
s:select{};
s:truncate();
function multi()
    db.begin()
    s:auto_increment{'first row'}
    s:auto_increment{'second row'}
    t = s:select{}
    db.rollback()
end;
multi();
t;
s:select{};
function multi()
    db.begin()
    s:insert{1, 'first row'}
    pcall(s.insert, s, {1, 'duplicate'})
    t = s:select{}
    db.commit()
end;
multi();
t;
s:select{};
s:truncate();
--
-- Test that fiber yield causes a transaction rollback
-- but only if the transaction has changed any data
--
-- Test admin console
db.begin();
-- should be ok - active transaction, and we don't
-- know, maybe it will use sham engine, which
-- may support yield() in the future, so we don't roll
-- back a transction with no statements.
db.commit();
db.begin() s:insert{1, 'Must be rolled back'};
-- nothing to commit because of yield
db.commit();
-- nothing - the transaction was rolled back
s:select{}
-- Test background fiber
--
function sloppy()
    db.begin()
    s:insert{1, 'From background fiber'}
end;
f = fiber.create(sloppy);
while f:status() == 'running' do
    fiber.sleep(0)
end;
-- When the sloppy fiber ends, its session has an active transction
-- It's rolled back automatically
s:select{};
function sloppy()
    db.begin()
    s:insert{1, 'From background fiber'}
    fiber.sleep(0)
    pcall(db.commit)
    t = s:select{}
end;
f = fiber.create(sloppy);
while f:status() == 'running' do
    fiber.sleep(0)
end;
t;
s:select{};
s:drop();
--# setopt delimiter ''
test = db.schema.space.create('test')
tindex = test:create_index('primary')
db.begin() test:insert{1} db.rollback()
test:select{1}
db.begin() test:insert{1} db.commit()
test:select{1}
--
-- gh-793 db.rollback() is not invoked after CALL
--
function test() db.begin() end
db.schema.func.create('test')
db.schema.user.grant('guest', 'execute', 'function', 'test')
cn = require('net.db').new(db.cfg.listen)
cn:call('test') -- first CALL starts transaction
cn:call('test') -- iproto reuses fiber on the second call
cn = nil
db.schema.func.drop('test')

--
-- Test statement-level rollback
--
db.space.test:truncate()
function insert(a) db.space.test:insert(a) end
--# setopt delimiter ';'
function dup_key()
    db.begin()
    db.space.test:insert{1}
    status, _ = pcall(insert, {1})
    if not status then
        if db.error.last().code ~= db.error.TUPLE_FOUND then
            db.error.raise()
        end
        db.space.test:insert{2}
    end
    db.commit()
end;
--# setopt delimiter ''
dup_key()
db.space.test:select{}
-- cleanup
--
db.space.test:drop()
