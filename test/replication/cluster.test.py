import os
import sys
import re
import yaml
import uuid
import glob
from lib.bee_server import BeeServer

## Get cluster uuid
cluster_uuid = ''
try:
    cluster_uuid = yaml.load(server.admin("db.space._schema:get('cluster')",
        silent = True))[0][1]
    uuid.UUID('{' + cluster_uuid + '}')
    print 'ok - cluster uuid'
except Exception as e:
    print 'not ok - invalid cluster uuid', e

print '-------------------------------------------------------------'
print ' gh-696: Check global READ permissions for replication'
print '-------------------------------------------------------------'

# Generate replica cluster UUID
replica_uuid = str(uuid.uuid4())

## Universal read permission is required to perform JOIN/SUBSCRIBE
rows = list(server.sql.py_con.join(replica_uuid))
print len(rows) == 1 and rows[0].return_message.find('Read access') >= 0 and \
    'ok' or 'not ok', '-', 'join without read permissions to universe'
rows = list(server.sql.py_con.subscribe(cluster_uuid, replica_uuid))
print len(rows) == 1 and rows[0].return_message.find('Read access') >= 0 and \
    'ok' or 'not ok', '-', 'subscribe without read permissions to universe'

## Write permission to space `_cluster` is required to perform JOIN
server.admin("db.schema.user.grant('guest', 'read', 'universe')")
server.sql.py_con.close() # re-connect with new permissions
rows = list(server.sql.py_con.join(replica_uuid))
print len(rows) == 1 and rows[0].return_message.find('Write access') >= 0 and \
    'ok' or 'not ok', '-', 'join without write permissions to _cluster'

def check_join(msg):
    ok = True
    for resp in server.sql.py_con.join(replica_uuid):
        if resp.completion_status != 0:
            print 'not ok', '-', msg, resp.return_message
            ok = False

    server.sql.py_con.close() # JOIN brokes protocol
    if not ok:
        return
    tuples = server.sql.py_con.space('_cluster').select(replica_uuid, index = 1)
    if len(tuples) == 0:
        print 'not ok', '-', msg, 'missing entry in _cluster'
        return
    server_id = tuples[0][0]
    print 'ok', '-', msg
    return server_id

## JOIN with permissions
server.admin("db.schema.user.grant('guest', 'write', 'space', '_cluster')")
server.sql.py_con.close() # re-connect with new permissions
server_id = check_join('join with granted permissions')
server.sql.py_con.space('_cluster').delete(server_id)

# JOIN with granted role
server.admin("db.schema.user.revoke('guest', 'read', 'universe')")
server.admin("db.schema.user.revoke('guest', 'write', 'space', '_cluster')")
server.admin("db.schema.user.grant('guest', 'replication')")
server.sql.py_con.close() # re-connect with new permissions
server_id = check_join('join with granted role')
server.sql.py_con.space('_cluster').delete(server_id)

print '-------------------------------------------------------------'
print 'gh-707: Master crashes on JOIN if it does not have snapshot files'
print 'gh-480: If socket is closed while JOIN, replica wont reconnect'
print '-------------------------------------------------------------'

for k in glob.glob(os.path.join(server.vardir, '*.snap')):
    os.unlink(k)

# remember the number of servers in _cluster table
server_count = len(server.sql.py_con.space('_cluster').select(()))

rows = list(server.sql.py_con.join(replica_uuid))
print len(rows) == 1 and rows[0].return_message.find('snapshot') >= 0 and \
    'ok' or 'not ok', '-', 'join without snapshots'

print server_count == len(server.sql.py_con.space('_cluster').select(())) and\
    'ok' or 'not ok', '-', '_cluster does not changed after unsuccessful JOIN'

server.admin("db.schema.user.revoke('guest', 'replication')")
server.admin('db.snapshot()')

print '-------------------------------------------------------------'
print 'gh-434: Assertion if replace _cluster tuple'
print '-------------------------------------------------------------'
server.stop()
script = server.script
server.script = "replication/panic.lua"
server.deploy()

new_uuid = '8c7ff474-65f9-4abe-81a4-a3e1019bb1ae'

# Check log message
# Requires panic_on_wal_error = false
server.admin("db.space._cluster:replace{{1, '{0}'}}".format(new_uuid))
server.admin("db.info.server.uuid")

line = "server UUID changed to " + new_uuid
print "check log line for '%s'" % line
print
if server.logfile_pos.seek_once(line) >= 0:
    print "'%s' exists in server log" % line
print
server.admin("db.info.server.uuid")

# Check that new UUID has been saved in snapshot
server.admin("db.snapshot()")
server.restart()

server.admin("db.info.server.uuid")

# Invalid UUID
server.admin("db.space._cluster:replace{1, require('uuid').NULL:str()}")

# Cleanup
server.stop()
server.script = script
server.deploy()

print '-------------------------------------------------------------'
print 'gh-527: update vclock on delete from db.space._cluster'
print '-------------------------------------------------------------'

# master server
master = server
master_id = master.get_param('server')['id']

master.admin("db.schema.user.grant('guest', 'replication')")

replica = BeeServer(server.ini)
replica.script = 'replication/replica.lua'
replica.vardir = os.path.join(server.vardir, 'replica')
replica.rpl_master = master
replica.deploy()
replica.wait_lsn(master_id, master.get_lsn(master_id))
replica_id = replica.get_param('server')['id']
replica_uuid = replica.get_param('server')['uuid']
sys.stdout.push_filter(replica_uuid, '<replica uuid>')
replica.admin('db.space._schema:insert{"test", 48}')

replica.admin('db.info.server.id')
replica.admin('db.info.server.ro')
replica.admin('db.info.server.lsn') # 1
replica.admin('db.info.vclock[%d]' % replica_id)

master.admin('db.space._cluster:delete{%d}' % replica_id)
replica.wait_lsn(master_id, master.get_lsn(master_id))
replica.admin('db.info.server.id')
replica.admin('db.info.server.ro')
replica.admin('db.info.server.lsn') # -1
replica.admin('db.info.vclock[%d]' % replica_id)
# replica is read-only
replica.admin('db.space._schema:replace{"test", 48}')

replica_id2 = 10
master.admin('db.space._cluster:insert{%d, "%s"}' %
    (replica_id2, replica_uuid))
replica.wait_lsn(master_id, master.get_lsn(master_id))
replica.admin('db.info.server.id')
replica.admin('db.info.server.ro')
replica.admin('db.info.server.lsn') # 0
replica.admin('db.info.vclock[%d]' % replica_id)
replica.admin('db.info.vclock[%d]' % replica_id2)

replica_id3 = 11
server.admin("db.space._cluster:update(%d, {{'=', 1, %d}})" %
    (replica_id2, replica_id3))
replica.wait_lsn(master_id, master.get_lsn(master_id))
replica.admin('db.info.server.id')
replica.admin('db.info.server.ro')
replica.admin('db.info.server.lsn') # 0
replica.admin('db.info.vclock[%d]' % replica_id)
replica.admin('db.info.vclock[%d]' % replica_id2)
replica.admin('db.info.vclock[%d]' % replica_id3)
replica.stop()
replica.cleanup(True)

# Cleanup
sys.stdout.pop_filter()

master.admin("db.schema.user.revoke('guest', 'replication')")
