import os
import sys
import struct
import socket
import msgpack
from bee.const import *
from bee import Connection
from bee.request import Request, RequestInsert, RequestSelect
from bee.response import Response
from lib.bee_connection import BeeConnection

admin("db.schema.user.grant('guest', 'read,write,execute', 'universe')")

print """
#
# iproto packages test
#
"""

# opeing new connection to bee/db
conn = BeeConnection(server.sql.host, server.sql.port)
conn.connect()
s = conn.socket

print """
# Test bug #899343 (server assertion failure on incorrect packet)
"""
print "# send the package with invalid length"
invalid_request = struct.pack('<LLL', 1, 4294967290, 1)
print s.send(invalid_request)
print "# check that is server alive"
sql("ping")

# closing connection
s.close()

key_names = {}
for (k,v) in globals().items():
    if type(k) == str and k.startswith('IPROTO_') and type(v) == int:
        key_names[v] = k

def repr_dict(todump):
    d = {}
    for (k, v) in todump.items():
        k_name = key_names.get(k, k)
        d[k_name] = v
    return repr(d)

def test(header, body):
    # Connect and authenticate
    c = Connection('localhost', server.sql.port)
    c.connect()
    print 'query', repr_dict(header), repr_dict(body)
    header = msgpack.dumps(header)
    body = msgpack.dumps(body)
    query = msgpack.dumps(len(header) + len(body)) + header + body
    # Send raw request using connectred socket
    s = c._socket
    try:
        s.send(query)
    except OSError as e:
        print '   => ', 'Failed to send request'
    c.close()
    sql("ping")

print """
#  Test gh-206 "Segfault if sending IPROTO package without `KEY` field"
"""

print "IPROTO_SELECT"
test({ IPROTO_CODE : REQUEST_TYPE_SELECT }, { IPROTO_SPACE_ID: 280 })
print "\n"

print "IPROTO_DELETE"
test({ IPROTO_CODE : REQUEST_TYPE_DELETE }, { IPROTO_SPACE_ID: 280 })
print "\n"

print "IPROTO_UPDATE"
test({ IPROTO_CODE : REQUEST_TYPE_UPDATE }, { IPROTO_SPACE_ID: 280 })
test({ IPROTO_CODE : REQUEST_TYPE_UPDATE },
     { IPROTO_SPACE_ID: 280, IPROTO_KEY: (1, )})
print "\n"

print "IPROTO_REPLACE"
test({ IPROTO_CODE : REQUEST_TYPE_REPLACE }, { IPROTO_SPACE_ID: 280 })
print "\n"

print "IPROTO_CALL"
test({ IPROTO_CODE : REQUEST_TYPE_CALL }, {})
test({ IPROTO_CODE : REQUEST_TYPE_CALL }, { IPROTO_KEY: ('procname', )})
print "\n"

# gh-434 Bee crashes on multiple iproto requests with WAL enabled
admin("db.cfg.wal_mode")
admin("space = db.schema.space.create('test', { id = 567 })")
admin("index = space:create_index('primary', { type = 'hash' })")
admin("db.schema.user.grant('guest', 'read,write,execute', 'space', 'test')")

c = Connection('localhost', server.sql.port)
c.connect()
request1 = RequestInsert(c, 567, [1, "baobab"])
request2 = RequestInsert(c, 567, [2, "obbaba"])
s = c._socket
try:
    s.send(bytes(request1) + bytes(request2))
except OSError as e:
    print '   => ', 'Failed to send request'
response1 = Response(c, c._read_response())
response2 = Response(c, c._read_response())
print response1.__str__()
print response2.__str__()

request1 = RequestInsert(c, 567, [3, "occama"])
request2 = RequestSelect(c, 567, 0, [1], 0, 1, 0)
s = c._socket
try:
    s.send(bytes(request1) + bytes(request2))
except OSError as e:
    print '   => ', 'Failed to send request'
