#!/usr/bin/env bee

package.path = "lua/?.lua;"..package.path

local tap = require('tap')
local common = require('serializer_test')

local function is_map(s)
    local b = string.byte(string.sub(s, 1, 1))
    return b >= 0x80 and b <= 0x8f or b == 0xde or b == 0xdf
end

local function is_array(s)
    local b = string.byte(string.sub(s, 1, 1))
    return b >= 0x90 and b <= 0x9f or b == 0xdc or b == 0xdd
end

local function test_offsets(test, s)
    test:plan(6)
    local arr1 = {1, 2, 3}
    local arr2 = {4, 5, 6}
    local dump = s.encode(arr1)..s.encode(arr2)
    test:is(dump:len(), 8, "length of part1 + part2")

    local a
    local offset = 1
    a, offset = s.decode(dump, offset)
    test:is_deeply(a, arr1, "decoded part1")
    test:is(offset, 5, "offset of part2")

    a, offset = s.decode(dump, offset)
    test:is_deeply(a, arr2, "decoded part2")
    test:is(offset, 9, "offset of end")

    test:ok(not pcall(s.decode, dump, offset), "invalid offset")
end

local function test_other(test, s)
    test:plan(3)
    local buf = string.char(0x93, 0x6e, 0xcb, 0x42, 0x2b, 0xed, 0x30, 0x47,
        0x6f, 0xff, 0xff, 0xac, 0x77, 0x6b, 0x61, 0x71, 0x66, 0x7a, 0x73,
        0x7a, 0x75, 0x71, 0x71, 0x78)
    local num = s.decode(buf)[2]
    test:ok(num < 59971740600 and num > 59971740599, "gh-633 double decode")

    -- gh-596: msgpack and msgpackffi have different behaviour
    local arr = {1, 2, 3}
    local map = {k1 = 'v1', k2 = 'v2', k3 = 'v3'}
    test:is(getmetatable(s.decode(s.encode(arr))).__serialize, "seq",
        "array save __serialize")
    test:is(getmetatable(s.decode(s.encode(map))).__serialize, "map",
        "map save __serialize")
end

tap.test("msgpackffi", function(test)
    local serializer = require('msgpackffi')
    test:plan(9)
    test:test("unsigned", common.test_unsigned, serializer)
    test:test("signed", common.test_signed, serializer)
    test:test("double", common.test_double, serializer)
    test:test("boolean", common.test_boolean, serializer)
    test:test("string", common.test_string, serializer)
    test:test("nil", common.test_nil, serializer)
    test:test("table", common.test_table, serializer, is_array, is_map)
    -- udata/cdata hooks are not implemented
    --test:test("ucdata", common.test_ucdata, serializer)
    test:test("offsets", test_offsets, serializer)
    test:test("other", test_other, serializer)
end)
