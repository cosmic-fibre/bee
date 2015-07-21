*******************************************************************************
                              Database
*******************************************************************************

This chapter describes how Bee stores values
and what operations with data it supports. 

==================
Document data model
==================

If you tried out the
:ref:`Starting Bee and making your first database <first database>`
exercise from the last chapter, then your database looks like this:

.. code-block:: none

   +--------------------------------------------+
   |                                            |
   | SPACE 'tester'                             |
   | +----------------------------------------+ |
   | |                                        | |
   | | TUPLE SET 'tester'                     | |
   | | +-----------------------------------+  | |
   | | | Tuple: [ 1 ]                      |  | |
   | | | Tuple: [ 2, 'Music' ]             |  | |
   | | | Tuple: [ 3, 'length', 93 ]        |  | |
   | | +-----------------------------------+  | |
   | |                                        | |
   | | INDEX 'primary'                        | |
   | | +-----------------------------------+  | |
   | | | Key: 1                            |  | |
   | | | Key: 2                            |  | |
   | | | Key: 3                            |  | |
   | | +-----------------------------------+  | |
   | |                                        | |
   | +----------------------------------------+ |
   +--------------------------------------------+

-----
Space
-----

A *space* -- 'tester' in the example -- is a container.

When Bee is being used to store data, there
is always at least one space. There can be many spaces.
Each space has a unique name specified by the user.
Each space has a unique numeric identifier which can
be specified by the user but usually is assigned
automatically by Bee. Spaces always
contain one tuple set and one or more indexes.

---------
Tuple Set
---------

A *tuple set* -- 'tester' in the example -- is a group of tuples.

There is always one tuple set in a space.
The identifier of a tuple set is the same
as the space name -- 'tester' in the example.

A tuple fills the same role as a “row” or a “record”,
and the components of a tuple (which we call “fields”)
fill the same role as a “row column” or “record field”,
except that: the fields of a tuple can be composite
structures, such as arrays or maps and don't need to have names.
That's why there was no need to pre-define the tuple set
when creating the space, and that's why each tuple can
have a different number of elements.
Tuples are stored as `MsgPack`_ arrays.

.. _MsgPack: https://en.wikipedia.org/wiki/MessagePack

Any given tuple may have any number of fields and
the fields may have a variety of types. The identifier
of a field is the field's number, base 1. For example
“1” can be used in some contexts to refer to the first field of a tuple.

When Bee returns a tuple value, it surrounds
strings with single quotes, separates fields with commas,
and encloses the tuple inside square brackets.
For example: [ 3, 'length', 93 ]. 

.. _db.index:

-----
Index
-----

An *index* -- 'primary' in the example -- is a group of key values and pointers.

In order for a tuple set to be useful, there must always
be at least one index in a space. There can be many.
As with spaces, the user can and should specify the index name,
and let Bee come up with a unique numeric identifier
(the "index id").
In our example there is one index and its name is “primary”.

An index may be *multi-part*, that is, the user can declare
that an index key value is taken from two or more fields
in the tuple, in any order. An index may be *unique*, that is,
the user can declare that it would be illegal to have the
same key value twice. An index may have *one of four types*:
HASH which is fastest and uses the least memory but must
be unique, TREE which allows partial-key searching and ordered
results, BITSET which can be good for searches that contain
'=' and multiple ANDed conditions, and RTREE for spatial coordinates.
The first index is called the “*primary key*” index and it must be unique;
all other indexes are called “secondary” indexes.

