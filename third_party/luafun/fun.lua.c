const char fun_lua[] =
"---\n"
"--- Lua Fun - a high-performance functional programming library for LuaJIT\n"
"---\n"
"--- Copyright (c) 2013-2014 Roman Tsisyk <roman@tsisyk.com>\n"
"---\n"
"--- Distributed under the MIT/X11 License. See COPYING.md for more details.\n"
"---\n"
"\n"
"local exports = {}\n"
"local methods = {}\n"
"\n"
"--------------------------------------------------------------------------------\n"
"-- Tools\n"
"--------------------------------------------------------------------------------\n"
"\n"
"local return_if_not_empty = function(state_x, ...)\n"
"    if state_x == nil then\n"
"        return nil\n"
"    end\n"
"    return ...\n"
"end\n"
"\n"
"local call_if_not_empty = function(fun, state_x, ...)\n"
"    if state_x == nil then\n"
"        return nil\n"
"    end\n"
"    return state_x, fun(...)\n"
"end\n"
"\n"
"local function deepcopy(orig) -- used by cycle()\n"
"    local orig_type = type(orig)\n"
"    local copy\n"
"    if orig_type == 'table' then\n"
"        copy = {}\n"
"        for orig_key, orig_value in next, orig, nil do\n"
"            copy[deepcopy(orig_key)] = deepcopy(orig_value)\n"
"        end\n"
"    else\n"
"        copy = orig\n"
"    end\n"
"    return copy\n"
"end\n"
"\n"
"local iterator_mt = {\n"
"    -- usually called by for-in loop\n"
"    __call = function(self, param, state)\n"
"        return self.gen(param, state)\n"
"    end;\n"
"    __tostring = function(self)\n"
"        return '<generator>'\n"
"    end;\n"
"    -- add all exported methods\n"
"    __index = methods;\n"
"}\n"
"\n"
"local wrap = function(gen, param, state)\n"
"    return setmetatable({\n"
"        gen = gen,\n"
"        param = param,\n"
"        state = state\n"
"    }, iterator_mt), param, state\n"
"end\n"
"exports.wrap = wrap\n"
"\n"
"local unwrap = function(self)\n"
"    return self.gen, self.param, self.state\n"
"end\n"
"methods.unwrap = unwrap\n"
"\n"
"--------------------------------------------------------------------------------\n"
"-- Basic Functions\n"
"--------------------------------------------------------------------------------\n"
"\n"
"local nil_gen = function(_param, _state)\n"
"    return nil\n"
"end\n"
"\n"
"local string_gen = function(param, state)\n"
"    local state = state + 1\n"
"    if state > #param then\n"
"        return nil\n"
"    end\n"
"    local r = string.sub(param, state, state)\n"
"    return state, r\n"
"end\n"
"\n"
"local pairs_gen = pairs({ a = 0 }) -- get the generating function from pairs\n"
"local map_gen = function(tab, key)\n"
"    local value\n"
"    local key, value = pairs_gen(tab, key)\n"
"    return key, key, value\n"
"end\n"
"\n"
"local rawiter = function(obj, param, state)\n"
"    assert(obj ~= nil, \"invalid iterator\")\n"
"    if type(obj) == \"table\" then\n"
"        local mt = getmetatable(obj);\n"
"        if mt ~= nil then\n"
"            if mt == iterator_mt then\n"
"                return obj.gen, obj.param, obj.state\n"
"            elseif mt.__ipairs ~= nil then\n"
"                return mt.__ipairs(obj)\n"
"            elseif mt.__pairs ~= nil then\n"
"                return mt.__pairs(obj)\n"
"            end\n"
"        end\n"
"        if #obj > 0 then\n"
"            -- array\n"
"            return ipairs(obj)\n"
"        else\n"
"            -- hash\n"
"            return map_gen, obj, nil\n"
"        end\n"
"    elseif (type(obj) == \"function\") then\n"
"        return obj, param, state\n"
"    elseif (type(obj) == \"string\") then\n"
"        if #obj == 0 then\n"
"            return nil_gen, nil, nil\n"
"        end\n"
"        return string_gen, obj, 0\n"
"    end\n"
"    error(string.format('object %s of type \"%s\" is not iterable',\n"
"          obj, type(obj)))\n"
"end\n"
"\n"
"local iter = function(obj, param, state)\n"
"    return wrap(rawiter(obj, param, state))\n"
"end\n"
"exports.iter = iter\n"
"\n"
"local method0 = function(fun)\n"
"    return function(self)\n"
"        return fun(self.gen, self.param, self.state)\n"
"    end\n"
"end\n"
"\n"
"local method1 = function(fun)\n"
"    return function(self, arg1)\n"
"        return fun(arg1, self.gen, self.param, self.state)\n"
"    end\n"
"end\n"
"\n"
"local method2 = function(fun)\n"
"    return function(self, arg1, arg2)\n"
"        return fun(arg1, arg2, self.gen, self.param, self.state)\n"
"    end\n"
"end\n"
"\n"
"local export0 = function(fun)\n"
"    return function(gen, param, state)\n"
"        return fun(rawiter(gen, param, state))\n"
"    end\n"
"end\n"
"\n"
"local export1 = function(fun)\n"
"    return function(arg1, gen, param, state)\n"
"        return fun(arg1, rawiter(gen, param, state))\n"
"    end\n"
"end\n"
"\n"
"local export2 = function(fun)\n"
"    return function(arg1, arg2, gen, param, state)\n"
"        return fun(arg1, arg2, rawiter(gen, param, state))\n"
"    end\n"
"end\n"
"\n"
"local each = function(fun, gen, param, state)\n"
"    repeat\n"
"        state = call_if_not_empty(fun, gen(param, state))\n"
"    until state == nil\n"
"end\n"
"methods.each = method1(each)\n"
"exports.each = export1(each)\n"
"methods.for_each = methods.each\n"
"exports.for_each = exports.each\n"
"methods.foreach = methods.each\n"
"exports.foreach = exports.each\n"
"\n"
"--------------------------------------------------------------------------------\n"
"-- Generators\n"
"--------------------------------------------------------------------------------\n"
"\n"
"local range_gen = function(param, state)\n"
"    local stop, step = param[1], param[2]\n"
"    local state = state + step\n"
"    if state > stop then\n"
"        return nil\n"
"    end\n"
"    return state, state\n"
"end\n"
"\n"
"local range_rev_gen = function(param, state)\n"
"    local stop, step = param[1], param[2]\n"
"    local state = state + step\n"
"    if state < stop then\n"
"        return nil\n"
"    end\n"
"    return state, state\n"
"end\n"
"\n"
"local range = function(start, stop, step)\n"
"    if step == nil then\n"
"        if stop == nil then\n"
"            if start == 0 then\n"
"                return nil_gen, nil, nil\n"
"            end\n"
"            stop = start\n"
"            start = stop > 0 and 1 or -1\n"
"        end\n"
"        step = start <= stop and 1 or -1\n"
"    end\n"
"\n"
"    assert(type(start) == \"number\", \"start must be a number\")\n"
"    assert(type(stop) == \"number\", \"stop must be a number\")\n"
"    assert(type(step) == \"number\", \"step must be a number\")\n"
"    assert(step ~= 0, \"step must not be zero\")\n"
"\n"
"    if (step > 0) then\n"
"        return wrap(range_gen, {stop, step}, start - step)\n"
"    elseif (step < 0) then\n"
"        return wrap(range_rev_gen, {stop, step}, start - step)\n"
"    end\n"
"end\n"
"exports.range = range\n"
"\n"
"local duplicate_table_gen = function(param_x, state_x)\n"
"    return state_x + 1, unpack(param_x)\n"
"end\n"
"\n"
"local duplicate_fun_gen = function(param_x, state_x)\n"
"    return state_x + 1, param_x(state_x)\n"
"end\n"
"\n"
"local duplicate_gen = function(param_x, state_x)\n"
"    return state_x + 1, param_x\n"
"end\n"
"\n"
"local duplicate = function(...)\n"
"    if select('#', ...) <= 1 then\n"
"        return wrap(duplicate_gen, select(1, ...), 0)\n"
"    else\n"
"        return wrap(duplicate_table_gen, {...}, 0)\n"
"    end\n"
"end\n"
"exports.duplicate = duplicate\n"
"exports.replicate = duplicate\n"
"exports.xrepeat = duplicate\n"
"\n"
"local tabulate = function(fun)\n"
"    assert(type(fun) == \"function\")\n"
"    return wrap(duplicate_fun_gen, fun, 0)\n"
"end\n"
"exports.tabulate = tabulate\n"
"\n"
"local zeros = function()\n"
"    return wrap(duplicate_gen, 0, 0)\n"
"end\n"
"exports.zeros = zeros\n"
"\n"
"local ones = function()\n"
"    return wrap(duplicate_gen, 1, 0)\n"
"end\n"
"exports.ones = ones\n"
"\n"
"local rands_gen = function(param_x, _state_x)\n"
"    return 0, math.random(param_x[1], param_x[2])\n"
"end\n"
"\n"
"local rands_nil_gen = function(_param_x, _state_x)\n"
"    return 0, math.random()\n"
"end\n"
"\n"
"local rands = function(n, m)\n"
"    if n == nil and m == nil then\n"
"        return wrap(rands_nil_gen, 0, 0)\n"
"    end\n"
"    assert(type(n) == \"number\", \"invalid first arg to rands\")\n"
"    if m == nil then\n"
"        m = n\n"
"        n = 0\n"
"    else\n"
"        assert(type(m) == \"number\", \"invalid second arg to rands\")\n"
"    end\n"
"    assert(n < m, \"empty interval\")\n"
"    return wrap(rands_gen, {n, m - 1}, 0)\n"
"end\n"
"exports.rands = rands\n"
"\n"
"--------------------------------------------------------------------------------\n"
"-- Slicing\n"
"--------------------------------------------------------------------------------\n"
"\n"
"local nth = function(n, gen_x, param_x, state_x)\n"
"    assert(n > 0, \"invalid first argument to nth\")\n"
"    -- An optimization for arrays and strings\n"
"    if gen_x == ipairs then\n"
"        return param_x[n]\n"
"    elseif gen_x == string_gen then\n"
"        if n < #param_x then\n"
"            return string.sub(param_x, n, n)\n"
"        else\n"
"            return nil\n"
"        end\n"
"    end\n"
"    for i=1,n-1,1 do\n"
"        state_x = gen_x(param_x, state_x)\n"
"        if state_x == nil then\n"
"            return nil\n"
"        end\n"
"    end\n"
"    return return_if_not_empty(gen_x(param_x, state_x))\n"
"end\n"
"methods.nth = method1(nth)\n"
"exports.nth = export1(nth)\n"
"\n"
"local head_call = function(state, ...)\n"
"    if state == nil then\n"
"        error(\"head: iterator is empty\")\n"
"    end\n"
"    return ...\n"
"end\n"
"\n"
"local head = function(gen, param, state)\n"
"    return head_call(gen(param, state))\n"
"end\n"
"methods.head = method0(head)\n"
"exports.head = export0(head)\n"
"exports.car = exports.head\n"
"methods.car = methods.head\n"
"\n"
"local tail = function(gen, param, state)\n"
"    state = gen(param, state)\n"
"    if state == nil then\n"
"        return wrap(nil_gen, nil, nil)\n"
"    end\n"
"    return wrap(gen, param, state)\n"
"end\n"
"methods.tail = method0(tail)\n"
"exports.tail = export0(tail)\n"
"exports.cdr = exports.tail\n"
"methods.cdr = methods.tail\n"
"\n"
"local take_n_gen_x = function(i, state_x, ...)\n"
"    if state_x == nil then\n"
"        return nil\n"
"    end\n"
"    return {i, state_x}, ...\n"
"end\n"
"\n"
"local take_n_gen = function(param, state)\n"
"    local n, gen_x, param_x = param[1], param[2], param[3]\n"
"    local i, state_x = state[1], state[2]\n"
"    if i >= n then\n"
"        return nil\n"
"    end\n"
"    return take_n_gen_x(i + 1, gen_x(param_x, state_x))\n"
"end\n"
"\n"
"local take_n = function(n, gen, param, state)\n"
"    assert(n >= 0, \"invalid first argument to take_n\")\n"
"    return wrap(take_n_gen, {n, gen, param}, {0, state})\n"
"end\n"
"methods.take_n = method1(take_n)\n"
"exports.take_n = export1(take_n)\n"
"\n"
"local take_while_gen_x = function(fun, state_x, ...)\n"
"    if state_x == nil or not fun(...) then\n"
"        return nil\n"
"    end\n"
"    return state_x, ...\n"
"end\n"
"\n"
"local take_while_gen = function(param, state_x)\n"
"    local fun, gen_x, param_x = param[1], param[2], param[3]\n"
"    return take_while_gen_x(fun, gen_x(param_x, state_x))\n"
"end\n"
"\n"
"local take_while = function(fun, gen, param, state)\n"
"    assert(type(fun) == \"function\", \"invalid first argument to take_while\")\n"
"    return wrap(take_while_gen, {fun, gen, param}, state)\n"
"end\n"
"methods.take_while = method1(take_while)\n"
"exports.take_while = export1(take_while)\n"
"\n"
"local take = function(n_or_fun, gen, param, state)\n"
"    if type(n_or_fun) == \"number\" then\n"
"        return take_n(n_or_fun, gen, param, state)\n"
"    else\n"
"        return take_while(n_or_fun, gen, param, state)\n"
"    end\n"
"end\n"
"methods.take = method1(take)\n"
"exports.take = export1(take)\n"
"\n"
"local drop_n = function(n, gen, param, state)\n"
"    assert(n >= 0, \"invalid first argument to drop_n\")\n"
"    local i\n"
"    for i=1,n,1 do\n"
"        state = gen(param, state)\n"
"        if state == nil then\n"
"            return wrap(nil_gen, nil, nil)\n"
"        end\n"
"    end\n"
"    return wrap(gen, param, state)\n"
"end\n"
"methods.drop_n = method1(drop_n)\n"
"exports.drop_n = export1(drop_n)\n"
"\n"
"local drop_while_x = function(fun, state_x, ...)\n"
"    if state_x == nil or not fun(...) then\n"
"        return state_x, false\n"
"    end\n"
"    return state_x, true, ...\n"
"end\n"
"\n"
"local drop_while = function(fun, gen_x, param_x, state_x)\n"
"    assert(type(fun) == \"function\", \"invalid first argument to drop_while\")\n"
"    local cont, state_x_prev\n"
"    repeat\n"
"        state_x_prev = deepcopy(state_x)\n"
"        state_x, cont = drop_while_x(fun, gen_x(param_x, state_x))\n"
"    until not cont\n"
"    if state_x == nil then\n"
"        return wrap(nil_gen, nil, nil)\n"
"    end\n"
"    return wrap(gen_x, param_x, state_x_prev)\n"
"end\n"
"methods.drop_while = method1(drop_while)\n"
"exports.drop_while = export1(drop_while)\n"
"\n"
"local drop = function(n_or_fun, gen_x, param_x, state_x)\n"
"    if type(n_or_fun) == \"number\" then\n"
"        return drop_n(n_or_fun, gen_x, param_x, state_x)\n"
"    else\n"
"        return drop_while(n_or_fun, gen_x, param_x, state_x)\n"
"    end\n"
"end\n"
"methods.drop = method1(drop)\n"
"exports.drop = export1(drop)\n"
"\n"
"local split = function(n_or_fun, gen_x, param_x, state_x)\n"
"    return take(n_or_fun, gen_x, param_x, state_x),\n"
"           drop(n_or_fun, gen_x, param_x, state_x)\n"
"end\n"
"methods.split = method1(split)\n"
"exports.split = export1(split)\n"
"methods.split_at = methods.split\n"
"exports.split_at = exports.split\n"
"methods.span = methods.split\n"
"exports.span = exports.split\n"
"\n"
"--------------------------------------------------------------------------------\n"
"-- Indexing\n"
"--------------------------------------------------------------------------------\n"
"\n"
"local index = function(x, gen, param, state)\n"
"    local i = 1\n"
"    for _k, r in gen, param, state do\n"
"        if r == x then\n"
"            return i\n"
"        end\n"
"        i = i + 1\n"
"    end\n"
"    return nil\n"
"end\n"
"methods.index = method1(index)\n"
"exports.index = export1(index)\n"
"methods.index_of = methods.index\n"
"exports.index_of = exports.index\n"
"methods.elem_index = methods.index\n"
"exports.elem_index = exports.index\n"
"\n"
"local indexes_gen = function(param, state)\n"
"    local x, gen_x, param_x = param[1], param[2], param[3]\n"
"    local i, state_x = state[1], state[2]\n"
"    local r\n"
"    while true do\n"
"        state_x, r = gen_x(param_x, state_x)\n"
"        if state_x == nil then\n"
"            return nil\n"
"        end\n"
"        i = i + 1\n"
"        if r == x then\n"
"            return {i, state_x}, i\n"
"        end\n"
"    end\n"
"end\n"
"\n"
"local indexes = function(x, gen, param, state)\n"
"    return wrap(indexes_gen, {x, gen, param}, {0, state})\n"
"end\n"
"methods.indexes = method1(indexes)\n"
"exports.indexes = export1(indexes)\n"
"methods.elem_indexes = methods.indexes\n"
"exports.elem_indexes = exports.indexes\n"
"methods.indices = methods.indexes\n"
"exports.indices = exports.indexes\n"
"methods.elem_indices = methods.indexes\n"
"exports.elem_indices = exports.indexes\n"
"\n"
"--------------------------------------------------------------------------------\n"
"-- Filtering\n"
"--------------------------------------------------------------------------------\n"
"\n"
"local filter1_gen = function(fun, gen_x, param_x, state_x, a)\n"
"    while true do\n"
"        if state_x == nil or fun(a) then break; end\n"
"        state_x, a = gen_x(param_x, state_x)\n"
"    end\n"
"    return state_x, a\n"
"end\n"
"\n"
"-- call each other\n"
"local filterm_gen\n"
"local filterm_gen_shrink = function(fun, gen_x, param_x, state_x)\n"
"    return filterm_gen(fun, gen_x, param_x, gen_x(param_x, state_x))\n"
"end\n"
"\n"
"filterm_gen = function(fun, gen_x, param_x, state_x, ...)\n"
"    if state_x == nil then\n"
"        return nil\n"
"    end\n"
"    if fun(...) then\n"
"        return state_x, ...\n"
"    end\n"
"    return filterm_gen_shrink(fun, gen_x, param_x, state_x)\n"
"end\n"
"\n"
"local filter_detect = function(fun, gen_x, param_x, state_x, ...)\n"
"    if select('#', ...) < 2 then\n"
"        return filter1_gen(fun, gen_x, param_x, state_x, ...)\n"
"    else\n"
"        return filterm_gen(fun, gen_x, param_x, state_x, ...)\n"
"    end\n"
"end\n"
"\n"
"local filter_gen = function(param, state_x)\n"
"    local fun, gen_x, param_x = param[1], param[2], param[3]\n"
"    return filter_detect(fun, gen_x, param_x, gen_x(param_x, state_x))\n"
"end\n"
"\n"
"local filter = function(fun, gen, param, state)\n"
"    return wrap(filter_gen, {fun, gen, param}, state)\n"
"end\n"
"methods.filter = method1(filter)\n"
"exports.filter = export1(filter)\n"
"methods.remove_if = methods.filter\n"
"exports.remove_if = exports.filter\n"
"\n"
"local grep = function(fun_or_regexp, gen, param, state)\n"
"    local fun = fun_or_regexp\n"
"    if type(fun_or_regexp) == \"string\" then\n"
"        fun = function(x) return string.find(x, fun_or_regexp) ~= nil end\n"
"    end\n"
"    return filter(fun, gen, param, state)\n"
"end\n"
"methods.grep = method1(grep)\n"
"exports.grep = export1(grep)\n"
"\n"
"local partition = function(fun, gen, param, state)\n"
"    local neg_fun = function(...)\n"
"        return not fun(...)\n"
"    end\n"
"    return filter(fun, gen, param, state),\n"
"           filter(neg_fun, gen, param, state)\n"
"end\n"
"methods.partition = method1(partition)\n"
"exports.partition = export1(partition)\n"
"\n"
"--------------------------------------------------------------------------------\n"
"-- Reducing\n"
"--------------------------------------------------------------------------------\n"
"\n"
"local foldl_call = function(fun, start, state, ...)\n"
"    if state == nil then\n"
"        return nil, start\n"
"    end\n"
"    return state, fun(start, ...)\n"
"end\n"
"\n"
"local foldl = function(fun, start, gen_x, param_x, state_x)\n"
"    while true do\n"
"        state_x, start = foldl_call(fun, start, gen_x(param_x, state_x))\n"
"        if state_x == nil then\n"
"            break;\n"
"        end\n"
"    end\n"
"    return start\n"
"end\n"
"methods.foldl = method2(foldl)\n"
"exports.foldl = export2(foldl)\n"
"methods.reduce = methods.foldl\n"
"exports.reduce = exports.foldl\n"
"\n"
"local length = function(gen, param, state)\n"
"    if gen == ipairs or gen == string_gen then\n"
"        return #param\n"
"    end\n"
"    local len = 0\n"
"    repeat\n"
"        state = gen(param, state)\n"
"        len = len + 1\n"
"    until state == nil\n"
"    return len - 1\n"
"end\n"
"methods.length = method0(length)\n"
"exports.length = export0(length)\n"
"\n"
"local is_null = function(gen, param, state)\n"
"    return gen(param, deepcopy(state)) == nil\n"
"end\n"
"methods.is_null = method0(is_null)\n"
"exports.is_null = export0(is_null)\n"
"\n"
"local is_prefix_of = function(iter_x, iter_y)\n"
"    local gen_x, param_x, state_x = iter(iter_x)\n"
"    local gen_y, param_y, state_y = iter(iter_y)\n"
"\n"
"    local r_x, r_y\n"
"    for i=1,10,1 do\n"
"        state_x, r_x = gen_x(param_x, state_x)\n"
"        state_y, r_y = gen_y(param_y, state_y)\n"
"        if state_x == nil then\n"
"            return true\n"
"        end\n"
"        if state_y == nil or r_x ~= r_y then\n"
"            return false\n"
"        end\n"
"    end\n"
"end\n"
"methods.is_prefix_of = is_prefix_of\n"
"exports.is_prefix_of = is_prefix_of\n"
"\n"
"local all = function(fun, gen_x, param_x, state_x)\n"
"    local r\n"
"    repeat\n"
"        state_x, r = call_if_not_empty(fun, gen_x(param_x, state_x))\n"
"    until state_x == nil or not r\n"
"    return state_x == nil\n"
"end\n"
"methods.all = method1(all)\n"
"exports.all = export1(all)\n"
"methods.every = methods.all\n"
"exports.every = exports.all\n"
"\n"
"local any = function(fun, gen_x, param_x, state_x)\n"
"    local r\n"
"    repeat\n"
"        state_x, r = call_if_not_empty(fun, gen_x(param_x, state_x))\n"
"    until state_x == nil or r\n"
"    return not not r\n"
"end\n"
"methods.any = method1(any)\n"
"exports.any = export1(any)\n"
"methods.some = methods.any\n"
"exports.some = exports.any\n"
"\n"
"local sum = function(gen, param, state)\n"
"    local s = 0\n"
"    local r = 0\n"
"    repeat\n"
"        s = s + r\n"
"        state, r = gen(param, state)\n"
"    until state == nil\n"
"    return s\n"
"end\n"
"methods.sum = method0(sum)\n"
"exports.sum = export0(sum)\n"
"\n"
"local product = function(gen, param, state)\n"
"    local p = 1\n"
"    local r = 1\n"
"    repeat\n"
"        p = p * r\n"
"        state, r = gen(param, state)\n"
"    until state == nil\n"
"    return p\n"
"end\n"
"methods.product = method0(product)\n"
"exports.product = export0(product)\n"
"\n"
"local min_cmp = function(m, n)\n"
"    if n < m then return n else return m end\n"
"end\n"
"\n"
"local max_cmp = function(m, n)\n"
"    if n > m then return n else return m end\n"
"end\n"
"\n"
"local min = function(gen, param, state)\n"
"    local state, m = gen(param, state)\n"
"    if state == nil then\n"
"        error(\"min: iterator is empty\")\n"
"    end\n"
"\n"
"    local cmp\n"
"    if type(m) == \"number\" then\n"
"        -- An optimization: use math.min for numbers\n"
"        cmp = math.min\n"
"    else\n"
"        cmp = min_cmp\n"
"    end\n"
"\n"
"    for _, r in gen, param, state do\n"
"        m = cmp(m, r)\n"
"    end\n"
"    return m\n"
"end\n"
"methods.min = method0(min)\n"
"exports.min = export0(min)\n"
"methods.minimum = methods.min\n"
"exports.minimum = exports.min\n"
"\n"
"local min_by = function(cmp, gen_x, param_x, state_x)\n"
"    local state_x, m = gen_x(param_x, state_x)\n"
"    if state_x == nil then\n"
"        error(\"min: iterator is empty\")\n"
"    end\n"
"\n"
"    for _, r in gen_x, param_x, state_x do\n"
"        m = cmp(m, r)\n"
"    end\n"
"    return m\n"
"end\n"
"methods.min_by = method1(min_by)\n"
"exports.min_by = export1(min_by)\n"
"methods.minimum_by = methods.min_by\n"
"exports.minimum_by = exports.min_by\n"
"\n"
"local max = function(gen_x, param_x, state_x)\n"
"    local state_x, m = gen_x(param_x, state_x)\n"
"    if state_x == nil then\n"
"        error(\"max: iterator is empty\")\n"
"    end\n"
"\n"
"    local cmp\n"
"    if type(m) == \"number\" then\n"
"        -- An optimization: use math.max for numbers\n"
"        cmp = math.max\n"
"    else\n"
"        cmp = max_cmp\n"
"    end\n"
"\n"
"    for _, r in gen_x, param_x, state_x do\n"
"        m = cmp(m, r)\n"
"    end\n"
"    return m\n"
"end\n"
"methods.max = method0(max)\n"
"exports.max = export0(max)\n"
"methods.maximum = methods.max\n"
"exports.maximum = exports.max\n"
"\n"
"local max_by = function(cmp, gen_x, param_x, state_x)\n"
"    local state_x, m = gen_x(param_x, state_x)\n"
"    if state_x == nil then\n"
"        error(\"max: iterator is empty\")\n"
"    end\n"
"\n"
"    for _, r in gen_x, param_x, state_x do\n"
"        m = cmp(m, r)\n"
"    end\n"
"    return m\n"
"end\n"
"methods.max_by = method1(max_by)\n"
"exports.max_by = export1(max_by)\n"
"methods.maximum_by = methods.maximum_by\n"
"exports.maximum_by = exports.maximum_by\n"
"\n"
"local totable = function(gen_x, param_x, state_x)\n"
"    local tab, key, val = {}\n"
"    while true do\n"
"        state_x, val = gen_x(param_x, state_x)\n"
"        if state_x == nil then\n"
"            break\n"
"        end\n"
"        table.insert(tab, val)\n"
"    end\n"
"    return tab\n"
"end\n"
"methods.totable = method0(totable)\n"
"exports.totable = export0(totable)\n"
"\n"
"local tomap = function(gen_x, param_x, state_x)\n"
"    local tab, key, val = {}\n"
"    while true do\n"
"        state_x, key, val = gen_x(param_x, state_x)\n"
"        if state_x == nil then\n"
"            break\n"
"        end\n"
"        tab[key] = val\n"
"    end\n"
"    return tab\n"
"end\n"
"methods.tomap = method0(tomap)\n"
"exports.tomap = export0(tomap)\n"
"\n"
"--------------------------------------------------------------------------------\n"
"-- Transformations\n"
"--------------------------------------------------------------------------------\n"
"\n"
"local map_gen = function(param, state)\n"
"    local gen_x, param_x, fun = param[1], param[2], param[3]\n"
"    return call_if_not_empty(fun, gen_x(param_x, state))\n"
"end\n"
"\n"
"local map = function(fun, gen, param, state)\n"
"    return wrap(map_gen, {gen, param, fun}, state)\n"
"end\n"
"methods.map = method1(map)\n"
"exports.map = export1(map)\n"
"\n"
"local enumerate_gen_call = function(state, i, state_x, ...)\n"
"    if state_x == nil then\n"
"        return nil\n"
"    end\n"
"    return {i + 1, state_x}, i, ...\n"
"end\n"
"\n"
"local enumerate_gen = function(param, state)\n"
"    local gen_x, param_x = param[1], param[2]\n"
"    local i, state_x = state[1], state[2]\n"
"    return enumerate_gen_call(state, i, gen_x(param_x, state_x))\n"
"end\n"
"\n"
"local enumerate = function(gen, param, state)\n"
"    return wrap(enumerate_gen, {gen, param}, {1, state})\n"
"end\n"
"methods.enumerate = method0(enumerate)\n"
"exports.enumerate = export0(enumerate)\n"
"\n"
"local intersperse_call = function(i, state_x, ...)\n"
"    if state_x == nil then\n"
"        return nil\n"
"    end\n"
"    return {i + 1, state_x}, ...\n"
"end\n"
"\n"
"local intersperse_gen = function(param, state)\n"
"    local x, gen_x, param_x = param[1], param[2], param[3]\n"
"    local i, state_x = state[1], state[2]\n"
"    if i % 2 == 1 then\n"
"        return {i + 1, state_x}, x\n"
"    else\n"
"        return intersperse_call(i, gen_x(param_x, state_x))\n"
"    end\n"
"end\n"
"\n"
"-- TODO: interperse must not add x to the tail\n"
"local intersperse = function(x, gen, param, state)\n"
"    return wrap(intersperse_gen, {x, gen, param}, {0, state})\n"
"end\n"
"methods.intersperse = method1(intersperse)\n"
"exports.intersperse = export1(intersperse)\n"
"\n"
"--------------------------------------------------------------------------------\n"
"-- Compositions\n"
"--------------------------------------------------------------------------------\n"
"\n"
"local function zip_gen_r(param, state, state_new, ...)\n"
"    if #state_new == #param / 2 then\n"
"        return state_new, ...\n"
"    end\n"
"\n"
"    local i = #state_new + 1\n"
"    local gen_x, param_x = param[2 * i - 1], param[2 * i]\n"
"    local state_x, r = gen_x(param_x, state[i])\n"
"    if state_x == nil then\n"
"        return nil\n"
"    end\n"
"    table.insert(state_new, state_x)\n"
"    return zip_gen_r(param, state, state_new, r, ...)\n"
"end\n"
"\n"
"local zip_gen = function(param, state)\n"
"    return zip_gen_r(param, state, {})\n"
"end\n"
"\n"
"-- A special hack for zip/chain to skip last two state, if a wrapped iterator\n"
"-- has been passed\n"
"local numargs = function(...)\n"
"    local n = select('#', ...)\n"
"    if n >= 3 then\n"
"        -- Fix last argument\n"
"        local it = select(n - 2, ...)\n"
"        if type(it) == 'table' and getmetatable(it) == iterator_mt and\n"
"           it.param == select(n - 1, ...) and it.state == select(n, ...) then\n"
"            return n - 2\n"
"        end\n"
"    end\n"
"    return n\n"
"end\n"
"\n"
"local zip = function(...)\n"
"    local n = numargs(...)\n"
"    if n == 0 then\n"
"        return wrap(nil_gen, nil, nil)\n"
"    end\n"
"    local param = { [2 * n] = 0 }\n"
"    local state = { [n] = 0 }\n"
"\n"
"    local i, gen_x, param_x, state_x\n"
"    for i=1,n,1 do\n"
"        local it = select(n - i + 1, ...)\n"
"        gen_x, param_x, state_x = rawiter(it)\n"
"        param[2 * i - 1] = gen_x\n"
"        param[2 * i] = param_x\n"
"        state[i] = state_x\n"
"    end\n"
"\n"
"    return wrap(zip_gen, param, state)\n"
"end\n"
"methods.zip = zip\n"
"exports.zip = zip\n"
"\n"
"local cycle_gen_call = function(param, state_x, ...)\n"
"    if state_x == nil then\n"
"        local gen_x, param_x, state_x0 = param[1], param[2], param[3]\n"
"        return gen_x(param_x, deepcopy(state_x0))\n"
"    end\n"
"    return state_x, ...\n"
"end\n"
"\n"
"local cycle_gen = function(param, state_x)\n"
"    local gen_x, param_x, state_x0 = param[1], param[2], param[3]\n"
"    return cycle_gen_call(param, gen_x(param_x, state_x))\n"
"end\n"
"\n"
"local cycle = function(gen, param, state)\n"
"    return wrap(cycle_gen, {gen, param, state}, deepcopy(state))\n"
"end\n"
"methods.cycle = method0(cycle)\n"
"exports.cycle = export0(cycle)\n"
"\n"
"-- call each other\n"
"local chain_gen_r1\n"
"local chain_gen_r2 = function(param, state, state_x, ...)\n"
"    if state_x == nil then\n"
"        local i = state[1]\n"
"        i = i + 1\n"
"        if i > #param / 3 then\n"
"            return nil\n"
"        end\n"
"        local state_x = param[3 * i]\n"
"        return chain_gen_r1(param, {i, state_x})\n"
"    end\n"
"    return {state[1], state_x}, ...\n"
"end\n"
"\n"
"chain_gen_r1 = function(param, state)\n"
"    local i, state_x = state[1], state[2]\n"
"    local gen_x, param_x = param[3 * i - 2], param[3 * i - 1]\n"
"    return chain_gen_r2(param, state, gen_x(param_x, state[2]))\n"
"end\n"
"\n"
"local chain = function(...)\n"
"    local n = numargs(...)\n"
"    if n == 0 then\n"
"        return wrap(nil_gen, nil, nil)\n"
"    end\n"
"\n"
"    local param = { [3 * n] = 0 }\n"
"    local i, gen_x, param_x, state_x\n"
"    for i=1,n,1 do\n"
"        local elem = select(i, ...)\n"
"        gen_x, param_x, state_x = iter(elem)\n"
"        param[3 * i - 2] = gen_x\n"
"        param[3 * i - 1] = param_x\n"
"        param[3 * i] = state_x\n"
"    end\n"
"\n"
"    return wrap(chain_gen_r1, param, {1, param[3]})\n"
"end\n"
"methods.chain = chain\n"
"exports.chain = chain\n"
"\n"
"--------------------------------------------------------------------------------\n"
"-- Operators\n"
"--------------------------------------------------------------------------------\n"
"\n"
"operator = {\n"
"    ----------------------------------------------------------------------------\n"
"    -- Comparison operators\n"
"    ----------------------------------------------------------------------------\n"
"    lt  = function(a, b) return a < b end,\n"
"    le  = function(a, b) return a <= b end,\n"
"    eq  = function(a, b) return a == b end,\n"
"    ne  = function(a, b) return a ~= b end,\n"
"    ge  = function(a, b) return a >= b end,\n"
"    gt  = function(a, b) return a > b end,\n"
"\n"
"    ----------------------------------------------------------------------------\n"
"    -- Arithmetic operators\n"
"    ----------------------------------------------------------------------------\n"
"    add = function(a, b) return a + b end,\n"
"    div = function(a, b) return a / b end,\n"
"    floordiv = function(a, b) return math.floor(a/b) end,\n"
"    intdiv = function(a, b)\n"
"        local q = a / b\n"
"        if a >= 0 then return math.floor(q) else return math.ceil(q) end\n"
"    end,\n"
"    mod = function(a, b) return a % b end,\n"
"    mul = function(a, b) return a * b end,\n"
"    neq = function(a) return -a end,\n"
"    unm = function(a) return -a end, -- an alias\n"
"    pow = function(a, b) return a ^ b end,\n"
"    sub = function(a, b) return a - b end,\n"
"    truediv = function(a, b) return a / b end,\n"
"\n"
"    ----------------------------------------------------------------------------\n"
"    -- String operators\n"
"    ----------------------------------------------------------------------------\n"
"    concat = function(a, b) return a..b end,\n"
"    len = function(a) return #a end,\n"
"    length = function(a) return #a end, -- an alias\n"
"\n"
"    ----------------------------------------------------------------------------\n"
"    -- Logical operators\n"
"    ----------------------------------------------------------------------------\n"
"    land = function(a, b) return a and b end,\n"
"    lor = function(a, b) return a or b end,\n"
"    lnot = function(a) return not a end,\n"
"    truth = function(a) return not not a end,\n"
"}\n"
"exports.operator = operator\n"
"methods.operator = operator\n"
"exports.op = operator\n"
"methods.op = operator\n"
"\n"
"--------------------------------------------------------------------------------\n"
"-- module definitions\n"
"--------------------------------------------------------------------------------\n"
"\n"
"-- a special syntax sugar to export all functions to the global table\n"
"setmetatable(exports, {\n"
"    __call = function(t)\n"
"        for k, v in pairs(t) do _G[k] = v end\n"
"    end,\n"
"})\n"
"\n"
"return exports\n"
""
;