json = require 'json'
yaml = require 'yaml'
pickle = require 'pickle'
socket = require 'socket'
fiber = require 'fiber'
msgpack = require 'msgpack'
log = require 'log'
errno = require 'errno'
fio = require 'fio'
type(socket)

socket('PF_INET', 'SOCK_STREAM', 'tcp121222');

s = socket('PF_INET', 'SOCK_STREAM', 'tcp')
s:wait(.01)
type(s)
s:errno()
type(s:error())
-- Invalid arguments
--# setopt delimiter ';'
for k in pairs(getmetatable(s).__index) do
    local r, msg = pcall(s[k])
    if not msg:match('Usage:') then
        error("Arguments is not checked for "..k)
    end
end;
s:close()
--# setopt delimiter ''

LISTEN = require('uri').parse(db.cfg.listen)
LISTEN ~= nil
s = socket.tcp_connect(LISTEN.host, LISTEN.service)
s:nonblock(true)
s:nonblock()
s:nonblock(false)
s:nonblock()
s:nonblock(true)

s:readable(.01)
s:wait(.01)
s:readable(0)
s:errno() > 0
s:error()
s:writable(.00000000000001)
s:writable(0)
s:wait(.01)

handshake = s:sysread(128)
string.len(handshake)
string.sub(handshake, 1, 9)

ping = msgpack.encode({ [0] = 64, [1] = 0 }) .. msgpack.encode({})
ping = msgpack.encode(string.len(ping)) .. ping

s:syswrite(ping)
s:readable(1)
s:wait(.01)

pong = s:sysread()
string.len(pong)
msgpack.decode(pong)
msgpack.decode(pong, 6)

s:close()

s = socket('PF_INET', 'SOCK_STREAM', 'tcp')
s:setsockopt('SOL_SOCKET', 'SO_REUSEADDR', true)
s:error()
s:bind('127.0.0.1', 3457)
s:error()
s:listen(128)
sevres = {}
type(require('fiber').create(function() s:readable() do local sc = s:accept() table.insert(sevres, sc) sc:syswrite('ok') sc:close() end end))
#sevres

sc = socket('PF_INET', 'SOCK_STREAM', 'tcp')
sc:nonblock(false)
sc:sysconnect('127.0.0.1', 3457)
sc:nonblock(true)
sc:readable(.5)
sc:sysread()
string.match(tostring(sc), ', peer') ~= nil
#sevres
sevres[1].host

s:setsockopt('SOL_SOCKET', 'SO_BROADCAST', false)
s:getsockopt('SOL_SOCKET', 'SO_TYPE')
s:error()
s:setsockopt('SOL_SOCKET', 'SO_BSDCOMPAT', false)
s:setsockopt('SOL_SOCKET', 'SO_DEBUG', false)
s:getsockopt('SOL_SOCKET', 'SO_DEBUG')
s:setsockopt('SOL_SOCKET', 'SO_ACCEPTCONN', 1)
s:getsockopt('SOL_SOCKET', 'SO_RCVBUF') > 32
s:error()

s:linger()
s:linger(true, 1)
s:linger()
s:linger(false, 1)
s:linger()
s:shutdown('R')
s:close()

s = socket('PF_INET', 'SOCK_STREAM', 'tcp')
s:setsockopt('SOL_SOCKET', 'SO_REUSEADDR', true)
s:bind('127.0.0.1', 3457)
s:listen(128)

sc = socket('PF_INET', 'SOCK_STREAM', 'tcp')

sc:writable()
sc:readable()
sc:sysconnect('127.0.0.1', 3457) or errno() == errno.EINPROGRESS
sc:writable(10)
sc:write('Hello, world')


sa, addr = s:accept()
addr2 = sa:name()
addr2.host == addr.host
addr2.family == addr.family
sa:nonblock(1)
sa:read(8)
sa:read(3)
sc:writable()
sc:write(', again')
sa:read(8)
sa:error()
string.len(sa:read(0))
type(sa:read(0))
sa:read(1, .01)
sc:writable()

sc:send('abc')
sa:read(3)

sc:send('Hello')
sa:readable()
sa:recv()
sa:recv()