An index definition may include identifiers of tuple fields
and their expected types. The allowed types for indexed fields are NUM
(64-bit unsigned integer between -9,223,372,036,854,775,807 and
18,446,744,073,709,551,615), or STR (string, any sequence of octets), or ARRAY
(a series of numbers for use with :ref:`RTREE indexes <RTREE>`.
Take our example, which has the request:

  i = s:create_index('primary', {type = 'hash', parts = {1, 'NUM'}})

The effect is that, for all tuples in tester,
field number 1 must exist and must be a 64-bit integer.

Space definitions and index definitions are stored permanently
in system spaces. It is possible to add, drop, or alter the
definitions at runtime, with some restrictions.
The syntax details for defining spaces and indexes
are in section :ref:`The db library <db-library>`. 

----------
Data types
----------

Bee can work with numbers, strings, booleans, tables, and userdata.

+--------------+-------------+----------------------------+------------------------+
|  General type|Specific type|What Lua type()|would return|Example                 |
+==============+=============+============================+========================+
|   scalar     |  number     |   "`number`_"              |      12345             |
+--------------+-------------+----------------------------+------------------------+
|   scalar     |  string     |   "`string`_"              |       'A B C'          |
+--------------+-------------+----------------------------+------------------------+
|   scalar     |  boolean    |   "`boolean`_"             |       true             |
+--------------+-------------+----------------------------+------------------------+
|   scalar     |  nil        |   "`nil`_"                 |       nil              |
+--------------+-------------+----------------------------+------------------------+
|   compound   |  Lua table  |   "`table`_"               |       table: 0x410f8b10|
+--------------+-------------+----------------------------+------------------------+
|   compound   |  tuple      |   "`Userdata`_"            |       12345: {'A B C'} |
+--------------+-------------+----------------------------+------------------------+

.. _number: http://www.lua.org/pil/2.3.html
.. _string: http://www.lua.org/pil/2.4.html
.. _boolean: http://www.lua.org/pil/2.2.html
.. _nil: http://www.lua.org/pil/2.1.html
.. _table: http://www.lua.org/pil/2.5.html
.. _userdata: http://www.lua.org/pil/28.1.html

In Lua a "number" is double-precision floating-point;
however, for database storage Bee uses MsgPack rules,
and MsgPack allows for both integer and floating-point values.
So Bee will store a number as a float if the value
contains a decimal point, and otherwise will store as an integer.
Bee can store signed numbers, but not in indexed fields
-- when a field has a 'NUM' index, the values must be unsigned
64-bit integers. Large numbers can be entered with exponential
notation, for example 9e99. Large integers greater than
100,000,000,000,000 (1e14) should be entered with the
:func:`tonumber64 <tonumber64>`
function. Storage is variable-length, so the smallest number
requires only one byte but the largest number requires nine bytes.

A "string" is a variable-length sequence of bytes,
usually represented with alphanumeric characters inside single quotes.

A "boolean" is either true or false.

A "nil" type has only one possible value, also called "nil",
but often displayed as "null".
Nils may be compared to values of any types with == (is-equal)
or ~= (is-not-equal), but other operations will not work.
Nils may not be used in Lua tables; the workaround
is to use :ref:`yaml.NULL <yaml-null>` or
:ref:`json.NULL <json-null>` or :ref:`msgpack.NULL <msgpack-null>`.

A tuple is returned in YAML format like - [120, 'a', 'b', 'c'].
A few functions may return tables with multiple tuples.
A scalar may be converted to a tuple with only one field.
A Lua table may contain all of a tuple's fields, but not nil.
For more tuple examples see :ref:`db.tuple <db-tuple>`.

----------
Operations
----------

The basic operations are: the four data-change operations
(insert, update, delete, replace), and the data-retrieval
operation (select). There are also minor operations like
“ping” which can only be used with the binary protocol.
Also, there are :ref:`index iterator <index-iterator>` operations, which can only
be used with Lua code. (Index iterators are for traversing
indexes one key at a time, taking advantage of features
that are specific to an index type, for example evaluating
Boolean expressions when traversing BITSET indexes, or
going in descending order when traversing TREE indexes.)

Five examples of basic operations:

.. code-block:: bash

   -- Add a new tuple to tuple set tester.
   -- The first field, field[1], will be 999 (type is NUM).
   -- The second field, field[2], will be 'Taranto' (type is STR).
   db.space.tester:insert{999, 'Taranto'}

   -- Update the tuple, changing field field[2].
   -- The clause "{999}", which has the value to look up in
   -- the index of the tuple's primary-key field, is mandatory
   -- because update() requests must always have a clause that
   -- specifies the primary key, which in this case is field[1].
   -- The clause "{{'=', 2, 'Tarantino'}}" specifies that assignment
   -- will happen to field[2] with the new value.
   db.space.tester:update({999}, {{'=', 2, 'Tarantino'}})

   -- Replace the tuple, adding a new field.
   -- This is also possible with the update() request but
   -- the update() request is usually more complicated.
   db.space.tester:replace{999, 'Tarantella', 'Tarantula'}

   -- Retrieve the tuple.
   -- The clause "{999}" is still mandatory, although it does not have to
   -- mention the primary key. */
   db.space.tester:select{999}

   -- Delete the tuple.
   -- Once again the clause to identify the primary-key field is mandatory.
   db.space.tester:delete{999}

How does Bee do a basic operation? Let's take this example:
   db.space.tester:update({3}, {{'=', 2, 'size'}, {'=', 3, 0}})
which, for those who know SQL, is equivalent to a statement like
   UPDATE tester SET "field[2]" = 'size', "field[3]" = 0 WHERE "field[[1]" = 3

STEP #1: if this is happening on a remote client,
then the client parses the statement and changes
it to a binary-protocol instruction which has already
been checked, and which the server can understand without
needing to parse everything again. The client ships a packet to the server.

STEP #2: the server's “transaction processor” thread uses
the primary-key index on field[1] to find the location
of the tuple in memory. It determines that the tuple can
be updated (not much can go wrong when you're merely
changing an unindexed field value to something shorter).

STEP #3: the transaction processor thread sends a message
to the write-ahead logging (WAL) thread.

At this point a “yield” takes place. To know the significance
of that -- and it's quite significant -- you have to know a few
facts and a few new words.

FACT #1: there is only one transaction processor thread.
Some people are used to the idea that there can be multiple
threads operating on the database, with (say) thread #1
reading row #x while thread#2 writes row#y. With Bee
no such thing ever happens. Only the transaction processor
thread can access the database, and there is only one
transaction processor thread for each instance of the server.

FACT #2: the transaction processor thread can handle many fibers.
A fiber is a set of computer instructions that may contain
“yield” signals. The transaction processor thread will execute
all computer instructions until a yield, then switch to execute
the instructions of a different fiber. Thus (say) the thread reads
row#x for the sake of fiber#1, then writes row#y for the sake of fiber#2.

FACT #3: yields must happen, otherwise the transaction processor
thread would stick permanently on the same fiber.
There are implicit yields: every data-change operation
or network-access causes an implicit yield, and every
statement that goes through the bee client causes
an implicit yield. And there are explicit yields:
in a Lua function one can and should add “yield” statements
to prevent hogging. This is called cooperative multitasking.

Since all data-change operations end with an implicit yield
and an implicit commit, and since no data-change operation
can change more than one tuple, there is no need for any locking.
Consider, for example, a Lua function that does three Bee operations:|br|
s:select{999}                -- this does not yield and does not commit |br|
s:update({...},{{...}})      -- this yields and commits |br|
s:select{999}                -- this does not yield and does not commit |br|
The combination “SELECT plus UPDATE” is an atomic transaction:
the function holds a consistent view of the database until the UPDATE ends.
For the combination “UPDATE plus SELECT” the view is not consistent,
because after the UPDATE the transaction processor thread can switch
to another fiber, and delete the tuple that was just updated.

Since locks don't exist, and disk writes only involve the write-ahead log,
transactions are usually fast. Also the Bee server may not be using
up all the threads of a powerful multi-core processor, so advanced users
may be able to start a second Bee server on the same processor without ill effects.

Additional examples of requests can be found in the `Bee regression test suite`_.
A complete grammar of supported data-manipulation functions will come later in this chapter.

.. _Bee regression test suite: https://github.com/bee/bee/tree/master/test/db

Since not all Bee operations can be expressed with the
data-manipulation functions, or with Lua, to gain complete access
to data manipulation functionality one must use a
:ref:`Perl, PHP, Python or other programming language connector <db-connectors>`.
The client/server protocol is open and documented:
an annotated BNF can be found in the source tree, file
`doc/db-protocol.html`_.

.. _doc/db-protocol.html: http://bee.org/doc/db-protocol.html

----------------
Data persistence
----------------

To maintain data persistence, Bee writes each data
change request (INSERT, UPDATE, DELETE, REPLACE) into
a write-ahead log (WAL) file in the :confval:`wal_dir <wal_dir>` directory.
A new WAL file is created for every :confval:`rows_per_wal <rows_per_wal>` records.
Each data change request gets assigned a continuously growing
64-bit log sequence number. The name of the WAL file is based
on the log sequence number of the first record in the file,
plus an extension .xlog.

Apart from a log sequence number and the data change request
(its format is the same as in the binary protocol and is described
in `doc/db-protocol.html`_), each WAL record contains a header,
some metadata, and then the data formatted according to msgpack rules.
For example this is what the WAL file looks like after the first INSERT
request ("s:insert({1})") for the introductory sanddb exercise
":ref:`Starting Bee and making your first database <first database>`“.
On the left are the hexadecimal bytes that one would see with

$ hexdump 00000000000000000001.xlog

and on the right are comments.

.. code-block:: none

   Hex dump of WAL file       Comment
   --------------------       -------
   58 4c 4f 47 0a             File header: "XLOG\n"
   30 2e 31 32 0a 0a          File header: "0.12\n\n" = version
   ...                        (not shown = inserted tuples for LSN and system spaces)
   d5 ba 0b ab                Magic row marker always = 0xab0bbad5 if version 0.12
   19 00                      Length, not including length of header, = 25 bytes
   ce 16 a4 38 6f             Record header: previous crc32, current crc32,
   a7 cc 73 7f 00 00 66 39
   84                         msgpack code meaning "Map of 4 elements" follows
   00 02 02 01                the 4 elements, including 0x02 which is IPROTO_INSERT
   03 04 04                   additional information
   cb 41 d4 e2 2f 62 fd d5 d4 msgpack code meaning "Double" follows, for next 8 bytes
   82                         msgpack code meaning "map of 2 elements" follows
   10                         IPROTO_SPACE_ID which is #defined as 16 (hex 10)
   cd                         msgpack code meaning 2-digit number follows
   02 00                      the id of "tester" which is 513, it's biggest byte first
   21                         Flags = IPROTO_TUPLE which is #defined as hex 21
   91                         msgpack code meaning "1-element fixed array" follows
   01                         Tuple: field[1] value = 1

Bee processes requests atomically: a change is either
accepted and recorded in the WAL, or discarded completely.
Let's clarify how this happens, using the REPLACE request as an example:

    1. The server attempts to locate the original tuple by primary key.
    If found, a reference to the tuple is retained for later use.

    2. The new tuple is then validated. If for example it does not contain
    an indexed field, or it has an indexed field whose type does not match
    the type according to the index definition, the change is aborted.

    3. The new tuple replaces the old tuple in all existing indexes.

    4. A message is sent to WAL writer running in a separate thread,
    requesting that the change be recorded in the WAL. The server
    switches to work on the next request until the write is acknowledged.

    5. On success, a confirmation is sent to the client. Upon failure,
    a rollback procedure is begun. During the rollback procedure,
    the transaction processor rolls back all changes to the database
    which occurred after the first failed change, from latest to oldest,
    up to the first failed change. All rolled back requests are aborted
    with :errcode:`ER_WAL_IO <ER_WAL_IO>` error. No new change is applied while rollback is
    in progress. When the rollback procedure is finished, the server
    restarts the processing pipeline. 

One advantage of the described algorithm is that complete request
pipelining is achieved, even for requests on the same value of the
primary key. As a result, database performance doesn't degrade even
if all requests touch upon the same key in the same space.

The transaction processor thread communicates with the WAL writer
thread using asynchronous (yet reliable) messaging; the transaction
processor thread, not being blocked on WAL tasks, continues to handle
requests quickly even at high volumes of disk I/O.
A response to a request is sent as soon as it is ready,
even if there were earlier incomplete requests on the same connection.
In particular, SELECT performance, even for SELECTs running
on a connection packed with UPDATEs and DELETEs,
remains unaffected by disk load.

The WAL writer employs a number of durability modes, as defined
in configuration variable :confval:`wal_mode <wal_mode>`. It is possible to turn
the write-ahead log completely off, by setting :confval:`wal_mode <wal_mode>` to *none*.
Even without the write-ahead log it's still possible to take
a persistent copy of the entire data set with the :func:`db.snapshot() <db.snapshot()>` request. 

-----------------
Data manipulation
-----------------

The basic "data-manipulation" requests are:
insert, replace, update, delete, select.
They all are part of the db library.
They all may return data. Usually both inputs and outputs may be Lua tables.

The Lua syntax for data-manipulation functions can vary.
Here are examples of the variations with select examples;
the same rules exist for the other data-manipulation functions.
Every one of the examples does the same thing:
select a tuple set from a space named tester where
the primary-key field value equals 1.

First there are "naming variations":

    1. db.space.tester:select{1}

    2. db.space['tester']:select{1}

    3. db.space[512]:select{1}

    4. variable = 'tester'; db.space[variable]:select{1}

... There is an assumption that the numeric id of
'tester' is 512, which happens to be the case in our
sanddb example only. Literal values such as 'tester'
may be replaced by variable names. Examples and descriptions
in this manual have the "db.space.space.tester:" form;
however, this is a matter of user preference and all
the variants exist in the wild.

Then there are "parameter variations":

    1. db.space.tester:select{1}

    2. db.space.tester:select({1})

    3. db.space.tester:select(1)

    4. db.space.tester:select({1},{iterator='EQ'})

    5. variable = 1; db.space.tester:select{variable}

    6. variable = {1}; db.space.tester:select(variable)

... The primary-key value is enclosed in braces, and if
it was a multi-part primary key then the value would be
multi-part, for example "...select{1,2,3}". The braces
can be enclosed inside parentheses -- "...select({...}) -- which
is optional unless it is necessary to pass something
besides the primary-key value, as in the fourth example.
Literal values such as 1 (a scalar value) or {1} (a Lua table
value) may be replaced by variable names, as in examples
[5] and [6]. Although there are special cases where braces
can be omitted, they are preferable because they signal
"Lua table". Examples and descriptions in this manual
have the "{1}" form; however, this too is a matter of
user preference and all the variants exist in the wild.

All the data-manipulation functions operate on tuple sets but,
since primary keys are unique, the number of tuples in the
tuple set is always 1. The only exception is db.space...select,
which may accept either a primary-key value or a secondary-key value. 

.. _db-library:

---------------
The db library
---------------

As well as executing Lua chunks or defining their own functions, users can exploit
the Bee server's storage functionality with the ``db`` Lua library.

=====================================================================
                     Packages of the db library
=====================================================================

The contents of the db library can be inspected at runtime with ``db``, with
no arguments. The packages inside the db library are:
db.schema, db.tuple, db.space, db.index, net.db, db.cfg, db.info, db.slab, db.stat.
Every package contains one or more Lua functions. A few packages contain
members as well as functions. The functions allow data definition (create
alter drop), data manipulation (insert delete update select replace), and
introspection (inspecting contents of spaces, accessing server configuration).


.. container:: table

    **Complexity Factors that may affect data
    manipulation functions in the db library**

    +-------------------+-----------------------------------------------------+
    | Index size        | The number of index keys is the same as the number  |
    |                   | of tuples in the data set. For a TREE index, if     |
    |                   | there are more keys then the lookup time will be    |
    |                   | greater, although of course the effect is not       |
    |                   | linear. For a HASH index, if there are more keys    |
    |                   | then there is more RAM use, but the number of       |
    |                   | low-level steps tends to remain constant.           |
    +-------------------+-----------------------------------------------------+
    | Index type        | Typically a HASH index is faster than a TREE index  |
    |                   | if the number of tuples in the tuple set is greater |
    |                   | than one.                                           |
    +-------------------+-----------------------------------------------------+
    | Number of indexes | Ordinarily only one index is accessed to retrieve   |
    | accessed          | one tuple. But to update the tuple, there must be N |
    |                   | accesses if the tuple set has N different indexes.  |
    +-------------------+-----------------------------------------------------+
    | Number of tuples  | A few requests, for example select, can retrieve    |
    | accessed          | multiple tuples. This factor is usually less        |
    |                   | important than the others.                          |
    +-------------------+-----------------------------------------------------+
    | WAL settings      | The important setting for the write-ahead log is    |
    |                   | :ref:`wal_mode <wal_mode>`. If the setting causes   |
    |                   | no writing or                                       |
    |                   | delayed writing, this factor is unimportant. If the |
    |                   | settings causes every data-change request to wait   |
    |                   | for writing to finish on a slow device, this factor |
    |                   | is more important than all the others.              |
    +-------------------+-----------------------------------------------------+

In the discussion of each data-manipulation function there will be a note about
which Complexity Factors might affect the function's resource usage.

=====================================================================
            The two storage engines: memtx and sham
=====================================================================

A storage engine is a set of very-low-level routines which actually store and
retrieve tuple values. Bee offers a choice of two storage engines: memtx
(the in-memory storage engine) and sham (the on-disk storage engine).
To specify that the engine should be sham, add a clause: ``engine = 'sham'``.
The manual concentrates on memtx because it is the default and has been around
longer. But sham is a working key-value engine and will especially appeal to
users who like to see data go directly to disk, so that recovery time might be
shorter and database size might be larger. For architectural explanations and
benchmarks, see `sphia.org`_. On the other hand, sham lacks some functions and
options that are available with memtx. Where that is the case, the relevant
description will contain the words "only applicable for the memtx storage engine".

.. _sphia.org: http://sphia.org

.. toctree::
    :maxdepth: 1

    db_schema
    db_space
    db_index
    db_session
    db_error
    db_tuple
    db_introspection
    net_db
    admin
    atomic
    authentication
    limitations
    triggers
