session = db.session
session.su('admin')
--
-- Check max function limit
--
--# setopt delimiter ';'
function func_limit()
    local i = 1
    while true do
        db.schema.func.create('func'..i)
        i = i + 1
    end
    return i
end;
function drop_limit_func()
    local i = 1
    while true do
        db.schema.func.drop('func'..i)
        i = i + 1
    end
end;
func_limit();
drop_limit_func();
db.schema.user.create('testuser');
db.schema.user.grant('testuser', 'read, write, execute', 'universe');
session.su('testuser');
func_limit();
drop_limit_func();
session.su('admin')
db.schema.user.drop('testuser');
--# setopt delimiter ''
