const char tap_lua[] =
"--- tap.lua internal file\n"
"---\n"
"--- The Test Anything Protocol vesion 13 producer\n"
"---\n"
"\n"
"-- yaml formatter must be able to encode any Lua variable\n"
"local yaml = require('yaml').new()\n"
"yaml.cfg{\n"
"    encode_invalid_numbers = true;\n"
"    encode_load_metatables = true;\n"
"    encode_use_tostring    = true;\n"
"    encode_invalid_as_nil  = true;\n"
"}\n"
"\n"
"local ffi = require('ffi') -- for iscdata\n"
"\n"
"local function traceback(level)\n"
"    local trace = {}\n"
"    level = level or 3\n"
"    while true do\n"
"        local info = debug.getinfo(level, \"nSl\")\n"
"        if not info then break end\n"
"        local frame = {\n"
"            source = info.source;\n"
"            src = info.short_src;\n"
"            line = info.linedefined or 0;\n"
"            what = info.what;\n"
"            name = info.name;\n"
"            namewhat = info.namewhat;\n"
"            filename = info.source:sub(1, 1) == \"@\" and info.source:sub(2) or \n"
"                'eval'\n"
"        }\n"
"        table.insert(trace, frame)\n"
"        level = level + 1\n"
"    end\n"
"    return trace\n"
"end\n"
"\n"
"local function diag(test, fmt, ...)\n"
"    io.write(string.rep(' ', 4 * test.level), \"# \", string.format(fmt, ...),\n"
"        \"\\n\")\n"
"end\n"
"\n"
"local function ok(test, cond, message, extra)\n"
"    test.total = test.total + 1\n"
"    io.write(string.rep(' ', 4 * test.level))\n"
"    if cond then\n"
"        io.write(string.format(\"ok - %s\\n\", message))\n"
"        return true\n"
"    end\n"
"\n"
"    test.failed = test.failed + 1\n"
"    io.write(string.format(\"not ok - %s\\n\", message))\n"
"    extra = extra or {}\n"
"    if test.trace then\n"
"        local frame = debug.getinfo(3, \"Sl\")\n"
"        extra.trace = traceback()\n"
"        extra.filename = extra.trace[#extra.trace].filename\n"
"        extra.line = extra.trace[#extra.trace].line\n"
"    end\n"
"    if next(extra) == nil then\n"
"        return false -- don't have extra information\n"
"    end\n"
"    -- print aligned yaml output\n"
"    for line in yaml.encode(extra):gmatch(\"[^\\n]+\") do\n"
"        io.write(string.rep(' ', 2 + 4 * test.level), line, \"\\n\")\n"
"    end\n"
"    return false\n"
"end\n"
"\n"
"local function fail(test, message, extra)\n"
"    return ok(test, false, message, extra)\n"
"end\n"
"\n"
"local function skip(test, message, extra)\n"
"    ok(test, true, message..\" # skip\", extra)\n"
"end\n"
"\n"
"\n"
"local nan = 0/0\n"
"\n"
"local function cmpdeeply(got, expected, extra)\n"
"    if type(expected) == \"number\" or type(got) == \"number\" then\n"
"        extra.got = got\n"
"        extra.expected = expected\n"
"        if got ~= got and expected ~= expected then\n"
"            return true -- nan\n"
"        end\n"
"        return got == expected\n"
"    end\n"
"\n"
"    if ffi.istype('bool', got) then got = (got == 1) end\n"
"    if ffi.istype('bool', expected) then expected = (expected == 1) end\n"
"    if got == nil and expected == nil then return true end\n"
"\n"
"    if type(got) ~= type(expected) then\n"
"        extra.got = type(got)\n"
"        extra.expected = type(expected)\n"
"        return false\n"
"    end\n"
"\n"
"    if type(got) ~= 'table' then\n"
"        extra.got = got\n"
"        extra.expected = expected\n"
"        return got == expected\n"
"    end\n"
"\n"
"    local path = extra.path or '/'\n"
"\n"
"    for i, v in pairs(got) do\n"
"        extra.path = path .. '/' .. i\n"
"        if not cmpdeeply(v, expected[i], extra) then\n"
"            return false\n"
"        end\n"
"    end\n"
"\n"
"    for i, v in pairs(expected) do\n"
"        extra.path = path .. '/' .. i\n"
"        if not cmpdeeply(got[i], v, extra) then\n"
"            return false\n"
"        end\n"
"    end\n"
"\n"
"    extra.path = path\n"
"\n"
"    return true\n"
"end\n"
"\n"
"local function like(test, got, pattern, message, extra)\n"
"    extra = extra or {}\n"
"    extra.got = got\n"
"    extra.expected = pattern\n"
"    return ok(test, string.match(tostring(got), pattern) ~= nil, message, extra)\n"
"end\n"
"\n"
"local function unlike(test, got, pattern, message, extra)\n"
"    extra = extra or {}\n"
"    extra.got = got\n"
"    extra.expected = pattern\n"
"    return ok(test, string.match(tostring(got), pattern) == nil, message, extra)\n"
"end\n"
"\n"
"local function is(test, got, expected, message, extra)\n"
"    extra = extra or {}\n"
"    extra.got = got\n"
"    extra.expected = expected\n"
"    return ok(test, got == expected, message, extra)\n"
"end\n"
"\n"
"local function isnt(test, got, unexpected, message, extra)\n"
"    extra = extra or {}\n"
"    extra.got = got\n"
"    extra.unexpected = unexpected\n"
"    return ok(test, got ~= unexpected, message, extra)\n"
"end\n"
"\n"
"\n"
"local function is_deeply(test, got, expected, message, extra)\n"
"    extra = extra or {}\n"
"    extra.got = got\n"
"    extra.expected = expected\n"
"    return ok(test, cmpdeeply(got, expected, extra), message, extra)\n"
"end\n"
"\n"
"local function isnil(test, v, message, extra)\n"
"    return is(test, not v and 'nil' or v, 'nil', message, extra)\n"
"end\n"
"\n"
"local function isnumber(test, v, message, extra)\n"
"    return is(test, type(v), 'number', message, extra)\n"
"end\n"
"\n"
"local function isstring(test, v, message, extra)\n"
"    return is(test, type(v), 'string', message, extra)\n"
"end\n"
"\n"
"local function istable(test, v, message, extra)\n"
"    return is(test, type(v), 'table', message, extra)\n"
"end\n"
"\n"
"local function isboolean(test, v, message, extra)\n"
"    return is(test, type(v), 'boolean', message, extra)\n"
"end\n"
"\n"
"local function isudata(test, v, utype, message, extra)\n"
"    extra = extra or {}\n"
"    extra.expected = 'userdata<'..utype..'>'\n"
"    if type(v) == 'userdata' then\n"
"        extra.got = 'userdata<'..getmetatable(v)..'>'\n"
"        return ok(test, getmetatable(v) == utype, message, extra)\n"
"    else\n"
"        extra.got = type(v)\n"
"        return fail(test, message, extra)\n"
"    end\n"
"end\n"
"\n"
"local function iscdata(test, v, ctype, message, extra)\n"
"    extra = extra or {}\n"
"    extra.expected = ffi.typeof(ctype)\n"
"    if type(v) == 'cdata' then\n"
"        extra.got = ffi.typeof(v)\n"
"        return ok(test, ffi.istype(ctype, v), message, extra)\n"
"    else\n"
"        extra.got = type(v)\n"
"        return fail(test, message, extra)\n"
"    end\n"
"end\n"
"\n"
"local test_mt\n"
"local function test(parent, name, fun, ...)\n"
"    local level = parent ~= nil and parent.level + 1 or 0\n"
"    local test = setmetatable({\n"
"        parent  = parent;\n"
"        name    = name;\n"
"        level   = level;\n"
"        total   = 0;\n"
"        failed  = 0;\n"
"        planned = 0;\n"
"        trace   = parent == nil and true or parent.trace;\n"
"    }, test_mt)\n"
"    if fun ~= nil then\n"
"        test:diag('%s', test.name)\n"
"        fun(test, ...)\n"
"        test:diag('%s: end', test.name)\n"
"        return test:check()\n"
"    else\n"
"        return test\n"
"    end\n"
"end\n"
"\n"
"local function plan(test, planned)\n"
"    test.planned = planned\n"
"    io.write(string.rep(' ', 4 * test.level), string.format(\"1..%d\\n\", planned))\n"
"end\n"
"\n"
"local function check(test)\n"
"    if test.checked then\n"
"        error('check called twice')\n"
"    end\n"
"    test.checked = true\n"
"    if test.planned ~= test.total then\n"
"        if test.parent ~= nil then\n"
"            ok(test.parent, false, \"bad plan\", { planned = test.planned;\n"
"                run = test.total})\n"
"        else\n"
"            diag(test, string.format(\"bad plan: planned %d run %d\",\n"
"                test.planned, test.total))\n"
"        end\n"
"    elseif test.failed > 0 then\n"
"        if test.parent ~= nil then\n"
"            ok(test.parent, false, \"failed subtests\", {\n"
"                failed = test.failed;\n"
"                planned = test.planned;\n"
"            })\n"
"            return false\n"
"        else\n"
"            diag(test, \"failed subtest: %d\", test.failed)\n"
"        end\n"
"    else\n"
"        if test.parent ~= nil then\n"
"            ok(test.parent, true, test.name)\n"
"        end\n"
"    end\n"
"    return true\n"
"end\n"
"\n"
"test_mt = {\n"
"    __index = {\n"
"        test      = test;\n"
"        plan      = plan;\n"
"        check     = check;\n"
"        diag      = diag;\n"
"        ok        = ok;\n"
"        fail      = fail;\n"
"        skip      = skip;\n"
"        is        = is;\n"
"        isnt      = isnt;\n"
"        isnil     = isnil;\n"
"        isnumber  = isnumber;\n"
"        isstring  = isstring;\n"
"        istable   = istable;\n"
"        isboolean = isboolean;\n"
"        isudata   = isudata;\n"
"        iscdata   = iscdata;\n"
"        is_deeply = is_deeply;\n"
"        like      = like;\n"
"        unlike    = unlike;\n"
"    }\n"
"}\n"
"\n"
"local function root_test(...)\n"
"    io.write('TAP version 13', '\\n')\n"
"    return test(nil, ...)\n"
"end\n"
"\n"
"return {\n"
"    test = root_test;\n"
"}\n"
""
;