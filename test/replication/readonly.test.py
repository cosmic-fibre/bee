import os
from glob import iglob as glob
from lib.bee_server import BeeServer

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
replica.admin('db.info.server.id')
replica.admin('db.info.server.ro')
replica.admin('db.info.server.lsn')
replica.stop()

print '-------------------------------------------------------------'
print 'replica is read-only until receive self server_id in _cluster'
print '-------------------------------------------------------------'

# Remove xlog retrived by SUBSCRIBE
filename = str(0).zfill(20) + ".xlog"
wal = os.path.join(replica.vardir, filename)
os.remove(wal)

# Start replica without master
server.stop()
replica.start()
replica.admin('db.cfg{replication_source = ""}')

# Check that replica in read-only mode
replica.admin('db.info.server.id')
replica.admin('db.info.server.ro')
replica.admin('db.info.server.lsn')
replica.admin('space = db.schema.space.create("ro")')
replica.admin('db.info.vclock[%d]' % replica_id)

replica.stop()
replica.cleanup(True)
server.deploy()