sc:send('Hello')
sc:send(', world')
sc:send("\\nnew line")
sa:read('\\n', 1)
sa:read({ chunk = 1, delimiter = 'ine'}, 1)
sa:read('ine', 1)
sa:read('ine', 0.1)

sc:send('Hello, world')
sa:read(',', 1)
sc:shutdown('W')
sa:read(100, 1)
sa:read(100, 1)
sa:close()
sc:close()

s = socket('PF_UNIX', 'SOCK_STREAM', 0)
s:setsockopt('SOL_SOCKET', 'SO_REUSEADDR', true)
s ~= nil
s:nonblock()
s:nonblock(true)
s:nonblock()
os.remove('/tmp/bee-test-socket')
s:bind('unix/', '/tmp/bee-test-socket')
sc ~= nil
s:listen(1234)

sc = socket('PF_UNIX', 'SOCK_STREAM', 0)
sc:nonblock(true)
sc:sysconnect('unix/', '/tmp/bee-test-socket')
sc:error()

s:readable()
sa = s:accept()
sa:nonblock(true)
sa:send('Hello, world')
sc:recv()

sc:close()
sa:close()
s:close()

os.remove('/tmp/bee-test-socket')

--# setopt delimiter ';'
function aexitst(ai, hostnames, port)
    for i, a in pairs(ai) do
        for j, host in pairs(hostnames) do
            if a.host == host and a.port == port then
                return true
            end
        end
    end
    return ai
end;


aexitst( socket.getaddrinfo('localhost', 'http', {  protocol = 'tcp',
    type = 'SOCK_STREAM'}), {'127.0.0.1', '::1'}, 80 );
--# setopt delimiter ''

#(socket.getaddrinfo('bee.org', 'http', {})) > 0
wrong_addr = socket.getaddrinfo('non-existing-domain-name-12211alklkl.com', 'http', {})
wrong_addr == nil or #wrong_addr == 0

sc = socket('PF_INET', 'SOCK_STREAM', 'tcp')
sc ~= nil
sc:getsockopt('SOL_SOCKET', 'SO_ERROR')
sc:nonblock(true)
sc:readable()
sc:sysconnect('127.0.0.1', 3458) or errno() == errno.EINPROGRESS
string.match(tostring(sc), ', peer') == nil
sc:writable()
string.match(tostring(sc), ', peer') == nil
require('errno').strerror(sc:getsockopt('SOL_SOCKET', 'SO_ERROR'))

--# setopt delimiter ';'
socket.getaddrinfo('127.0.0.1', '80', { type = 'SOCK_DGRAM',
    flags = { 'AI_NUMERICSERV', 'AI_NUMERICHOST', } });
--# setopt delimiter ''

sc = socket('AF_INET', 'SOCK_STREAM', 'tcp')
json.encode(sc:name())
sc:name()
sc:nonblock(true)
sc:close()

s = socket('AF_INET', 'SOCK_DGRAM', 'udp')
s:bind('127.0.0.1', 3548)
sc = socket('AF_INET', 'SOCK_DGRAM', 'udp')
sc:sendto('127.0.0.1', 3548, 'Hello, world')
s:readable(10)
s:recv()

sc:sendto('127.0.0.1', 3548, 'Hello, world, 2')
s:readable(10)
d, from = s:recvfrom()
from.port > 0
from.port = 'Random port'
json.encode{d, from}
s:close()
sc:close()

s = socket('AF_INET', 'SOCK_DGRAM', 'udp')
s:nonblock(true)
s:bind('127.0.0.1')
s:name().port > 0
sc = socket('AF_INET', 'SOCK_DGRAM', 'udp')
sc:nonblock(true)
sc:sendto('127.0.0.1', s:name().port)
sc:sendto('127.0.0.1', s:name().port, 'Hello, World!')
s:readable(1)
data, from = s:recvfrom(10)
data
s:sendto(from.host, from.port, 'Hello, hello!')
sc:readable(1)
data_r, from_r = sc:recvfrom()
data_r
from_r.host
from_r.port == s:name().port
s:close()
sc:close()

-- tcp_connect

