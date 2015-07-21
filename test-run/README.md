# Bee Functional testing framework

### Test Suite
Bunch of tests, that lay down in the subfolder (recursively) with `suite.ini` file. `suite.ini` is basic ini-file, that consists of one section `default`, and a number of fields:
* `core`
* `description` - Test Suite description
* `script` - shebang file to start bee with
* disables:
    * `disabled` - tests that must be skipped
    * `release_disabled` - tests that must be skipped when Bee has been builded with `Release`
    * `valgrind_disabled` - tests that must be skipped when Valgrind is enabled
* `lua_libs` - paths for lua files, that should be copied into the folder, where server is started (delimited with the space, e.g. `lua_libs=lua/1.lua lua/2.lua`)
* `long_run` - mark tests as long, enabled only with `--long` option (delimited with the space, e.g. `long_run=t1.test.lua t2.test.lua`)

Field `core` must be one of:
* `bee` - Test-Suite for Functional Testing
* `app` - Another functional Test-Suite
* `unit` - Unit-Testing Test Suite

### Test
Each test consists of files `*.test(.lua|.py)?`, `*.result`, and may have skip condition file `*.skipcond`.
On first run (without `.result`) `.result` is generated from output.  
Each run, in the beggining, `.skipcond` file is executed. In the local env there's object `self`, that's `Test` object. If test must be skipped - you must put `self.skip = 1` in this file. Next, `.test(.lua|.py)?` is executed and file `.reject` is created, then `.reject` is compared with `.result`. If something differs, then 15 last string of this diff file are printed and `.reject` file is saving in the folder, where `.result` file is. If not, then `.reject` is deleted.

#### Python
Files: `<name>.test.py`, `<name>.result` and `<name>.skipcond`(optionaly).
Environment:
* `sql` - `DbConnection` class. Convert our subclass of SQL into IProto query and then decode it. Print into `.result` in YAML. Examples:
    * `sql("select * from t<space> where k<key>=<string|number>[ limit <number>]")`
    * `sql("insert into t<space> values ([<string|number> [, <string|number>]*])")`
    * `sql("delete from t<space> where k<key>=<string|number>")`
    * `sql("call <proc_name>([string|number]*)")`
    * `sql("update t<space> set [k<field>=<string|number> [, k<field>=<string|number>]*] where k<key>=<string|number>"")`
    * `sql("ping")`
* `admin` - `AdminConnection` - simply send admin query on admin port (LUA), then, receive answer. Examples
    * `admin('db.info')`

**Example:**
```python
import os
import time

from lib.admin_connection import AdminConnection
from lib.bee_server import BeeServer

master = server
admin("db.info.lsn") # equivalent to master.admin("db.info.lsn") and server.admin(...)
sql("select * from t0 where k0=1")
replica = BeeServer()
replica.script = 'replication/replica.lua'
replica.vardir = os.path.join(server.vardir, "replica")
replica.deploy()
master.admin("db.insert(0, 1, 'hello')")
print('sleep_1')
time.sleep(0.1)
print('sleep_finished')
print('sleep_2')
admin("require('fiber').sleep(0.1)")
print('sleep_finished')
replica.admin("db.select(0, 0, 1)")
con2 = AdminConnection('localhost', server.admin.port)
con2("db.info.lsn")
replica.stop()
replica.cleanup(True)
con2.disconnect()
```

**Result:**
```yaml
db.info.lsn
---
- null
...
select * from t0 where k0=1
---
- error:
    errcode: ER_NO_SUCH_SPACE
    errmsg: Space '#0' does not exist
...
db.insert(0, 1, 'hello')
---
- error: '[string "return db.insert(0, 1, ''hello'')"]:1: attempt to call field ''insert''
    (a nil value)'
...
sleep_1
sleep_finished
sleep_2
require('fiber').sleep(0.1)
---
...
sleep_finished
db.select(0, 0, 1)
---
- error: '[string "return db.select(0, 0, 1)"]:1: attempt to call field ''select''
    (a nil value)'
...
db.info.lsn
---
- null
...
```

#### Lua
Files: `<name>.test.lua`, `<name>.result` and `<name>.skipcond`(optionaly).
Tests interact only with `AdminConnection`. Supports some preprocessor functions (eg `delimiter`)

**Delimiter example:**
```
db.schema.space.create('temp')
t1 = db.space.temp
t1:create_index('primary', { type = 'hash', parts = {1, 'num'}, unique = true})
t1:insert{0, 1, 'hello'}
--# setopt delimiter ';'
function test()
    return {1,2,3}
end;
test(
);
--# setopt delimiter ''
test(
);
test

```

**Delimiter result:**
```
db.schema.space.create('temp')
---
- index: []
  on_replace: 'function: 0x40e4fdf0'
  temporary: false
  id: 512
  engine: memtx
  enabled: false
  name: temp
  field_count: 0
- created
...
t1 = db.space.temp
---
...
t1:create_index('primary', { type = 'hash', parts = {1, 'num'}, unique = true})
---
...
t1:insert{0, 1, 'hello'}
---
- [0, 1, 'hello']
...
--# setopt delimiter ';' -- !!COMMENTARY - delimiter is changed to ';\n'
function test()
    return {1,2,3}
end;
---
...
test(
);
---
- - 1
  - 2
  - 3
...
--# setopt delimiter '' -- !!COMMENTARY - delimiter is changed back - to '\n'
test(
---
- error: '[string "test( "]:1: unexpected symbol near ''<eof>'''
...
);
---
- error: '[string "); "]:1: unexpected symbol near '')'''
...
test
---
- 'function: 0x40e533b8'
...
```

##### Preprocessor description
prefix `--#` means that this line must processed to preprocessor

__Base directives:__
* `--# setopt delimiter '<delimiter>'` - Sets delimiter to <delimiter>\n

__Server directives:__
* `--# create server <name> with ...` - Create server with name <name>, where `...` may be:
    * `script = '<path>'` - script to start
    * `rpl_master = <server>` - replication master server name
* `--# start server <name>` - Run server <name>
* `--# stop server <name> ` - Stop server <name> 
* `--# cleanup server <name> ` - Cleanup (basically after server has been stopped)

__Connection directives:__
* `--# create connection <name-con> to <name-serv>` - create connection named <name-con> to <name-serv> server
* `--# drop connection <name>` - Turn connection <name> off and delete it
* `--# set connection <name>` - Set connection <name> to be main, for next commands

__Filter directives:__
* `--# push filter '<regexp_from>' to '<regexp_to>'` - e,g, `--# push filter 'listen: .*' to 'listen: <uri>'`

__Set variables:__
* `--# set variables '<variable_name>' to '<where>'` - execute `<variable_name> = *<where>` where *<where> is value of where. Where must be
    * `<server_name>.admin` - admin port of this server
    * `<server_name>.master` - listen port of master of this replica
    * `<server_name>.listen` - listen port of this server
