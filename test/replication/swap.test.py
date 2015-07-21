import os
import bee
from lib.bee_server import BeeServer
import re
import yaml

REPEAT = 20
ID_BEGIN = 0
ID_STEP = 5
LOGIN = 'test'
PASSWORD = 'pass123456'

def insert_tuples(_server, begin, end, msg = "tuple"):
    for i in range(begin, end):
        _server.sql("insert into t0 values (%d, '%s %d')" % (i, msg, i))

def select_tuples(_server, begin, end):
    for i in range(begin, end):
        _server.sql("select * from t0 where k0 = %d" % i)

# master server
master = server
master.admin("db.schema.user.create('%s', { password = '%s'})" % (LOGIN, PASSWORD))
master.admin("db.schema.user.grant('%s', 'read,write,execute', 'universe')" % LOGIN)
master.sql.py_con.authenticate(LOGIN, PASSWORD)
master.uri = '%s:%s@%s' % (LOGIN, PASSWORD, master.sql.uri)
os.putenv('MASTER', master.uri)

# replica server
replica = BeeServer()
replica.script = "replication/replica.lua"
replica.vardir = os.path.join(server.vardir, 'replica')
replica.deploy()
replica.admin("while db.info.server.id == 0 do require('fiber').sleep(0.01) end")
replica.uri = '%s:%s@%s' % (LOGIN, PASSWORD, replica.sql.uri)
replica.admin("while db.space['_priv']:len() < 1 do require('fiber').sleep(0.01) end")
replica.sql.py_con.authenticate(LOGIN, PASSWORD)

master.admin("s = db.schema.space.create('tweedledum', {id = 0})")
master.admin("index = s:create_index('primary', {type = 'hash'})")

### gh-343: replica.cc must not add login and password to proc title
#status = replica.get_param("status")
#host_port = "%s:%s" % master.sql.uri
#m = re.search(r'replica/(.*)/.*', status)
#if not m or m.group(1) != host_port:
#    print 'invalid db.info.status', status, 'expected host:port', host_port

master_id = master.get_param('server')['id']
replica_id = replica.get_param('server')['id']

id = ID_BEGIN
for i in range(REPEAT):
    print "test %d iteration" % i

    # insert to master
    insert_tuples(master, id, id + ID_STEP)
    # select from replica
    replica.wait_lsn(master_id, master.get_lsn(master_id))
    select_tuples(replica, id, id + ID_STEP)
    id += ID_STEP

    # insert to master
    insert_tuples(master, id, id + ID_STEP)
    # select from replica
    replica.wait_lsn(master_id, master.get_lsn(master_id))
    select_tuples(replica, id, id + ID_STEP)
    id += ID_STEP

    print "swap servers"
    # reconfigure replica to master
    replica.rpl_master = None
    print("switch replica to master")
    replica.admin("db.cfg{replication_source=''}")
    # reconfigure master to replica
    master.rpl_master = replica
    print("switch master to replica")
    master.admin("db.cfg{replication_source='%s'}" % replica.uri, silent=True)

    # insert to replica
    insert_tuples(replica, id, id + ID_STEP)
    # select from master
    master.wait_lsn(replica_id, replica.get_lsn(replica_id))
    select_tuples(master, id, id + ID_STEP)
    id += ID_STEP

    # insert to replica
    insert_tuples(replica, id, id + ID_STEP)
    # select from master
    master.wait_lsn(replica_id, replica.get_lsn(replica_id))
    select_tuples(master, id, id + ID_STEP)
    id += ID_STEP

    print "rollback servers configuration"
    # reconfigure replica to master
    master.rpl_master = None
    print("switch master to master")
    master.admin("db.cfg{replication_source=''}")
    # reconfigure master to replica
    replica.rpl_master = master
    print("switch replica to replica")
    replica.admin("db.cfg{replication_source='%s'}" % master.uri, silent=True)


# Cleanup.
replica.stop()
replica.cleanup(True)
server.stop()
server.deploy()
