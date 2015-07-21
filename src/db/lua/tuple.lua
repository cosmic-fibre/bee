-- tuple.lua (internal file)

local ffi = require('ffi')
local yaml = require('yaml')
local msgpackffi = require('msgpackffi')
local fun = require('fun')
local internal = require('db.internal')

ffi.cdef([[
struct tuple
{
    uint32_t _version;
    uint16_t _refs;
    uint16_t _format_id;
    uint32_t _bsize;
    char data[0];
} __attribute__((packed));

void
tuple_unref(struct tuple *tuple);
uint32_t
tuple_field_count(const struct tuple *tuple);
const char *
tuple_field(const struct tuple *tuple, uint32_t i);

struct tuple_iterator {
    const struct tuple *tuple;
    const char *pos;
    int fieldno;
};

void
tuple_rewind(struct tuple_iterator *it, const struct tuple *tuple);

const char *
tuple_seek(struct tuple_iterator *it, uint32_t field_no);

const char *
tuple_next(struct tuple_iterator *it);

void
tuple_to_buf(struct tuple *tuple, char *buf);

struct tuple *
dbffi_tuple_update(struct tuple *tuple, const char *expr, const char *expr_end);
]])

local builtin = ffi.C

local const_struct_tuple_ref_t = ffi.typeof('const struct tuple&')

local tuple_gc = function(tuple)
    builtin.tuple_unref(tuple)
end

local tuple_bless = function(tuple)
    -- must never fail:
    return ffi.gc(ffi.cast(const_struct_tuple_ref_t, tuple), tuple_gc)
end

local tuple_iterator_t = ffi.typeof('struct tuple_iterator')

local function tuple_iterator_next(it, tuple, pos)
    if pos == nil then
        pos = 0
    elseif type(pos) ~= "number" then
         error("error: invalid key to 'next'")
    end
    local field
    if it.tuple == tuple and it.fieldno == pos then
        -- Sequential iteration
        field = builtin.tuple_next(it)
    else
        -- Seek
        builtin.tuple_rewind(it, tuple)
        field = builtin.tuple_seek(it, pos);
    end
    if field == nil then
        if #tuple == pos then
            -- No more fields, stop iteration
            return nil
        else
            -- Invalid pos
            error("error: invalid key to 'next'")
        end
    end
    -- () used to shrink the return stack to one value
    return it.fieldno, (msgpackffi.decode_unchecked(field))
end;

-- precreated iterator for tuple_next
local next_it = ffi.new(tuple_iterator_t)

-- See http://www.lua.org/manual/5.2/manual.html#pdf-next
local function tuple_next(tuple, pos)
    return tuple_iterator_next(next_it, tuple, pos);
end

-- See http://www.lua.org/manual/5.2/manual.html#pdf-ipairs
local function tuple_ipairs(tuple, pos)
    local it = ffi.new(tuple_iterator_t)
    return fun.wrap(it, tuple, pos)
end

local function tuple_totable(tuple, i, j)
    -- use a precreated iterator for tuple_next
    builtin.tuple_rewind(next_it, tuple)
    local field
    if i ~= nil then
        if i < 1 then
            error('tuple.totable: invalid second argument')
        end
        field = builtin.tuple_seek(next_it, i - 1)
    else
        i = 1
        field = builtin.tuple_next(next_it)
    end
    if j ~= nil then
        if j <= 0 then
            error('tuple.totable: invalid third argument')
        end
    else
        j = 4294967295
    end
    local ret = {}
    while field ~= nil and i <= j do
        local val = msgpackffi.decode_unchecked(field)
        table.insert(ret, val)
        i = i + 1
        field = builtin.tuple_next(next_it)
    end
    return setmetatable(ret, msgpackffi.array_mt)
end

local function tuple_unpack(tuple, i, j)
    return unpack(tuple_totable(tuple, i, j))
end

local function tuple_find(tuple, offset, val)
    if val == nil then
        val = offset
        offset = 0
    end
    local r = tuple:pairs(offset):index(val)
    return r ~= nil and offset + r or nil
end

local function tuple_findall(tuple, offset, val)
    if val == nil then
        val = offset
        offset = 0
    end
    return tuple:pairs(offset):indexes(val)
        :map(function(i) return offset + i end)
        :totable()
end

local function tuple_update(tuple, expr)
    if type(expr) ~= 'table' then
        error("Usage: tuple:update({ { op, field, arg}+ })")
    end
    local pexpr, pexpr_end = msgpackffi.encode_tuple(expr)
    local tuple = builtin.dbffi_tuple_update(tuple, pexpr, pexpr_end)
    if tuple == NULL then
        return db.error()
    end
    return tuple_bless(tuple)
end

-- Set encode hooks for msgpackffi
local function tuple_to_msgpack(buf, tuple)
    buf:reserve(tuple._bsize)
    builtin.tuple_to_buf(tuple, buf.p)
    buf.p = buf.p + tuple._bsize
end

msgpackffi.on_encode(ffi.typeof('const struct tuple &'), tuple_to_msgpack)


-- cfuncs table is set by C part

local methods = {
    ["next"]        = tuple_next;
    ["ipairs"]      = tuple_ipairs;
    ["pairs"]       = tuple_ipairs; -- just alias for ipairs()
    ["slice"]       = cfuncs.slice;
    ["transform"]   = cfuncs.transform;
    ["find"]        = tuple_find;
    ["findall"]     = tuple_findall;
    ["unpack"]      = tuple_unpack;
    ["totable"]     = tuple_totable;
    ["update"]      = tuple_update;
    ["bsize"]       = function(tuple)
        return tonumber(tuple._bsize)
    end;
    ["__serialize"] = tuple_totable; -- encode hook for msgpack/yaml/json
}

local tuple_field = function(tuple, field_n)
    local field = builtin.tuple_field(tuple, field_n - 1)
    if field == nil then
        return nil
    end
    -- Use () to shrink stack to the first return value
    return (msgpackffi.decode_unchecked(field))
end


ffi.metatype('struct tuple', {
    __len = function(tuple)
        return builtin.tuple_field_count(tuple)
    end;
    __tostring = function(tuple)
        -- Unpack tuple, call yaml.encode, remove yaml header and footer
        -- 5 = '---\n\n' (header), -6 = '\n...\n' (footer)
        return yaml.encode(methods.totable(tuple)):sub(5, -6)
    end;
    __index = function(tuple, key)
        if type(key) == "number" then
            return tuple_field(tuple, key)
        end
        return methods[key]
    end;
    __eq = function(tuple_a, tuple_b)
        -- Two tuple are considered equal if they have same memory address
        return ffi.cast('void *', tuple_a) == ffi.cast('void *', tuple_b);
    end;
    __pairs = tuple_ipairs;  -- Lua 5.2 compatibility
    __ipairs = tuple_ipairs; -- Lua 5.2 compatibility
})

ffi.metatype(tuple_iterator_t, {
    __call = tuple_iterator_next;
    __tostring = function(it) return "<tuple iterator>" end;
})

-- Remove the global variable
cfuncs = nil

-- internal api for db.select and iterators
db.tuple.bless = tuple_bless