response1 = Response(c, c._read_response())
response2 = Response(c, c._read_response())
print response1.__str__()
print response2.__str__()

request1 = RequestSelect(c, 567, 0, [2], 0, 1, 0)
request2 = RequestInsert(c, 567, [4, "ockham"])
s = c._socket
try:
    s.send(bytes(request1) + bytes(request2))
except OSError as e:
    print '   => ', 'Failed to send request'
response1 = Response(c, c._read_response())
response2 = Response(c, c._read_response())
print response1.__str__()
print response2.__str__()

request1 = RequestSelect(c, 567, 0, [1], 0, 1, 0)
request2 = RequestSelect(c, 567, 0, [2], 0, 1, 0)
s = c._socket
try:
    s.send(bytes(request1) + bytes(request2))
except OSError as e:
    print '   => ', 'Failed to send request'
response1 = Response(c, c._read_response())
response2 = Response(c, c._read_response())
print response1.__str__()
print response2.__str__()

c.close()

admin("space:drop()")

#
# gh-522: Broken compatibility with msgpack-python for strings of size 33..255
#
admin("space = db.schema.space.create('test')")
admin("index = space:create_index('primary', { type = 'hash', parts = {1, 'str'}})")

class RawInsert(Request):
    request_type = REQUEST_TYPE_INSERT
    def __init__(self, conn, space_no, blob):
        super(RawInsert, self).__init__(conn)
        request_body = "\x82" + msgpack.dumps(IPROTO_SPACE_ID) + \
            msgpack.dumps(space_id) + msgpack.dumps(IPROTO_TUPLE) + blob
        self._bytes = self.header(len(request_body)) + request_body

class RawSelect(Request):
    request_type = REQUEST_TYPE_SELECT
    def __init__(self, conn, space_no, blob):
        super(RawSelect, self).__init__(conn)
        request_body = "\x83" + msgpack.dumps(IPROTO_SPACE_ID) + \
            msgpack.dumps(space_id) + msgpack.dumps(IPROTO_KEY) + blob + \
            msgpack.dumps(IPROTO_LIMIT) + msgpack.dumps(100);
        self._bytes = self.header(len(request_body)) + request_body

c = sql.py_con
space = c.space('test')
space_id = space.space_no

TESTS = [
    (1,     "\xa1", "\xd9\x01", "\xda\x00\x01", "\xdb\x00\x00\x00\x01"),
    (31,    "\xbf", "\xd9\x1f", "\xda\x00\x1f", "\xdb\x00\x00\x00\x1f"),
    (32,    "\xd9\x20", "\xda\x00\x20", "\xdb\x00\x00\x00\x20"),
    (255,   "\xd9\xff", "\xda\x00\xff", "\xdb\x00\x00\x00\xff"),
    (256,   "\xda\x01\x00", "\xdb\x00\x00\x01\x00"),
    (65535, "\xda\xff\xff", "\xdb\x00\x00\xff\xff"),
    (65536, "\xdb\x00\x01\x00\x00"),
]

for test in TESTS:
    it = iter(test)
    size = next(it)
    print 'STR', size
    print '--'
    for fmt in it:
        print '0x' + fmt.encode('hex'), '=>',
        field = '*' * size
        c._send_request(RawInsert(c, space_id, "\x91" + fmt + field))
        tuple = space.select(field)[0]
        print len(tuple[0])== size and 'ok' or 'fail',
        it2 = iter(test)
        next(it2)
        for fmt2 in it2:
            tuple = c._send_request(RawSelect(c, space_id,
                "\x91" + fmt2 + field))[0]
            print len(tuple[0]) == size and 'ok' or 'fail',
        tuple = space.delete(field)[0]
        print len(tuple[0]) == size and 'ok' or 'fail',
        print
    print

admin("space:drop()")
admin("db.schema.user.revoke('guest', 'read,write,execute', 'universe')")
