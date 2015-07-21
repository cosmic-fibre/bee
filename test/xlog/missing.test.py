import yaml
import os 
#
# gh-716: infinite loop at start if missing xlog
#

server.stop()
server.deploy()

# Create wal#1
server.admin("space = db.schema.space.create('test')")
server.admin("index = db.space.test:create_index('primary')")
server.stop()
server.start()
# these inserts will be in their own xlog, which then will
# get "lost"
lsn = int(yaml.load(server.admin("db.info.server.lsn", silent=True))[0])
wal = os.path.join(server.vardir, str(lsn).zfill(20) + ".xlog")
server.admin("db.space.test:insert{1, 'first tuple'}")
server.admin("db.space.test:insert{2, 'second tuple'}")
server.admin("db.space.test:insert{3, 'third tuple'}")
server.stop()
server.start()
# put deletes in their own xlog
server.admin("db.space.test:delete{1}")
server.admin("db.space.test:delete{2}")
server.admin("db.space.test:delete{3}")
server.stop()

# Remove xlog with inserts
os.unlink(wal)
# bee doesn't issue an LSN for deletes which delete nothing
# this may lead to infinite recursion at start
server.start()
line="ignoring missing WAL"
print "check log line for '%s'" % line
print
if server.logfile_pos.seek_once(line) >= 0:
    print "'%s' exists in server log" % line
print

# missing tuples from removed xlog
server.admin("db.space.test:select{}")
server.admin("db.space.test:drop()")
