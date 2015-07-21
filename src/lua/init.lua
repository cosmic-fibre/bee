-- init.lua -- internal file

-- Override pcall to support Bee exceptions

local ffi = require('ffi')
ffi.cdef[[
char *
bee_error_message(void);
double
bee_uptime(void);
typedef int32_t pid_t;
pid_t getpid(void);
]]

local pcall_lua = pcall

local function pcall_wrap(status, ...)
    if status == false and ... == 'C++ exception' then
        return false, ffi.string(ffi.C.bee_error_message())
    end
    return status, ...
end
pcall = function(fun, ...)
    return pcall_wrap(pcall_lua(fun, ...))
end

dostring = function(s, ...)
    local chunk, message = loadstring(s)
    if chunk == nil then
        error(message, 2)
    end
    return chunk(...)
end

local function uptime()
    return tonumber(ffi.C.bee_uptime());
end

local function pid()
    return tonumber(ffi.C.getpid())
end

return {
    uptime = uptime;
    pid = pid;
}
