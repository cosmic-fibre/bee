
import sys
import yaml

server_uuid = server.get_param('server')['uuid']
sys.stdout.push_filter(server_uuid, '<server uuid>')
cluster_uuid = yaml.load(server.admin('db.space._schema:get("cluster")',
    silent = True))[0][1]
sys.stdout.push_filter(cluster_uuid, '<cluster uuid>')
sys.stdout.push_filter(server.sourcedir, '<sourcedir>')

server.admin('dofile("%s/extra/schema_erase.lua")' % server.sourcedir)
server.admin('db.space._schema:select{}')
server.admin('db.space._cluster:select{}')
server.admin('db.space._space:select{}')
server.admin('db.space._index:select{}')
server.admin('db.space._user:select{}')
server.admin('db.space._func:select{}')
server.admin('db.space._priv:select{}')

server.admin('dofile("%s/extra/schema_fill.lua")' % server.sourcedir)
server.admin("db.snapshot()")
server.restart()

server.admin('db.space._schema:select{}')
server.admin('db.space._cluster:select{}')
server.admin('db.space._space:select{}')
server.admin('db.space._index:select{}')
server.admin('db.space._user:select{}')
server.admin('db.space._func:select{}')
server.admin('db.space._priv:select{}')

# Cleanup
sys.stdout.pop_filter()
