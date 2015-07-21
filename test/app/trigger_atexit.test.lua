#!/usr/bin/env bee
-- vim: set ft=lua :

-- see https://github.com/bee/bee/issues/583

tap = require 'tap'
fio = require 'fio'
log = require 'log'

tempdir = fio.tempdir()

db.cfg {
    wal_dir = tempdir,
    snap_dir = tempdir,
    sham_dir = tempdir,
    logger      = fio.pathjoin(tempdir, 'bee.log'),
    slab_alloc_arena=0.1 -- for small systems
}

local function test_replace(old_tuple, new_tuple)

end


db.schema.space.create('abc')
db.space.abc:create_index('pk', { type = 'tree' })
db.space.abc:on_replace(test_replace)


cleanup_list = fio.glob(fio.pathjoin(tempdir, '*'))
for _, file in pairs(cleanup_list) do
    fio.unlink(file)
end
fio.rmdir(tempdir)
print("done")
os.exit(0)

