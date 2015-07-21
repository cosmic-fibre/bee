_schema = db.space[db.schema.SCHEMA_ID]
_space = db.space[db.schema.SPACE_ID]
_index = db.space[db.schema.INDEX_ID]
_user = db.space[db.schema.USER_ID]
_func = db.space[db.schema.FUNC_ID]
_priv = db.space[db.schema.PRIV_ID]
_cluster = db.space[db.schema.CLUSTER_ID]
-- destroy everything - save snapshot produces an empty snapshot now

-- space:truncate() doesn't work with disabled triggers on __index
local function truncate(space)
    local pk = space.index[0]
    while pk:len() > 0 do
        local state, t
        for state, t in pk:pairs() do
            local key = {}
            for _k2, parts in ipairs(pk.parts) do
                table.insert(key, t[parts.fieldno])
            end
            space:delete(key)
        end
    end
end

_space:run_triggers(false)
_index:run_triggers(false)
_user:run_triggers(false)
_func:run_triggers(false)
_priv:run_triggers(false)

truncate(_space)
truncate(_index)
truncate(_user)
truncate(_func)
truncate(_priv)
_schema:delete('version')
_schema:delete('max_id')
