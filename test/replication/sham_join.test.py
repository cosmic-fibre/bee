import os
import glob
from lib.bee_server import BeeServer

# master server
master = server
master_id = master.get_param('server')['id']

master.admin("db.schema.user.grant('guest', 'replication')")
master.admin("space = db.schema.space.create('test', { id = 99999, engine = \"sham\" })")
master.admin("index = space:create_index('primary', { type = 'tree'})")
master.admin('for k = 1, 123 do space:insert{k, k*k} end')
master.admin('db.snapshot()')
lsn = master.get_lsn(master_id)

print '-------------------------------------------------------------'
print 'replica JOIN'
print '-------------------------------------------------------------'

# replica server
replica = BeeServer(server.ini)
replica.script = 'replication/replica.lua'
replica.vardir = os.path.join(server.vardir, 'replica')
replica.rpl_master = master
replica.deploy()
replica.wait_lsn(master_id, lsn)
replica.admin('db.space.test:select()')

replica.stop()
replica.cleanup(True)

# remove space
master.admin("space:drop()")
master.admin('db.snapshot()')
master.admin("ffi = require('ffi')")
master.admin("ffi.cdef(\"int sham_schedule(void);\")")
master.admin("ffi.C.sham_schedule() >= 0")
