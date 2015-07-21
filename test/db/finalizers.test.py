import os
import sys
import re
import yaml
from lib.bee_server import BeeServer

server = BeeServer(server.ini)
server.script = 'db/lua/finalizers.lua'
server.vardir = os.path.join(server.vardir, 'finalizers')
try:
    server.deploy()
except:
    print "Expected error:", sys.exc_info()[0]
else:
    print "Error! exception did not occur"