s = socket.tcp_connect('bee.org', 80)
string.match(tostring(s), ', aka') ~= nil
string.match(tostring(s), ', peer') ~= nil
s:write("HEAD / HTTP/1.0\r\nHost: bee.org\r\n\r\n")
header = s:read({chunk = 4000, delimiter = {"\n\n", "\r\n\r\n" }}, 1)
string.match(header, "\r\n\r\n$") ~= nil
string.match(header, "200 [Oo][Kk]") ~= nil
s:close()

socket.tcp_connect('127.0.0.1', 80, 0.00000000001)

-- AF_INET
port = 35490
s = socket('AF_INET', 'SOCK_STREAM', 'tcp')
s:bind('127.0.0.1', port)
socket.tcp_connect('127.0.0.1', port), errno() == errno.ECONNREFUSED
s:listen()
sc, e = socket.tcp_connect('127.0.0.1', port), errno()
sc ~= nil
e == 0
sc:close()
s:close()
socket.tcp_connect('127.0.0.1', porrt), errno() == errno.ECONNREFUSED

-- AF_UNIX
path = '/tmp/bee-test-socket'
s = socket('AF_UNIX', 'SOCK_STREAM', 0)
s:bind('unix/', path)
socket.tcp_connect('unix/', path), errno() == errno.ECONNREFUSED
s:listen()
sc, e = socket.tcp_connect('unix/', path), errno()
sc ~= nil
e
sc:close()
s:close()
socket.tcp_connect('unix/', path), errno() == errno.ECONNREFUSED
os.remove(path)
socket.tcp_connect('unix/', path), errno() == errno.ENOENT

-- invalid fd
s = socket('AF_INET', 'SOCK_STREAM', 'tcp')
s:read(9)
s:close()
s.socket.fd = 512
tostring(s)
s:readable(0)
s:writable(0)
s = nil

-- close
port = 65454
serv = socket('AF_INET', 'SOCK_STREAM', 'tcp')
serv:setsockopt('SOL_SOCKET', 'SO_REUSEADDR', true)
serv:bind('127.0.0.1', port)
serv:listen()
--# setopt delimiter ';'
f = fiber.create(function(serv)
    serv:readable()
    sc = serv:accept()
    sc:write("Bee test server")
    sc:shutdown()
    sc:close()
    serv:close()
end, serv);
--# setopt delimiter ''

s = socket.tcp_connect('127.0.0.1', port)
ch = fiber.channel()
f = fiber.create(function() s:read(12) ch:put(true) end)
s:close()
ch:get(1)
s:error()

-- random port
port = 33123
master = socket('PF_INET', 'SOCK_STREAM', 'tcp')
master:setsockopt('SOL_SOCKET', 'SO_REUSEADDR', true)
master:bind('127.0.0.1', port)
master:listen()
--# setopt delimiter ';'
function gh361()
    local s = socket('PF_INET', 'SOCK_STREAM', 'tcp')
    s:sysconnect('127.0.0.1', port)
    s:wait()
    res = s:read(1200)
end;

--# setopt delimiter ''
f = fiber.create(gh361)
fiber.cancel(f)
while f:status() ~= 'dead' do fiber.sleep(0.001) end
master:close()
f = nil


path = '/tmp/bee-test-socket'
s = socket('PF_UNIX', 'SOCK_STREAM', 0)
s:setsockopt('SOL_SOCKET', 'SO_REUSEADDR', true)
s:error()
s:bind('unix/', path)
s:error()
s:listen(128)
--# setopt delimiter ';'
f = fiber.create(function()
    for i=1,2 do
        s:readable()
        local sc = s:accept()
        sc:write('ok!')
        sc:shutdown()
        sc:close()
    end
end);
--# setopt delimiter ''

c = socket.tcp_connect('unix/', path)
c:error()
x = c:read('!')
x, type(x), #x
x = c:read('!')
c:error()
x, type(x), #x
x = c:read('!')
c:error()
x, type(x), #x
c:close()

c = socket.tcp_connect('unix/', path)
c:error()
x = c:read(3)
c:error()
x, type(x), #x
x = c:read(1)
c:error()
x, type(x), #x
x = c:read(1)
c:error()
x, type(x), #x
x = c:sysread(1)
c:error()
x, type(x), #x
c:close()

s:close()

os.remove(path)


server, addr = socket.tcp_server('unix/', path, function(s) s:write('Hello, world') end)
type(addr)
server ~= nil
fiber.sleep(.5)
client = socket.tcp_connect('unix/', path)
client ~= nil
client:read(123)
server:close()
-- unix socket automatically removed
fio.stat(path) == nil

