import os
import yaml

#print """
#A test case for https://bugs.launchpad.net/bee/+bug/1052018
#panic_on_wal_error doesn't work for duplicate key errors
#"""

server.stop()
server.deploy()
lsn = int(yaml.load(server.admin("db.info.server.lsn", silent=True))[0])
filename = str(lsn).zfill(20) + ".xlog"
wal_old = os.path.join(server.vardir, "old_" + filename)
wal = os.path.join(server.vardir, filename)

# Create wal#1
server.admin("space = db.schema.space.create('test')")
server.admin("index = db.space.test:create_index('primary')")
server.admin("db.space.test:insert{1, 'first tuple'}")
server.admin("db.space.test:insert{2, 'second tuple'}")
server.stop()

# Save wal #1
if os.access(wal, os.F_OK):
    print ".xlog exists"
    os.rename(wal, wal_old)

lsn += 4

# Create another wal#1
server.start()
server.admin("space = db.schema.space.create('test')")
server.admin("index = db.space.test:create_index('primary')")
server.admin("db.space.test:insert{1, 'first tuple'}")
server.admin("db.space.test:delete{1}")
server.stop()

# Create wal#2
server.start()
server.admin("db.space.test:insert{1, 'third tuple'}")
server.admin("db.space.test:insert{2, 'fourth tuple'}")
server.stop()

if os.access(wal, os.F_OK):
    print ".xlog exists"
    # Replace wal#1 with saved copy
    os.unlink(wal)
    os.rename(wal_old, wal)


server.start()
line = 'Duplicate key'
print "check log line for '%s'" % line
print
if server.logfile_pos.seek_once(line) >= 0:
    print "'%s' exists in server log" % line
print

server.admin("db.space.test:get{1}")
server.admin("db.space.test:get{2}")
server.admin("db.space.test:len()")

