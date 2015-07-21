--# stop server default
--# start server default

space = db.schema.space.create('tweedledum')
index = space:create_index('primary')

--# push filter 'listen: .*' to 'listen: <uri>'
help()
db.cfg
space:insert{1, 'tuple'}
db.snapshot()
space:delete{1}
--# clear filter

--# setopt delimiter ';'
function check_type(arg, typeof)
    return type(arg) == typeof
end;

function test_db_info()
    local tmp = db.info()
    local num = {'pid', 'snapshot_pid', 'uptime'}
    local str = {'version', 'status' }
    local failed = {}
    if check_type(tmp.server, 'table') == false then
        table.insert(failed1, 'db.info().server')
    else
        tmp.server = nil
    end
    if check_type(tmp.replication, 'table') == false then
        table.insert(failed1, 'db.info().replication')
    else
        tmp.replication = nil
    end
    for k, v in ipairs(num) do
        if check_type(tmp[v], 'number') == false then
            table.insert(failed, 'db.info().'..v)
        else
            tmp[v] = nil
        end
    end
    for k, v in ipairs(str) do
        if check_type(tmp[v], 'string') == false then
            table.insert(failed, 'db.info().'..v)
        else
            tmp[v] = nil
        end
    end
    if #tmp > 0 or #failed > 0 then
        return 'db.info() is not ok.', 'failed: ', failed, tmp
    else
        return 'db.info() is ok.'
    end
end;

function test_slab(tbl)
    local num = {'item_size', 'item_count', 'slab_size', 'slab_count', 'mem_used', 'mem_free'}
    local failed = {}
    for k, v in ipairs(num) do
        if check_type(tbl[v], 'number') == false then
            table.insert(failed, 'db.slab.info().<slab_size>.'..v)
        else
            tbl[v] = nil
        end
    end
    if #tbl > 0 or #failed > 0 then
        return false, failed
    else
        return true, {}
    end
end;

function test_db_slab_info()
    local tmp = db.slab.info()
    local cdata = {'arena_size', 'arena_used'}
    local failed = {}
    if type(tmp.slabs) == 'table' then
        for name, tbl in ipairs(tmp.slabs) do
            local bl, fld = test_slab(tbl)
            if bl == true then
                tmp[name] = nil
            else
                for k, v in ipairs(fld) do
                    table.append(failed, v)
                end
            end
        end
    else
        table.append(failed, 'db.slab.info().slabs is not ok')
    end
    if #tmp.slabs == 0 then
        tmp.slabs = nil
    end
    for k, v in ipairs(cdata) do
        if check_type(tmp[v], 'number') == false then
            table.insert(failed, 'db.slab.info().'..v)
        else
            tmp[v] = nil
        end
    end
    if #tmp > 0 or #failed > 0 then
        return "db.slab.info() is not ok", tmp, failed
    else
        return "db.slab.info() is ok"
    end
end;

function test_fiber(tbl)
    local num = {'fid', 'csw'}
    for k, v in ipairs(num) do
        if check_type(tmp[v], 'number') == false then
            table.insert(failed, "require('fiber').info().<fiber_name>."..v)
        else
            tmp[v] = nil
        end
    end
    if type(tbl.backtrace) == 'table' and #tbl.backtrace > 0 then
        tbl.backtrace = nil
    else
        table.append(failed, 'backtrace')
    end
    if #tbl > 0 or #failed > 0 then
        return false, failed
    else
        return true, {}
    end
end;

function test_db_fiber_info()
    local tmp = require('fiber').info()
    local failed = {}
    for name, tbl in ipairs(tmp) do
        local bl, fld = test_fiber(tbl)
        if bl == true then
            tmp[name] = nil
        else
            for k, v in ipairs(fld) do
                table.append(failed, v)
            end
        end
    end
    if #tmp > 0 or #failed > 0 then
        return "require('fiber').info is not ok. failed: ", tmp, failed
    else
        return "require('fiber').info() is ok"
    end
end;

test_db_info();
test_db_slab_info();
test_db_fiber_info();
space:drop();
