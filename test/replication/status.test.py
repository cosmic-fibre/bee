import os
import re
import time

from lib.bee_server import BeeServer

# master server
master = server
master.admin("db.schema.user.grant('guest', 'read,write,execute', 'universe')")

# replica server
replica = BeeServer()
replica.script = "replication/replica.lua"
replica.rpl_master = master
replica.vardir = os.path.join(master.vardir, 'replica')
replica.deploy()

replica.get_param('node')

cycles = 0
status = replica.admin.execute_no_reconnect("db.info.status", True)
while (re.search(r'replica/.*/(connecting|connected)\n', status) == None and cycles < 500):
    time.sleep(0.01)
    status = replica.admin.execute_no_reconnect("db.info.status", True)
    cycles += 1
print(re.search(r'replica/.*/(connecting|connected)\n', status) != None)

master.stop()
cycles = 0

while (re.search(r'replica/.*/(connecting|failed)\n', status) == None and cycles < 500):
    time.sleep(0.01)
    status = replica.admin.execute_no_reconnect("db.info.status", True)
    cycles += 1
print(re.search(r'replica/.*/(connecting|failed)\n', status) != None)

# Cleanup.
replica.stop()
replica.cleanup(True)

master.deploy()
