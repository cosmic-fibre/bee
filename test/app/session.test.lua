#!/usr/bin/env bee

--
-- Check that Bee creates ADMIN session for #! script
--
db.cfg{logger="bee.log", slab_alloc_arena=0.1}
print('session.id()', db.session.id())
print('session.uid()', db.session.uid())
os.exit(0)
