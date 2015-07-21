-------------------------------------------------------------------------------
                            Packages db.cfg, db.info, db.slab, and db.stat: server introspection
-------------------------------------------------------------------------------

=====================================================================
                         Package `db.cfg`
=====================================================================

.. module:: db.cfg

The ``db.cfg`` package is for administrators to specify all the server
configuration parameters; the full description of the parameters is in
section :ref:`book-cfg`. Use ``db.cfg`` without braces to get read-only
access to those parameters.

.. data:: db.cfg

    .. code-block:: lua

        bee> db.cfg
        ---
        - too_long_threshold: 0.5
          slab_alloc_factor: 1.1
          slab_alloc_minimal: 64
          background: false
          slab_alloc_arena: 1
          log_level: 5
          ...
        ...

=====================================================================
                         Package `db.info`
=====================================================================

.. module:: db.info

The ``db.info`` package provides access to information about server variables
-- ``pid``, ``uptime``, ``version`` and others.

**recovery_lag** holds the difference (in seconds) between the current time on
the machine (wall clock time) and the time stamp of the last applied record.
In replication setup, this difference can indicate the delay taking place
before a change is applied to a replica.

**recovery_last_update** is the wall clock time of the last change recorded in
the write-ahead log. To convert it to human-readable time,
you can use **date -d@** 1306964594.980.

**status** is either "primary" or "replica/<hostname>".

.. function:: db.info()

    Since ``db.info`` contents are dynamic, it's not possible to iterate over
    keys with the Lua ``pairs()`` function. For this purpose, ``db.info()``
    builds and returns a Lua table with all keys and values provided in the
    package.

    :return: keys and values in the package.
    :rtype:  table

    .. code-block:: yaml

        bee> db.info()
        ---
        - server:
            lsn: 158
            ro: false
            uuid: 75967704-0115-47c2-9d03-bd2bdcc60d64
            id: 1
          pid: 32561
          version: 1.6.4-411-gcff798b
          snapshot_pid: 0
          status: running
          vclock: {1: 158}
          replication:
            status: off
          uptime: 2778
        ...

.. data:: status
          pid
          version
          ...

    .. code-block:: lua

        bee> db.info.pid
        ---
        - 1747
        ...
        bee> db.info.logger_pid
        ---
        - 1748
        ...
        bee> db.info.version
        ---
        - 1.6.4-411-gcff798b
        ...
        bee> db.info.uptime
        ---
        - 3672
        ...
        bee> db.info.status
        ---
        - running
        ...
        bee> db.info.recovery_lag
        ---
        - 0.000
        ...
        bee> db.info.recovery_last_update
        ---
        - 1306964594.980
        ...
        bee> db.info.snapshot_pid
        ---
        - 0
        ...

=====================================================================
                         Package `db.slab`
=====================================================================

.. module:: db.slab

The ``db.slab`` package provides access to slab allocator statistics. The
slab allocator is the main allocator used to store tuples. This can be used
to monitor the total memory use and memory fragmentation.

The display of slabs is broken down by the slab size -- 64-byte, 136-byte,
and so on. The example omits the slabs which are empty. The example display
is saying that: there are 16 items stored in the 64-byte slab (and 16*64=102
so bytes_used = 1024); there is 1 item stored in the 136-byte slab
(and 136*1=136 so bytes_used = 136); the arena_used value is the total of all
the bytes_used values (1024+136 = 1160); the arena_size value is the arena_used
value plus the total of all the bytes_free values (1160+4193200+4194088 = 8388448).
The arena_size and arena_used values are the amount of the % of
:confval:`slab_alloc_arena` that is already distributed to the slab allocator.

.. data:: slab

    .. code-block:: lua

        bee> db.slab.info().arena_used
        ---
        - 4194304
        ...
        bee> db.slab.info().arena_size
        ---
        - 104857600
        ...
        bee> db.slab.info().slabs
        ---
        - - {mem_free: 9320, mem_used: 6976, 'item_count': 109,
            'item_size': 64, 'slab_count': 1, 'slab_size': 16384}
          - {mem_free: 16224, mem_used: 72, 'item_count': 1,
            'item_size': 72, 'slab_count': 1,'slab_size': 16384}
        etc.
        ...
        bee> db.slab.info().slabs[1]
        ---
        - {mem_free: 9320, mem_used: 6976, 'item_count': 109,
          'item_size': 64, 'slab_count': 1, 'slab_size': 16384}
        ...

=====================================================================
                         Package `db.stat`
=====================================================================

.. module:: db.stat

The ``db.stat`` package provides access to request statistics. Show the
average number of requests per second, and the total number of requests
since startup, broken down by request type.

.. data:: db.stat

    .. code-block:: lua

        bee> db.stat, type(db.stat) -- a virtual table
        ---
        - []
        - table
        ...
        bee> db.stat() -- the full contents of the table
        ---
        - DELETE:
            total: 48902544
            rps: 147
          EVAL:
            total: 0
            rps: 0
          SELECT:
            total: 388322317
            rps: 1246
          REPLACE:
            total: 4
            rps: 0
          INSERT:
            total: 48207694
            rps: 139
          AUTH:
            total: 0
            rps: 0
          CALL:
            total: 8
            rps: 0
          UPDATE:
            total: 743350520
            rps: 1874
        ...
        bee> db.stat().DELETE -- a selected item of the table
        ---
        - total: 48902544
          rps: 0
        ...

