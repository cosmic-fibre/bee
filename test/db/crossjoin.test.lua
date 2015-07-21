space = db.schema.space.create('tweedledum')
index = space:create_index('primary', { type = 'tree' })
--# setopt delimiter ';'
function crossjoin(space0, space1, limit)
  local result = {}
  for _,v0 in space0:pairs() do
    for _,v1 in space1:pairs() do
      if limit <= 0 then
        return result
      end
      newtuple = v0:totable()
      for _, v in v1:pairs() do table.insert(newtuple, v) end
      table.insert(result, newtuple)
      limit = limit - 1
    end
  end
  return result
end;
--# setopt delimiter ''
crossjoin(space, space, 0)
crossjoin(space, space, 10000)
space:insert{1}
crossjoin(space, space, 10000)
space:insert{2}
crossjoin(space, space, 10000)
space:insert{3, 'hello'}
crossjoin(space, space, 10000)
space:insert{4, 'world'}
space[0]:insert{5, 'hello world'}
crossjoin(space, space, 10000)
space:drop()
crossjoin = nil
