#!/usr/bin/env bee

package.path = "lua/?.lua;"..package.path

local tap = require('tap')
local common = require('serializer_test')

local function is_map(s)
    return s:match("---[\n ]%w+%:") or s:match("---[\n ]{%w+%:")
end

local function is_array(s)
    return s:match("---[\n ]%[") or s:match("---[\n ]- ");
end

local function test_compact(test, s)
    test:plan(9)

    local ss = s.new()
    ss.cfg{encode_load_metatables = true, decode_save_metatables = true}

    test:is(ss.encode({10, 15, 20}), "---\n- 10\n- 15\n- 20\n...\n",
        "block array")
    test:is(ss.encode(setmetatable({10, 15, 20}, { __serialize="array"})),
        "---\n- 10\n- 15\n- 20\n...\n", "block array")
    test:is(ss.encode(setmetatable({10, 15, 20}, { __serialize="sequence"})),
        "---\n- 10\n- 15\n- 20\n...\n", "block array")
    test:is(ss.encode({setmetatable({10, 15, 20}, { __serialize="seq"})}),
        "---\n- [10, 15, 20]\n...\n", "flow array")
    test:is(getmetatable(ss.decode(ss.encode({10, 15, 20}))).__serialize, "seq",
        "decoded __serialize is seq")

    test:is(ss.encode({k = 'v'}), "---\nk: v\n...\n", "block map")
    test:is(ss.encode(setmetatable({k = 'v'}, { __serialize="mapping"})),
        "---\nk: v\n...\n", "block map")
    test:is(ss.encode({setmetatable({k = 'v'}, { __serialize="map"})}),
        "---\n- {k: v}\n...\n", "flow map")
    test:is(getmetatable(ss.decode(ss.encode({k = 'v'}))).__serialize, "map",
        "decoded __serialize is map")

    ss = nil
end

local function test_output(test, s)
    test:plan(6)
    test:is(s.encode({true}), '---\n- true\n...\n', "encode for true")
    test:is(s.decode("---\nyes\n..."), true, "decode for 'yes'")
    test:is(s.encode({false}), '---\n- false\n...\n', "encode for false")
    test:is(s.decode("---\nno\n..."), false, "decode for 'no'")
    test:is(s.encode({s.NULL}), '---\n- null\n...\n', "encode for nil")
    test:is(s.decode("---\n~\n..."), s.NULL, "decode for ~")
end

tap.test("yaml", function(test)
    local serializer = require('yaml')
    test:plan(10)
    test:test("unsigned", common.test_unsigned, serializer)
    test:test("signed", common.test_signed, serializer)
    test:test("double", common.test_double, serializer)
    test:test("boolean", common.test_boolean, serializer)
    test:test("string", common.test_string, serializer)
    test:test("nil", common.test_nil, serializer)
    test:test("table", common.test_table, serializer, is_array, is_map)
    test:test("ucdata", common.test_ucdata, serializer)
    test:test("compact", test_compact, serializer)
    test:test("output", test_output, serializer)
end)
