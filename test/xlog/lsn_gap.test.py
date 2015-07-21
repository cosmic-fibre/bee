import os
import yaml
#
# gh-167: Replica can't find next xlog file if there is a gap in LSN
#

server.stop()
server.deploy()

# Create wal#1
server.admin("space = db.schema.space.create('test')")
server.admin("index = db.space.test:create_index('primary')")
server.admin("db.space.test:insert{1, 'first tuple'}")
server.admin("db.space.test:insert{2, 'second tuple'}")
lsn = int(yaml.load(server.admin("db.info.server.lsn", silent=True))[0])
wal = os.path.join(server.vardir, str(lsn).zfill(20) + ".xlog")
server.stop()
server.start()
server.admin("db.space.test:insert{3, 'third tuple'}")
server.stop()
server.start()
server.admin("db.space.test:insert{4, 'fourth tuple'}")
server.stop()

# Remove xlog with {3, 'third tuple'}
os.unlink(wal)

server.start()
line="ignoring missing WAL"
print "check log line for '%s'" % line
print
if server.logfile_pos.seek_once(line) >= 0:
    print "'%s' exists in server log" % line
print

# missing tuple from removed xlog
server.admin("db.space.test:select{}")

