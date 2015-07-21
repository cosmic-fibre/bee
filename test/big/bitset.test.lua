dofile('utils.lua')
dofile('bitset.lua')

create_space()

------------------------------------------------------------------------------
-- BitsetIndex: insert/delete
------------------------------------------------------------------------------
test_insert_delete(128)

------------------------------------------------------------------------------
-- BitsetIndex: ALL
------------------------------------------------------------------------------
clear()
fill(1, 128)
dump(db.index.BITS_ALL)
db.space.tweedledum.index.bitset:count()
------------------------------------------------------------------------------
-- BitsetIndex: BITS_ALL_SET (single bit)
------------------------------------------------------------------------------
dump(db.index.BITS_ALL_SET, 0)
db.space.tweedledum.index.bitset:count(0, { iterator = db.index.BITS_ALL_SET})
dump(db.index.BITS_ALL_SET, 1)
db.space.tweedledum.index.bitset:count(1, { iterator = db.index.BITS_ALL_SET})
dump(db.index.BITS_ALL_SET, 2)
db.space.tweedledum.index.bitset:count(2, { iterator = db.index.BITS_ALL_SET})
dump(db.index.BITS_ALL_SET, 8)
db.space.tweedledum.index.bitset:count(8, { iterator = db.index.BITS_ALL_SET})
dump(db.index.BITS_ALL_SET, 128)
db.space.tweedledum.index.bitset:count(128, { iterator = db.index.BITS_ALL_SET})
dump(db.index.BITS_ALL_SET, 1073741824)
db.space.tweedledum.index.bitset:count(1073741824, { iterator = db.index.BITS_ALL_SET})
dump(db.index.BITS_ALL_SET, 2147483648)
db.space.tweedledum.index.bitset:count(2147483648, { iterator = db.index.BITS_ALL_SET})

------------------------------------------------------------------------------
-- BitsetIndex: BITS_ALL_SET (multiple bit)
------------------------------------------------------------------------------
dump(db.index.BITS_ALL_SET, 3)
dump(db.index.BITS_ALL_SET, 7)
dump(db.index.BITS_ALL_SET, 31)
dump(db.index.BITS_ALL_SET, 5)
dump(db.index.BITS_ALL_SET, 10)
dump(db.index.BITS_ALL_SET, 27)
dump(db.index.BITS_ALL_SET, 341)
dump(db.index.BITS_ALL_SET, 2147483649)
dump(db.index.BITS_ALL_SET, 4294967295)
------------------------------------------------------------------------------
-- BitsetIndex: BITS_ALL_NOT_SET (single bit)
------------------------------------------------------------------------------
dump(db.index.BITS_ALL_NOT_SET, 0)
db.space.tweedledum.index.bitset:count(0, { iterator = db.index.BITS_ALL_NOT_SET})
dump(db.index.BITS_ALL_NOT_SET, 2)
db.space.tweedledum.index.bitset:count(2, { iterator = db.index.BITS_ALL_NOT_SET})
dump(db.index.BITS_ALL_NOT_SET, 8)
db.space.tweedledum.index.bitset:count(8, { iterator = db.index.BITS_ALL_NOT_SET})
dump(db.index.BITS_ALL_NOT_SET, 128)
db.space.tweedledum.index.bitset:count(128, { iterator = db.index.BITS_ALL_NOT_SET})
dump(db.index.BITS_ALL_NOT_SET, 1073741824)
db.space.tweedledum.index.bitset:count(1073741824, { iterator = db.index.BITS_ALL_NOT_SET})
dump(db.index.BITS_ALL_NOT_SET, 2147483648)
db.space.tweedledum.index.bitset:count(2147483648, { iterator = db.index.BITS_ALL_NOT_SET})

------------------------------------------------------------------------------
-- BitsetIndex: BITS_ALL_NOT_SET (multiple bit)
------------------------------------------------------------------------------
dump(db.index.BITS_ALL_NOT_SET, 3)
db.space.tweedledum.index.bitset:count(3, { iterator = db.index.BITS_ALL_NOT_SET})
dump(db.index.BITS_ALL_NOT_SET, 7)
db.space.tweedledum.index.bitset:count(7, { iterator = db.index.BITS_ALL_NOT_SET})
dump(db.index.BITS_ALL_NOT_SET, 10)
db.space.tweedledum.index.bitset:count(10, { iterator = db.index.BITS_ALL_NOT_SET})
dump(db.index.BITS_ALL_NOT_SET, 27)
db.space.tweedledum.index.bitset:count(27, { iterator = db.index.BITS_ALL_NOT_SET})
dump(db.index.BITS_ALL_NOT_SET, 85)
db.space.tweedledum.index.bitset:count(85, { iterator = db.index.BITS_ALL_NOT_SET})
dump(db.index.BITS_ALL_NOT_SET, 4294967295)
db.space.tweedledum.index.bitset:count(4294967295, { iterator = db.index.BITS_ALL_NOT_SET})

------------------------------------------------------------------------------
-- BitsetIndex: BITS_ANY_SET (single bit)
------------------------------------------------------------------------------
dump(db.index.BITS_ANY_SET, 0)
db.space.tweedledum.index.bitset:count(0, { iterator = db.index.BITS_ANY_SET})
dump(db.index.BITS_ANY_SET, 16)
db.space.tweedledum.index.bitset:count(16, { iterator = db.index.BITS_ANY_SET})
dump(db.index.BITS_ANY_SET, 128)
db.space.tweedledum.index.bitset:count(128, { iterator = db.index.BITS_ANY_SET})

------------------------------------------------------------------------------
-- BitsetIndex: BITS_ANY_SET (multiple bit)
------------------------------------------------------------------------------
dump(db.index.BITS_ANY_SET, 7)
dump(db.index.BITS_ANY_SET, 84)
dump(db.index.BITS_ANY_SET, 113)

drop_space()