--# setopt delimiter ';'
server, addr = socket.tcp_server('localhost', 0, { handler = function(s)
    s:read(2)
    s:write('Hello, world')
end, name = 'testserv'});
--# setopt delimiter ''
type(addr)
server ~= nil
addr2 = server:name()
addr.host == addr2.host
addr.family == addr2.family
fiber.sleep(.5)
client = socket.tcp_connect(addr2.host, addr2.port)
client ~= nil
-- Check that listen and client fibers have appropriate names
cnt = 0
--# setopt delimiter ';'
for i=100,200 do
    local f = fiber.find(i)
    if f and f:name():match('^testserv/') then
        cnt = cnt + 1
    end
end;
--# setopt delimiter ''
cnt
client:write('hi')
client:read(123)
client:close()
server:close()

longstring = string.rep("abc", 65535)
server = socket.tcp_server('unix/', path, function(s) s:write(longstring) end)
client = socket.tcp_connect('unix/', path)
client:read(#longstring) == longstring
client = socket.tcp_connect('unix/', path)
client:read(#longstring + 1) == longstring
client = socket.tcp_connect('unix/', path)
client:read(#longstring - 1) == string.sub(longstring, 1, #longstring - 1)
longstring = "Hello\r\n\r\nworld\n\n"
client = socket.tcp_connect('unix/', path)
client:read{ line = { "\n\n", "\r\n\r\n" } }
server:close()

-- gh-658: socket:read() incorrectly handles size and delimiter together
body = "a 10\nb 15\nabc"
remaining = #body
--# setopt delimiter ';'
server = socket.tcp_server('unix/', path, function(s)
    s:write(body)
    s:read(100500)
end);
--# setopt delimiter ''
client = socket.tcp_connect('unix/', path)
buf = client:read({ size = remaining, delimiter = "[\r\n]+"})
buf == "a 10\n"
remaining = remaining - #buf
buf = client:read({ size = remaining, delimiter = "[\r\n]+"})
buf == "b 15\n"
remaining = remaining - #buf
buf = client:read({ size = remaining, delimiter = "[\r\n]+"})
buf == "abc"
remaining = remaining - #buf
remaining == 0
buf = client:read({ size = remaining, delimiter = "[\r\n]+"})
buf == ""
buf = client:read({ size = remaining, delimiter = "[\r\n]+"})
buf == ""
client:close()
server:close()

-- Test that socket is closed on GC
s = socket('AF_UNIX', 'SOCK_STREAM', 'ip')
s:bind('unix/', path)
s:listen()
s = nil
collectgarbage('collect')
collectgarbage('collect')
socket.tcp_connect('unix/', path), errno() == errno.ECONNREFUSED
os.remove(path)

-- Test serializers with sockets
s = socket('AF_UNIX', 'SOCK_STREAM', 'ip')
x = s:wait()
-- waiters is map
s.waiters
-- check __serialize hook
json.decode(json.encode(s)).fd == s:fd()
yaml.decode(yaml.encode(s)).fd == s:fd()
s = nil

-- start AF_UNIX server with dead socket exists
path = '/tmp/bee-test-socket'
s = socket('AF_UNIX', 'SOCK_STREAM', 0)
s:bind('unix/', path)
s:close()

s = socket('AF_UNIX', 'SOCK_STREAM', 0)
{ s:bind('unix/', path), errno.strerror() }
s:close()

s = socket.tcp_server('unix/', path, function() end)
s ~= nil
s:close()
fio.stat(path) == nil

{ socket.tcp_connect('abrakadabra#123') == nil, errno.strerror() }


-- wrong options for getaddrinfo
socket.getaddrinfo('host', 'port', { type = 'WRONG' }) == nil and errno() == errno.EINVAL
socket.getaddrinfo('host', 'port', { family = 'WRONG' }) == nil and errno() == errno.EINVAL
socket.getaddrinfo('host', 'port', { protocol = 'WRONG' }) == nil and errno() == errno.EINVAL
socket.getaddrinfo('host', 'port', { flags = 'WRONG' }) == nil and errno() == errno.EINVAL
