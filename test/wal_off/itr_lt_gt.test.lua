-- test for https://github.com/bee/bee/issues/769

s = db.schema.create_space('test')
i = s:create_index('primary', { type = 'TREE', parts = {1, 'num', 2, 'num'} })
s:insert{0, 0} s:insert{2, 0}
for i=1,10000 do s:insert{1, i} end
test_itrs = {'EQ', 'REQ', 'GT', 'LT', 'GE', 'LE'}
test_res = {}
too_longs = {}

--# setopt delimiter ';'
function test_run_itr(itr, key)
    for i=1,50 do
        local gen, param, state = s.index.primary:pairs({key}, {iterator = itr})
        state, v =  gen(param, state)
        test_res[itr .. ' ' .. key] = v
    end
end;

function test_itr(itr, key)
    test_run_itr(itr, key)
end;

for _,itr in pairs(test_itrs) do
    local t = os.clock()
    for key = 0,2 do
        test_itr(itr, key)
    end
    local diff = os.clock() - t
    if  diff > 0.05 then
        table.insert(too_longs, 'Some of the iterators takes too long to position: '.. diff)
    end
end;
--# setopt delimiter ''

test_res
too_longs
s:drop()
test_itr = nil test_run_itr = nil test_itrs = nil s = nil
'done'
