#!/usr/bin/env bee

--
-- Check that Bee creates configuration accepts boolean values
-- 
db.cfg{logger_nonblock=true,
    panic_on_wal_error=true,
    slab_alloc_arena=0.1,
    logger="bee.log"
}
print(db.cfg.logger_nonblock)
print(db.cfg.panic_on_wal_error)
os.exit()
