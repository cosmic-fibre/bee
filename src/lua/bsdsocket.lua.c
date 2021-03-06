const char bsdsocket_lua[] =
"-- bsdsocket.lua (internal file)\n"
"\n"
"local TIMEOUT_INFINITY      = 500 * 365 * 86400\n"
"local LIMIT_INFINITY = 4294967295\n"
"local READAHEAD = 16380\n"
"\n"
"local ffi = require('ffi')\n"
"local dberrno = require('errno')\n"
"local internal = require('socket')\n"
"local fiber = require('fiber')\n"
"local fio = require('fio')\n"
"local log = require('log')\n"
"\n"
"ffi.cdef[[\n"
"    struct socket {\n"
"        int fd;\n"
"    };\n"
"    typedef uint32_t socklen_t;\n"
"    typedef ptrdiff_t ssize_t;\n"
"\n"
"    int connect(int sockfd, const struct sockaddr *addr,\n"
"                socklen_t addrlen);\n"
"    int bind(int sockfd, const struct sockaddr *addr,\n"
"                socklen_t addrlen);\n"
"\n"
"    ssize_t write(int fd, const char *octets, size_t len);\n"
"    ssize_t read(int fd, void *buf, size_t count);\n"
"    int listen(int fd, int backlog);\n"
"    int socket(int domain, int type, int protocol);\n"
"    int close(int s);\n"
"    int shutdown(int s, int how);\n"
"    ssize_t send(int sockfd, const void *buf, size_t len, int flags);\n"
"    ssize_t recv(int s, void *buf, size_t len, int flags);\n"
"    int accept(int s, void *addr, void *addrlen);\n"
"    ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,\n"
"                   const struct sockaddr *dest_addr, socklen_t addrlen);\n"
"\n"
"    int\n"
"    bsdsocket_local_resolve(const char *host, const char *port,\n"
"                            struct sockaddr *addr, socklen_t *socklen);\n"
"    int bsdsocket_nonblock(int fd, int mode);\n"
"\n"
"    int setsockopt(int s, int level, int iname, const void *opt, size_t optlen);\n"
"    int getsockopt(int s, int level, int iname, void *ptr, size_t *optlen);\n"
"\n"
"    typedef struct { int active; int timeout; } linger_t;\n"
"\n"
"    struct protoent {\n"
"        char  *p_name;       /* official protocol name */\n"
"        char **p_aliases;    /* alias list */\n"
"        int    p_proto;      /* protocol number */\n"
"    };\n"
"    struct protoent *getprotobyname(const char *name);\n"
"]]\n"
"\n"
"local function sprintf(fmt, ...)\n"
"    return string.format(fmt, ...)\n"
"end\n"
"\n"
"local function printf(fmt, ...)\n"
"    print(sprintf(fmt, ...))\n"
"end\n"
"\n"
"local socket_t = ffi.typeof('struct socket');\n"
"\n"
"local function socket_cdata_gc(socket)\n"
"    if socket.fd < 0 then\n"
"        log.error(\"socket: attempt to double close on gc\")\n"
"        return\n"
"    end\n"
"    if ffi.C.close(socket.fd) ~= 0 then\n"
"        log.error(\"socket: failed to close fd=%d on gc: %s\", socket.fd,\n"
"            dberrno.strerror())\n"
"    end\n"
"end\n"
"\n"
"local function check_socket(self)\n"
"    local socket = type(self) == 'table' and self.socket\n"
"    if not ffi.istype(socket_t, socket) then\n"
"        error('Usage: socket:method()');\n"
"    end\n"
"    if socket.fd < 0 then\n"
"        error(\"attempt to use closed socket\")\n"
"    end\n"
"    return socket.fd\n"
"end\n"
"\n"
"local socket_mt\n"
"local function bless_socket(fd)\n"
"    -- Make socket to be non-blocked by default\n"
"    if ffi.C.bsdsocket_nonblock(fd, 1) < 0 then\n"
"        local errno = dberrno()\n"
"        ffi.C.close(fd)\n"
"        dberrno(errno)\n"
"        return nil\n"
"    end\n"
"\n"
"    local socket = ffi.new(socket_t, { fd = fd });\n"
"    ffi.gc(socket, socket_cdata_gc)\n"
"    return setmetatable({ socket = socket }, socket_mt)\n"
"end\n"
"\n"
"local function get_ivalue(table, key)\n"
"    if type(key) == 'number' then\n"
"        return key\n"
"    end\n"
"    return table[key]\n"
"end\n"
"\n"
"local function get_iflags(table, flags)\n"
"    if flags == nil then\n"
"        return 0\n"
"    end\n"
"    local res = 0\n"
"    if type(flags) ~= 'table' then\n"
"        flags = { flags }\n"
"    end\n"
"    for i, f in pairs(flags) do\n"
"        if table[f] == nil then\n"
"            return nil\n"
"        end\n"
"        res = bit.bor(res, table[f])\n"
"    end\n"
"    return res\n"
"end\n"
"\n"
"local socket_methods  = {}\n"
"socket_methods.errno = function(self)\n"
"    check_socket(self)\n"
"    if self['_errno'] == nil then\n"
"        return 0\n"
"    else\n"
"        return self['_errno']\n"
"    end\n"
"end\n"
"\n"
"socket_methods.error = function(self)\n"
"    check_socket(self)\n"
"    if self['_errno'] == nil then\n"
"        return nil\n"
"    else\n"
"        return dberrno.strerror(self._errno)\n"
"    end\n"
"end\n"
"\n"
"-- addrbuf is equivalent to struct sockaddr_storage\n"
"local addrbuf = ffi.new('char[128]') -- enough to fit any address\n"
"local addr = ffi.cast('struct sockaddr *', addrbuf)\n"
"local addr_len = ffi.new('socklen_t[1]')\n"
"socket_methods.sysconnect = function(self, host, port)\n"
"    local fd = check_socket(self)\n"
"    self._errno = nil\n"
"\n"
"    host = tostring(host)\n"
"    port = tostring(port)\n"
"\n"
"    addr_len[0] = ffi.sizeof(addrbuf)\n"
"    local res = ffi.C.bsdsocket_local_resolve(host, port, addr, addr_len)\n"
"    if res == 0 then\n"
"        res = ffi.C.connect(fd, addr, addr_len[0]);\n"
"        if res == 0 then\n"
"            return true\n"
"        end\n"
"    end\n"
"    self._errno = dberrno()\n"
"    return false\n"
"end\n"
"\n"
"socket_methods.syswrite = function(self, octets)\n"
"    local fd = check_socket(self)\n"
"    self._errno = nil\n"
"    local done = ffi.C.write(fd, octets, string.len(octets))\n"
"    if done < 0 then\n"
"        self._errno = dberrno()\n"
"        return nil\n"
"    end\n"
"    return tonumber(done)\n"
"end\n"
"\n"
"socket_methods.sysread = function(self, size)\n"
"    local fd = check_socket(self)\n"
"    size = size or READAHEAD\n"
"    self._errno = nil\n"
"    local buf = ffi.new('char[?]', size)\n"
"    local res = ffi.C.read(fd, buf, size)\n"
"\n"
"    if res < 0 then\n"
"        self._errno = dberrno()\n"
"        return nil\n"
"    end\n"
"\n"
"    buf = ffi.string(buf, res)\n"
"    return buf\n"
"end\n"
"\n"
"socket_methods.nonblock = function(self, nb)\n"
"    local fd = check_socket(self)\n"
"    self._errno = nil\n"
"\n"
"    local res\n"
"\n"
"    if nb == nil then\n"
"        res = ffi.C.bsdsocket_nonblock(fd, 0x80)\n"
"    elseif nb then\n"
"        res = ffi.C.bsdsocket_nonblock(fd, 1)\n"
"    else\n"
"        res = ffi.C.bsdsocket_nonblock(fd, 0)\n"
"    end\n"
"\n"
"    if res < 0 then\n"
"        self._errno = dberrno()\n"
"        return nil\n"
"    end\n"
"\n"
"    if res == 1 then\n"
"        return true\n"
"    else\n"
"        return false\n"
"    end\n"
"end\n"
"\n"
"local waiters_mt = { __serialize = 'mapping' }\n"
"local function wait_safely(self, what, timeout)\n"
"    local fd = check_socket(self)\n"
"    local f = fiber.self()\n"
"    local fid = f:id()\n"
"\n"
"    self._errno = nil\n"
"    timeout = timeout or TIMEOUT_INFINITY\n"
"\n"
"    if self.waiters == nil then\n"
"        self.waiters = setmetatable({}, waiters_mt)\n"
"    end\n"
"\n"
"    self.waiters[fid] = true\n"
"    local res = internal.iowait(fd, what, timeout)\n"
"    self.waiters[fid] = nil\n"
"    fiber.testcancel()\n"
"    if res == 0 then\n"
"        self._errno = dberrno.ETIMEDOUT\n"
"        return 0\n"
"    end\n"
"    return res\n"
"end\n"
"\n"
"socket_methods.readable = function(self, timeout)\n"
"    return wait_safely(self, 0, timeout) ~= 0\n"
"end\n"
"\n"
"socket_methods.wait = function(self, timeout)\n"
"    local wres = wait_safely(self, 2, timeout)\n"
"    local res = ''\n"
"    if bit.band(wres, 1) ~= 0 then\n"
"        res = res .. 'R'\n"
"    end\n"
"    if bit.band(wres, 2) ~= 0 then\n"
"        res = res .. 'W'\n"
"    end\n"
"    return res\n"
"end\n"
"\n"
"socket_methods.writable = function(self, timeout)\n"
"    return wait_safely(self, 1, timeout) ~= 0\n"
"end\n"
"\n"
"socket_methods.listen = function(self, backlog)\n"
"    local fd = check_socket(self)\n"
"    self._errno = nil\n"
"    if backlog == nil then\n"
"        backlog = 256\n"
"    end\n"
"    local res = ffi.C.listen(fd, backlog)\n"
"    if res < 0 then\n"
"        self._errno = dberrno()\n"
"        return false\n"
"    end\n"
"    return true\n"
"end\n"
"\n"
"socket_methods.bind = function(self, host, port)\n"
"    local fd = check_socket(self)\n"
"    self._errno = nil\n"
"\n"
"    host = tostring(host)\n"
"    port = tostring(port)\n"
"\n"
"    addr_len[0] = ffi.sizeof(addrbuf)\n"
"    local res = ffi.C.bsdsocket_local_resolve(host, port, addr, addr_len)\n"
"    if res == 0 then\n"
"        res = ffi.C.bind(fd, addr, addr_len[0]);\n"
"    end\n"
"    if res == 0 then\n"
"        return true\n"
"    end\n"
"\n"
"    self._errno = dberrno()\n"
"    return false\n"
"end\n"
"\n"
"socket_methods.close = function(self)\n"
"    local fd = check_socket(self)\n"
"    if self.waiters ~= nil then\n"
"        for fid in pairs(self.waiters) do\n"
"            internal.abort(fid)\n"
"            self.waiters[fid] = nil\n"
"        end\n"
"    end\n"
"\n"
"    self._errno = nil\n"
"    local r = ffi.C.close(fd)\n"
"    self.socket.fd = -1\n"
"    ffi.gc(self.socket, nil)\n"
"    if r ~= 0 then\n"
"        self._errno = dberrno()\n"
"        return false\n"
"    end\n"
"    return true\n"
"end\n"
"\n"
"socket_methods.shutdown = function(self, how)\n"
"    local fd = check_socket(self)\n"
"    local hvariants = {\n"
"        ['R']           = 0,\n"
"        ['READ']        = 0,\n"
"        ['W']           = 1,\n"
"        ['WRITE']       = 1,\n"
"        ['RW']          = 2,\n"
"        ['READ_WRITE']  = 2,\n"
"\n"
"        [0]             = 0,\n"
"        [1]             = 1,\n"
"        [2]             = 2\n"
"    }\n"
"    local ihow = hvariants[how]\n"
"\n"
"    if ihow == nil then\n"
"        ihow = 2\n"
"    end\n"
"    self._errno = nil\n"
"    if ffi.C.shutdown(fd, ihow) < 0 then\n"
"        self._errno = dberrno()\n"
"        return false\n"
"    end\n"
"    return true\n"
"end\n"
"\n"
"socket_methods.setsockopt = function(self, level, name, value)\n"
"    local fd = check_socket(self)\n"
"\n"
"    local info = get_ivalue(internal.SO_OPT, name)\n"
"\n"
"    if info == nil then\n"
"        error(sprintf(\"Unknown socket option name: %s\", tostring(name)))\n"
"    end\n"
"\n"
"    if not info.rw then\n"
"        error(sprintf(\"Socket option %s is read only\", name))\n"
"    end\n"
"\n"
"    self._errno = nil\n"
"\n"
"    if type(level) == 'string' then\n"
"        if level == 'SOL_SOCKET' then\n"
"            level = internal.SOL_SOCKET\n"
"        else\n"
"            local p = ffi.C.getprotobyname(level)\n"
"            if p == nil then\n"
"                self._errno = dberrno()\n"
"                return false\n"
"            end\n"
"            level = p.p_proto\n"
"        end\n"
"    else\n"
"        level = tonumber(level)\n"
"    end\n"
"\n"
"    if type(value) == 'boolean' then\n"
"        if value then\n"
"            value = 1\n"
"        else\n"
"            value = 0\n"
"        end\n"
"    end\n"
"\n"
"    if info.type == 1 then\n"
"        local value = ffi.new(\"int[1]\", value)\n"
"        local res = ffi.C.setsockopt(fd,\n"
"            level, info.iname, value, ffi.sizeof('int'))\n"
"\n"
"        if res < 0 then\n"
"            self._errno = dberrno()\n"
"            return false\n"
"        end\n"
"        return true\n"
"    end\n"
"\n"
"    if info.type == 2 then\n"
"        local res = ffi.C.setsockopt(fd,\n"
"            level, info.iname, value, ffi.sizeof('size_t'))\n"
"        if res < 0 then\n"
"            self._errno = dberrno()\n"
"            return false\n"
"        end\n"
"        return true\n"
"    end\n"
"\n"
"    if name == 'SO_LINGER' then\n"
"        error(\"Use s:linger(active[, timeout])\")\n"
"    end\n"
"    error(sprintf(\"Unsupported socket option: %s\", name))\n"
"end\n"
"\n"
"socket_methods.getsockopt = function(self, level, name)\n"
"    local fd = check_socket(self)\n"
"\n"
"    local info = get_ivalue(internal.SO_OPT, name)\n"
"\n"
"    if info == nil then\n"
"        error(sprintf(\"Unknown socket option name: %s\", tostring(name)))\n"
"    end\n"
"\n"
"    self._errno = nil\n"
"\n"
"    if type(level) == 'string' then\n"
"        if level == 'SOL_SOCKET' then\n"
"            level = internal.SOL_SOCKET\n"
"        else\n"
"            local p = ffi.C.getprotobyname(level)\n"
"            if p == nil then\n"
"                self._errno = dberrno(dberrno.EINVAL)\n"
"                return nil\n"
"            end\n"
"            level = p.p_proto\n"
"        end\n"
"    else\n"
"        level = tonumber(level)\n"
"    end\n"
"\n"
"\n"
"    if info.type == 1 then\n"
"        local value = ffi.new(\"int[1]\", 0)\n"
"        local len = ffi.new(\"size_t[1]\", ffi.sizeof('int'))\n"
"        local res = ffi.C.getsockopt(fd, level, info.iname, value, len)\n"
"\n"
"        if res < 0 then\n"
"            self._errno = dberrno()\n"
"            return nil\n"
"        end\n"
"\n"
"        if len[0] ~= 4 then\n"
"            error(sprintf(\"Internal error: unexpected optlen: %d\", len[0]))\n"
"        end\n"
"        return tonumber(value[0])\n"
"    end\n"
"\n"
"    if info.type == 2 then\n"
"        local value = ffi.new(\"char[256]\", { 0 })\n"
"        local len = ffi.new(\"size_t[1]\", 256)\n"
"        local res = ffi.C.getsockopt(fd, level, info.iname, value, len)\n"
"        if res < 0 then\n"
"            self._errno = dberrno()\n"
"            return nil\n"
"        end\n"
"        return ffi.string(value, tonumber(len[0]))\n"
"    end\n"
"\n"
"    if name == 'SO_LINGER' then\n"
"        error(\"Use s:linger()\")\n"
"    end\n"
"    error(sprintf(\"Unsupported socket option: %s\", name))\n"
"end\n"
"\n"
"socket_methods.linger = function(self, active, timeout)\n"
"    local fd = check_socket(self)\n"
"\n"
"    local info = internal.SO_OPT.SO_LINGER\n"
"    self._errno = nil\n"
"    if active == nil then\n"
"        local value = ffi.new(\"linger_t[1]\")\n"
"        local len = ffi.new(\"size_t[1]\", 2 * ffi.sizeof('int'))\n"
"        local res = ffi.C.getsockopt(fd,\n"
"            internal.SOL_SOCKET, info.iname, value, len)\n"
"        if res < 0 then\n"
"            self._errno = dberrno()\n"
"            return nil\n"
"        end\n"
"        if value[0].active ~= 0 then\n"
"            active = true\n"
"        else\n"
"            active = false\n"
"        end\n"
"        return active, value[0].timeout\n"
"    end\n"
"\n"
"    if timeout == nil then\n"
"        timeout = 0\n"
"    end\n"
"\n"
"    local iactive\n"
"    if active then\n"
"        iactive = 1\n"
"    else\n"
"        iactive = 0\n"
"    end\n"
"\n"
"    local value = ffi.new(\"linger_t[1]\",\n"
"        { { active = iactive, timeout = timeout } })\n"
"    local len = 2 * ffi.sizeof('int')\n"
"    local res = ffi.C.setsockopt(fd,\n"
"        internal.SOL_SOCKET, info.iname, value, len)\n"
"    if res < 0 then\n"
"        self._errno = dberrno()\n"
"        return nil\n"
"    end\n"
"\n"
"    return active, timeout\n"
"end\n"
"\n"
"socket_methods.accept = function(self)\n"
"    local fd = check_socket(self)\n"
"    self._errno = nil\n"
"\n"
"    local cfd, from = internal.accept(fd)\n"
"    if cfd == nil then\n"
"        self._errno = dberrno()\n"
"        return nil\n"
"    end\n"
"    local c = bless_socket(cfd)\n"
"    if c == nil then\n"
"        self._errno = dberrno()\n"
"        return nil\n"
"    end\n"
"    return c, from\n"
"end\n"
"\n"
"local errno_is_transient = {\n"
"    [dberrno.EAGAIN] = true;\n"
"    [dberrno.EWOULDBLOCK] = true;\n"
"    [dberrno.EINTR] = true;\n"
"}\n"
"\n"
"local errno_is_fatal = {\n"
"    [dberrno.EBADF] = true;\n"
"    [dberrno.EINVAL] = true;\n"
"    [dberrno.EOPNOTSUPP] = true;\n"
"    [dberrno.ENOTSOCK] = true;\n"
"}\n"
"\n"
"local function readchunk(self, limit, timeout)\n"
"    if self.rbuf == nil then\n"
"        self.rbuf = ''\n"
"        self.rpos = 1\n"
"        self.rlen = 0\n"
"    end\n"
"\n"
"    if self.rlen >= limit then\n"
"        self._errno = nil\n"
"        local data = string.sub(self.rbuf, self.rpos, self.rpos - 1 + limit)\n"
"        self.rlen = self.rlen - limit\n"
"        self.rpos = self.rpos + limit\n"
"        return data\n"
"    end\n"
"\n"
"    while true do\n"
"        local started = fiber.time()\n"
"\n"
"        local to_read\n"
"        if limit ~= LIMIT_INFINITY and limit > READAHEAD then\n"
"            to_read = limit - self.rlen\n"
"        end\n"
"        local data = self:sysread(to_read)\n"
"        if data ~= nil then\n"
"            self.rbuf = string.sub(self.rbuf, self.rpos) .. data\n"
"            self.rpos = 1\n"
"            self.rlen = string.len(self.rbuf)\n"
"            if string.len(data) == 0 then   -- eof\n"
"                limit = self.rlen\n"
"            end\n"
"            if self.rlen >= limit then\n"
"               data = string.sub(self.rbuf, self.rpos, self.rpos - 1 + limit)\n"
"               self.rlen = self.rlen - limit\n"
"               self.rpos = self.rpos + limit\n"
"               return data\n"
"            end\n"
"        elseif not errno_is_transient[self:errno()] then\n"
"            self._errno = dberrno()\n"
"            return nil\n"
"        end\n"
"\n"
"        if not self:readable(timeout) then\n"
"            return nil\n"
"        end\n"
"        if timeout <= 0 then\n"
"            break\n"
"        end\n"
"        timeout = timeout - ( fiber.time() - started )\n"
"    end\n"
"    self._errno = dberrno.ETIMEDOUT\n"
"    return nil\n"
"end\n"
"\n"
"local function readline_check(self, eols, limit)\n"
"    if limit == 0 then\n"
"        return ''\n"
"    end\n"
"    if self.rlen == 0 then\n"
"        return nil\n"
"    end\n"
"\n"
"    local shortest\n"
"    for i, eol in ipairs(eols) do\n"
"        local data = string.match(self.rbuf, \"^(.-\" .. eol .. \")\", self.rpos)\n"
"        if data ~= nil then\n"
"            if string.len(data) > limit then\n"
"                data = string.sub(data, 1, limit)\n"
"            end\n"
"            if shortest == nil or string.len(shortest) > string.len(data) then\n"
"                shortest = data\n"
"            end\n"
"        end\n"
"    end\n"
"    if shortest == nil and self.rlen >= limit then\n"
"        shortest = string.sub(self.rbuf, self.rpos, self.rpos - 1 + limit)\n"
"    end\n"
"    if shortest ~= nil then\n"
"        local len = string.len(shortest)\n"
"        self.rpos = self.rpos + len\n"
"        self.rlen = self.rlen - len\n"
"    end\n"
"    return shortest\n"
"end\n"
"\n"
"local function readline(self, limit, eol, timeout)\n"
"    if self.rbuf == nil then\n"
"        self.rbuf = ''\n"
"        self.rpos = 1\n"
"        self.rlen = 0\n"
"    end\n"
"\n"
"    self._errno = nil\n"
"    local data = readline_check(self, eol, limit)\n"
"    if data ~= nil then\n"
"        return data\n"
"    end\n"
"\n"
"    local started = fiber.time()\n"
"    while timeout > 0 do\n"
"        local started = fiber.time()\n"
"        \n"
"        if not self:readable(timeout) then\n"
"            self._errno = dberrno()\n"
"            return nil\n"
"        end\n"
"        \n"
"        timeout = timeout - ( fiber.time() - started )\n"
"\n"
"        local to_read\n"
"        if limit ~= LIMIT_INFINITY and limit > READAHEAD then\n"
"            to_read = limit - self.rlen\n"
"        end\n"
"        local data = self:sysread(to_read)\n"
"        if data ~= nil then\n"
"            self.rbuf = string.sub(self.rbuf, self.rpos) .. data\n"
"            self.rpos = 1\n"
"            self.rlen = string.len(self.rbuf)\n"
"            if string.len(data) == 0 then       -- eof\n"
"                limit = self.rlen\n"
"            end\n"
"            data = readline_check(self, eol, limit)\n"
"            if data ~= nil then\n"
"                return data\n"
"            end\n"
"\n"
"        elseif not errno_is_transient[self:errno()] then\n"
"            self._errno = dberrno()\n"
"            return nil\n"
"        end\n"
"    end\n"
"    return nil\n"
"end\n"
"\n"
"socket_methods.read = function(self, opts, timeout)\n"
"    check_socket(self)\n"
"    timeout = timeout or TIMEOUT_INFINITY\n"
"    if type(opts) == 'number' then\n"
"        return readchunk(self, opts, timeout)\n"
"    elseif type(opts) == 'string' then\n"
"        return readline(self, LIMIT_INFINITY, { opts }, timeout)\n"
"    elseif type(opts) == 'table' then\n"
"        local chunk = opts.chunk or opts.size or 4294967295\n"
"        local delimiter = opts.delimiter or opts.line\n"
"        if delimiter == nil then\n"
"            return readchunk(self, chunk, timeout)\n"
"        elseif type(delimiter) == 'string' then\n"
"            return readline(self, chunk, { delimiter }, timeout)\n"
"        elseif type(delimiter) == 'table' then\n"
"            return readline(self, chunk, delimiter, timeout)\n"
"        end\n"
"    end\n"
"    error('Usage: s:read(delimiter|chunk|{delimiter = x, chunk = x}, timeout)')\n"
"end\n"
"\n"
"socket_methods.write = function(self, octets, timeout)\n"
"    check_socket(self)\n"
"    if timeout == nil then\n"
"        timeout = TIMEOUT_INFINITY\n"
"    end\n"
"\n"
"    local total_len = #octets\n"
"    local started = fiber.time()\n"
"    while true do\n"
"        local written = self:syswrite(octets)\n"
"        if written == nil then\n"
"            if not errno_is_transient[self:errno()] then\n"
"                return nil\n"
"            end\n"
"            written = 0\n"
"        end\n"
"\n"
"        if written == string.len(octets) then\n"
"            return total_len\n"
"        end\n"
"        if written > 0 then\n"
"            octets = string.sub(octets, written + 1)\n"
"        end\n"
"        timeout = timeout - (fiber.time() - started)\n"
"        if timeout <= 0 or not self:writable(timeout) then\n"
"            break\n"
"        end\n"
"    end\n"
"end\n"
"\n"
"socket_methods.send = function(self, octets, flags)\n"
"    local fd = check_socket(self)\n"
"    local iflags = get_iflags(internal.SEND_FLAGS, flags)\n"
"\n"
"    self._errno = nil\n"
"    local res = ffi.C.send(fd, octets, string.len(octets), iflags)\n"
"    if res < 0 then\n"
"        self._errno = dberrno()\n"
"        return nil\n"
"    end\n"
"    return tonumber(res)\n"
"end\n"
"\n"
"socket_methods.recv = function(self, size, flags)\n"
"    local fd = check_socket(self)\n"
"    local iflags = get_iflags(internal.SEND_FLAGS, flags)\n"
"    if iflags == nil then\n"
"        self._errno = dberrno.EINVAL\n"
"        return nil\n"
"    end\n"
"\n"
"    size = size or 512\n"
"    self._errno = nil\n"
"    local buf = ffi.new(\"char[?]\", size)\n"
"\n"
"    local res = ffi.C.recv(fd, buf, size, iflags)\n"
"\n"
"    if res == -1 then\n"
"        self._errno = dberrno()\n"
"        return nil\n"
"    end\n"
"    return ffi.string(buf, res)\n"
"end\n"
"\n"
"socket_methods.recvfrom = function(self, size, flags)\n"
"    local fd = check_socket(self)\n"
"    local iflags = get_iflags(internal.SEND_FLAGS, flags)\n"
"    if iflags == nil then\n"
"        self._errno = dberrno.EINVAL\n"
"        return nil\n"
"    end\n"
"\n"
"    size = size or 512\n"
"    self._errno = nil\n"
"    local res, from = internal.recvfrom(fd, size, iflags)\n"
"    if res == nil then\n"
"        self._errno = dberrno()\n"
"        return nil\n"
"    end\n"
"    return res, from\n"
"end\n"
"\n"
"socket_methods.sendto = function(self, host, port, octets, flags)\n"
"    local fd = check_socket(self)\n"
"    local iflags = get_iflags(internal.SEND_FLAGS, flags)\n"
"\n"
"    if iflags == nil then\n"
"        self._errno = dberrno.EINVAL\n"
"        return nil\n"
"    end\n"
"\n"
"    self._errno = nil\n"
"    if octets == nil or octets == '' then\n"
"        return true\n"
"    end\n"
"\n"
"    host = tostring(host)\n"
"    port = tostring(port)\n"
"    octets = tostring(octets)\n"
"\n"
"    addr_len[0] = ffi.sizeof(addrbuf)\n"
"    local res = ffi.C.bsdsocket_local_resolve(host, port, addr, addr_len)\n"
"    if res == 0 then\n"
"        res = ffi.C.sendto(fd, octets, string.len(octets), iflags,\n"
"            addr, addr_len[0])\n"
"    end\n"
"    if res < 0 then\n"
"        self._errno = dberrno()\n"
"        return nil\n"
"    end\n"
"    return tonumber(res)\n"
"end\n"
"\n"
"local function create_socket(domain, stype, proto)\n"
"    local idomain = get_ivalue(internal.DOMAIN, domain)\n"
"    if idomain == nil then\n"
"        dberrno(dberrno.EINVAL)\n"
"        return nil\n"
"    end\n"
"\n"
"    local itype = get_ivalue(internal.SO_TYPE, stype)\n"
"    if itype == nil then\n"
"        dberrno(dberrno.EINVAL)\n"
"        return nil\n"
"    end\n"
"\n"
"    local iproto\n"
"    if type(proto) == 'string' then\n"
"        local p = ffi.C.getprotobyname(proto)\n"
"        if p == nil then\n"
"            dberrno(dberrno.EINVAL)\n"
"            return nil\n"
"        end\n"
"        iproto = p.p_proto\n"
"    else\n"
"        iproto = tonumber(proto)\n"
"    end\n"
"\n"
"    local fd = ffi.C.socket(idomain, itype, iproto)\n"
"    if fd < 1 then\n"
"        return nil\n"
"    end\n"
"    return bless_socket(fd)\n"
"end\n"
"\n"
"local function getaddrinfo(host, port, timeout, opts)\n"
"    if type(timeout) == 'table' and opts == nil then\n"
"        opts = timeout\n"
"        timeout = TIMEOUT_INFINITY\n"
"    elseif timeout == nil then\n"
"        timeout = TIMEOUT_INFINITY\n"
"    end\n"
"    if port == nil then\n"
"        port = 0\n"
"    end\n"
"    local ga_opts = {}\n"
"    if opts ~= nil then\n"
"        if opts.type ~= nil then\n"
"            local itype = get_ivalue(internal.SO_TYPE, opts.type)\n"
"            if itype == nil then\n"
"                dberrno(dberrno.EINVAL)\n"
"                return nil\n"
"            end\n"
"            ga_opts.type = itype\n"
"        end\n"
"\n"
"        if opts.family ~= nil then\n"
"            local ifamily = get_ivalue(internal.DOMAIN, opts.family)\n"
"            if ifamily == nil then\n"
"                dberrno(dberrno.EINVAL)\n"
"                return nil\n"
"            end\n"
"            ga_opts.family = ifamily\n"
"        end\n"
"\n"
"        if opts.protocol ~= nil then\n"
"            local p = ffi.C.getprotobyname(opts.protocol)\n"
"            if p == nil then\n"
"                dberrno(dberrno.EINVAL)\n"
"                return nil\n"
"            end\n"
"            ga_opts.protocol = p.p_proto\n"
"        end\n"
"\n"
"        if opts.flags ~= nil then\n"
"            ga_opts.flags =\n"
"                get_iflags(internal.AI_FLAGS, opts.flags)\n"
"            if ga_opts.flags == nil then\n"
"                dberrno(dberrno.EINVAL)\n"
"                return nil\n"
"            end\n"
"        end\n"
"\n"
"    end\n"
"    return internal.getaddrinfo(host, port, timeout, ga_opts)\n"
"end\n"
"\n"
"local soname_mt = {\n"
"    __tostring = function(si)\n"
"        if si.host == nil and si.port == nil then\n"
"            return ''\n"
"        end\n"
"        if si.host == nil then\n"
"            return sprintf('%s:%s', '0', tostring(si.port))\n"
"        end\n"
"\n"
"        if si.port == nil then\n"
"            return sprintf('%s:%', tostring(si.host), 0)\n"
"        end\n"
"        return sprintf('%s:%s', tostring(si.host), tostring(si.port))\n"
"    end\n"
"}\n"
"\n"
"socket_methods.name = function(self)\n"
"    local fd = check_socket(self)\n"
"    local aka = internal.name(fd)\n"
"    if aka == nil then\n"
"        self._errno = dberrno()\n"
"        return nil\n"
"    end\n"
"    self._errno = nil\n"
"    setmetatable(aka, soname_mt)\n"
"    return aka\n"
"end\n"
"\n"
"socket_methods.peer = function(self)\n"
"    local fd = check_socket(self)\n"
"    local peer = internal.peer(fd)\n"
"    if peer == nil then\n"
"        self._errno = dberrno()\n"
"        return nil\n"
"    end\n"
"    self._errno = nil\n"
"    setmetatable(peer, soname_mt)\n"
"    return peer\n"
"end\n"
"\n"
"socket_methods.fd = function(self)\n"
"    return check_socket(self)\n"
"end\n"
"\n"
"-- tcp connector\n"
"local function tcp_connect_remote(remote, timeout)\n"
"    local s = create_socket(remote.family, remote.type, remote.protocol)\n"
"    if not s then\n"
"        -- Address family is not supported by the host\n"
"        return nil\n"
"    end\n"
"    local res = s:sysconnect(remote.host, remote.port)\n"
"    if res then\n"
"        -- Even through the socket is nonblocking, if the server to which we\n"
"        -- are connecting is on the same host, the connect is normally\n"
"        -- established immediately when we call connect (Stevens UNP).\n"
"        dberrno(0)\n"
"        return s\n"
"    end\n"
"    local save_errno = s:errno()\n"
"    if save_errno ~= dberrno.EINPROGRESS then\n"
"        s:close()\n"
"        dberrno(save_errno)\n"
"        return nil\n"
"    end\n"
"    res = s:wait(timeout)\n"
"    save_errno = s:errno()\n"
"    -- When the connection completes succesfully, the descriptor becomes\n"
"    -- writable. When the connection establishment encounters an error,\n"
"    -- the descriptor becomes both readable and writable (Stevens UNP).\n"
"    -- However, in real life if server sends some data immediately on connect,\n"
"    -- descriptor can also become RW. For this case we also check SO_ERROR.\n"
"    if res ~= nil and res ~= 'W' then\n"
"        save_errno = s:getsockopt('SOL_SOCKET', 'SO_ERROR')\n"
"    end\n"
"    if save_errno ~= 0 then\n"
"        s:close()\n"
"        dberrno(save_errno)\n"
"        return nil\n"
"    end\n"
"    -- Connected\n"
"    dberrno(0)\n"
"    return s\n"
"end\n"
"\n"
"local function tcp_connect(host, port, timeout)\n"
"    if host == 'unix/' then\n"
"        return tcp_connect_remote({ host = host, port = port, protocol = 0,\n"
"            family = 'PF_UNIX', type = 'SOCK_STREAM' }, timeout)\n"
"    end\n"
"    local timeout = timeout or TIMEOUT_INFINITY\n"
"    local stop = fiber.time() + timeout\n"
"    local dns = getaddrinfo(host, port, timeout, { type = 'SOCK_STREAM',\n"
"        protocol = 'tcp' })\n"
"    if dns == nil or #dns == 0 then\n"
"        dberrno(dberrno.EINVAL)\n"
"        return nil\n"
"    end\n"
"    for i, remote in pairs(dns) do\n"
"        timeout = stop - fiber.time()\n"
"        if timeout <= 0 then\n"
"            dberrno(dberrno.ETIMEDOUT)\n"
"            return nil\n"
"        end\n"
"        local s = tcp_connect_remote(remote, timeout)\n"
"        if s then\n"
"            return s\n"
"        end\n"
"    end\n"
"    -- errno is set by tcp_connect_remote()\n"
"    return nil\n"
"end\n"
"\n"
"local function tcp_server_handler(server, sc, from)\n"
"    fiber.name(sprintf(\"%s/%s:%s\", server.name, from.host, from.port))\n"
"    server.handler(sc, from)\n"
"    sc:shutdown()\n"
"    sc:close()\n"
"end\n"
"\n"
"local function tcp_server_loop(server, s, addr)\n"
"    fiber.name(sprintf(\"%s/%s:%s\", server.name, addr.host, addr.port))\n"
"    log.info(\"started\")\n"
"    while s:readable() do\n"
"        local sc, from = s:accept()\n"
"        if sc == nil then\n"
"            local errno = s:errno()\n"
"            if not errno_is_transient[errno] then\n"
"                log.error('accept(%s) failed: %s', tostring(s), s:error())\n"
"            end\n"
"            if  errno_is_fatal[errno] then\n"
"                break\n"
"            end\n"
"        else\n"
"            fiber.create(tcp_server_handler, server, sc, from)\n"
"        end\n"
"    end\n"
"    -- Socket was closed\n"
"    if addr.family == 'AF_UNIX' and addr.port then\n"
"        fio.unlink(addr.port) -- remove unix socket\n"
"    end\n"
"    log.info(\"stopped\")\n"
"end\n"
"\n"
"local function tcp_server_usage()\n"
"    error('Usage: socket.tcp_server(host, port, handler | opts)')\n"
"end\n"
"\n"
"local function tcp_server_bind(s, addr)\n"
"    if s:bind(addr.host, addr.port) then\n"
"        return true\n"
"    end\n"
"\n"
"    if addr.family ~= 'AF_UNIX' then\n"
"        return false\n"
"    end\n"
"\n"
"    if dberrno() ~= dberrno.EADDRINUSE then\n"
"        return false\n"
"    end\n"
"\n"
"    local save_errno = dberrno()\n"
"\n"
"    local sc = tcp_connect(addr.host, addr.port)\n"
"    if sc ~= nil then\n"
"        sc:close()\n"
"        dberrno(save_errno)\n"
"        return false\n"
"    end\n"
"\n"
"    if dberrno() ~= dberrno.ECONNREFUSED then\n"
"        dberrno(save_errno)\n"
"        return false\n"
"    end\n"
"\n"
"    log.info(\"tcp_server: remove dead UNIX socket: %s\", addr.port)\n"
"    if not fio.unlink(addr.port) then\n"
"        log.warn(\"tcp_server: %s\", dberrno.strerror())\n"
"        dberrno(save_errno)\n"
"        return false\n"
"    end\n"
"    return s:bind(addr.host, addr.port)\n"
"end\n"
"\n"
"\n"
"local function tcp_server(host, port, opts, timeout)\n"
"    local server = {}\n"
"    if type(opts) == 'function' then\n"
"        server.handler = opts\n"
"    elseif type(opts) == 'table' then\n"
"        if type(opts.handler) ~='function' or (opts.prepare ~= nil and\n"
"            type(opts.prepare) ~= 'function') then\n"
"            tcp_server_usage()\n"
"        end\n"
"        for k, v in pairs(opts) do\n"
"            server[k] = v\n"
"        end\n"
"    else\n"
"        tcp_server_usage()\n"
"    end\n"
"    server.name = server.name or 'server'\n"
"    timeout = timeout and tonumber(timeout) or TIMEOUT_INFINITY\n"
"    local dns\n"
"    if host == 'unix/' then\n"
"        dns = {{host = host, port = port, family = 'AF_UNIX', protocol = 0,\n"
"            type = 'SOCK_STREAM' }}\n"
"    else\n"
"        dns = getaddrinfo(host, port, timeout, { type = 'SOCK_STREAM',\n"
"            flags = 'AI_PASSIVE'})\n"
"        if dns == nil then\n"
"            return nil\n"
"        end\n"
"    end\n"
"\n"
"    for _, addr in ipairs(dns) do\n"
"        local s = create_socket(addr.family, addr.type, addr.protocol)\n"
"        if s ~= nil then\n"
"            local backlog\n"
"            if server.prepare then\n"
"                backlog = server.prepare(s)\n"
"            else\n"
"                s:setsockopt('SOL_SOCKET', 'SO_REUSEADDR', 1) -- ignore error\n"
"            end\n"
"            if not tcp_server_bind(s, addr) or not s:listen(backlog) then\n"
"                local save_errno = dberrno()\n"
"                s:close()\n"
"                dberrno(save_errno)\n"
"                return nil\n"
"            end\n"
"            fiber.create(tcp_server_loop, server, s, addr)\n"
"            return s, addr\n"
"       end\n"
"    end\n"
"    -- DNS resolved successfully, but addresss family is not supported\n"
"    dberrno(dberrno.EAFNOSUPPORT)\n"
"    return nil\n"
"end\n"
"\n"
"socket_mt   = {\n"
"    __index     = socket_methods,\n"
"    __tostring  = function(self)\n"
"        local fd = check_socket(self)\n"
"\n"
"        local save_errno = self._errno\n"
"        local name = sprintf(\"fd %d\", fd)\n"
"        local aka = self:name()\n"
"        if aka ~= nil then\n"
"            name = sprintf(\"%s, aka %s:%s\", name, aka.host, aka.port)\n"
"        end\n"
"        local peer = self:peer(self)\n"
"        if peer ~= nil then\n"
"            name = sprintf(\"%s, peer %s:%s\", name, peer.host, peer.port)\n"
"        end\n"
"        self._errno = save_errno\n"
"        return name\n"
"    end,\n"
"    __serialize = function(self)\n"
"        -- Allow YAML, MsgPack and JSON to dump objects with sockets\n"
"        local fd = check_socket(self)\n"
"        return { fd = fd, peer = self:peer(), name = self:name() }\n"
"    end\n"
"}\n"
"\n"
"return setmetatable({\n"
"    getaddrinfo = getaddrinfo,\n"
"    tcp_connect = tcp_connect,\n"
"    tcp_server = tcp_server\n"
"}, {\n"
"    __call = function(self, ...) return create_socket(...) end;\n"
"})\n"
""
;
