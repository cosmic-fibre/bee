import os
import yaml

from os.path import abspath

# cleanup server.vardir
server.stop()
server.deploy()
lsn = int(yaml.load(server.admin("db.info.server.lsn", silent=True))[0])
server.stop()

print """
# Inprogress xlog must be renamed before second insert.
"""
filename = str(lsn).zfill(20) + ".xlog"
wal_inprogress = os.path.join(server.vardir, filename + ".inprogress")
wal = os.path.join(server.vardir, filename)

server.start()

server.admin("space = db.schema.space.create('tweedledum', { id = 0 })")
if os.access(wal_inprogress, os.F_OK):
  print ".xlog.inprogress exists"

server.admin("index = space:create_index('primary', { type = 'hash' })")

if os.access(wal, os.F_OK) and not os.access(wal_inprogress, os.F_OK):
  print ".xlog.inprogress has been successfully renamed"
server.stop()
lsn += 2

print """
# Inprogress xlog must be renamed during regular termination.
"""
filename = str(lsn).zfill(20) + ".xlog"
server.start()

wal_inprogress = os.path.join(server.vardir, filename + ".inprogress")
wal = os.path.join(server.vardir, filename)

server.admin("db.space[0]:insert{3, 'third tuple'}")

if os.access(wal_inprogress, os.F_OK):
  print ".xlog.inprogress exists"

server.stop()

if os.access(wal, os.F_OK) and not os.access(wal_inprogress, os.F_OK):
  print ".xlog.inprogress has been successfully renamed"
lsn += 1

print """
# An inprogress xlog file with one record must be renamed during recovery.
"""

server.start()
filename = str(lsn).zfill(20) + ".xlog"
wal_inprogress = os.path.join(server.vardir, filename + ".inprogress")
wal = os.path.join(server.vardir, filename)
server.admin("db.space[0]:insert{4, 'fourth tuple'}")
server.admin("db.space[0]:insert{5, 'Unfinished record'}")
pid = int(yaml.load(server.admin("require('bee').pid()", silent=True))[0])
from signal import SIGKILL
if pid > 0:
    os.kill(pid, SIGKILL)
server.stop()

if os.access(wal, os.F_OK):
    print ".xlog exists"
    # Remove last byte from xlog
    f = open(wal, "a")
    size = f.tell()
    f.truncate(size - 1)
    f.close()
    os.rename(wal, wal_inprogress)

server.start()

if os.access(wal, os.F_OK) and not os.access(wal_inprogress, os.F_OK):
  print ".xlog.inprogress hash been successfully renamed"
server.stop()
lsn += 1

# print """
# # Empty (header only, no records) inprogress xlog must be deleted
# # during recovery.
# """
#
# # If the previous test has failed, there is a dangling link
# # and symlink fails.
# try:
#   os.symlink(abspath("db/just_header.xlog"), wal_inprogress)
# except OSError as e:
#   print e
#
# server.start()
#
# if not os.access(wal_inprogress, os.F_OK) and not os.access(wal, os.F_OK):
#    print "00000000000000000006.xlog.inprogress has been successfully deleted"
# server.stop()

# print """
# # Inprogress xlog with bad record must be deleted during recovery.
# """
#
# # If the previous test has failed, there is a dangling link
# # and symlink fails.
# try:
#   os.symlink(abspath("db/bad_record.xlog"), wal_inprogress)
# except OSError as e:
#   print e
#
# server.start()
#
# if not os.access(wal_inprogress, os.F_OK) and not os.access(wal, os.F_OK):
#    print "00000000000000000006.xlog.inprogress has been successfully deleted"

server.start()
orig_lsn = int(yaml.load(admin("db.info.server.lsn", silent=True))[0])

# create .snap.inprogress
admin("db.snapshot()")
admin("db.space._schema:insert({'test', 'test'})")
admin("db.snapshot()")
lsn = int(yaml.load(admin("db.info.server.lsn", silent=True))[0])
snapshot = str(lsn).zfill(20) + ".snap"
snapshot = os.path.join(server.vardir, snapshot)
server.stop()
os.rename(snapshot, snapshot + ".inprogress")
# remove .xlogs
for f in os.listdir(server.vardir):
    if f.endswith(".xlog"):
        os.remove(os.path.join(server.vardir, f))

# check that .snap.inprogress is ignored during scan
server.start()
lsn = int(yaml.load(admin("db.info.server.lsn", silent=True))[0])
if lsn == orig_lsn:
    print ".snap.inprogress is ignored"
