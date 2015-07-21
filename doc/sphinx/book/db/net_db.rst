-----------------------------------------------------------------------------------------
                            Package `net.db` -- working with networked Bee peers
-----------------------------------------------------------------------------------------

The ``net.db`` package contains connectors to remote database systems. One
variant, ``db.net.sql``, is for connecting to MySQL or MariaDB or PostgreSQL —
that variant is the subject of the :ref:`SQL DBMS plugins <dbms-plugins>` appendix.
In this section the subject is the built-in variant, ``db.net``. This is for
connecting to bee servers via a network.

Call ``require('net.db')`` to get a ``net.db`` object, which will be called
``net_db`` for examples in this section. Call ``net_db.new()`` to connect and
get a connection object, which will be called conn for examples in this section.
Call the other ``net.db()`` routines, passing ``conn:``, to execute requests on
the remote db. Call :func:`conn:close <socket_object.close>` to disconnect.

All `net.db`` methods are fiber-safe, that is, it is safe to share and use the
same connection object across multiple concurrent fibers. In fact, it's perhaps
the best programming practice with Bee. When multiple fibers use the same
connection, all requests are pipelined through the same network socket, but each
fiber gets back a correct response. Reducing the number of active sockets lowers
the overhead of system calls and increases the overall server performance. There
are, however, cases when a single connection is not enough — for example when it's
necessary to prioritize requests or to use different authentication ids.

.. module:: net_db

.. class:: conn

  conn = net_db:new(host, port [, {other parameter[s]}])

    Create a new connection. The connection is established on demand, at the
    time of the first request. It is re-established automatically after a
    disconnect. The returned conn object supports methods for making remote
    requests, such as select, update or delete.

    For the local bee server there is a pre-created always-established
    connection object named net_db.self. Its purpose is to make polymorphic
    use of the ``net_db`` API easier. Therefore ``conn = net_db:new('localhost', 3301)``
    can be replaced by ``conn = net_db.self``. However, there is an important
    difference between the embedded connection and a remote one. With the
    embedded connection, requests which do not modify data do not yield.
    When using a remote connection, any request can yield, and local database
    state may have changed by the time it returns.

    :param string host:
    :param number port:
    :param boolean wait_connect:
    :param string user:
    :param string password:
    :return: conn object
    :rtype:  userdata

    .. code-block:: lua

        EXAMPLE

        conn = net_db:new('localhost', 3301)
        conn = net_db:new('127.0.0.1', db.cfg.listen, {
            wait_connect = false,
            user = 'guest',
            password = ''
        })


    .. method:: ping()

        Execute a PING command.

        :return: true on success, false on error
        :rtype:  boolean

        .. code-block:: lua

            net_db.self:ping()

    .. method:: wait_connected([timeout])

        Wait for connection to be active or closed.

        :param number timeout:
        :return: true when connected, false on failure.
        :rtype:  boolean

        .. code-block:: lua

            net_db.self:wait_connected().

    .. method:: close()

        Close a connection.

        Connection objects are garbage collected just like any other objects in Lua, so
        an explicit destruction is not mandatory. However, since close() is a system
        call, it is good programming practice to close a connection explicitly when it
        is no longer needed, to avoid lengthy stalls of the garbage collector.

        .. code-block:: lua

            conn:close()

    .. method:: conn.space.<space-name>:select{field-value, ...}

        ``conn.space.space-name:select{...}`` is the remote-call equivalent
        of the local call ``db.space.space-name:select{...}``. Please note
        this difference: a local ``db.space.space-name:select{...}`` does
        not yield, but a remote ``conn.space.space-name:select{...}`` call
        does yield, so local data may change while a remote
        ``conn.space.space-name:select{...}`` is running.

    .. method:: conn.space.<space-name>:insert{field-value, ...}

        ``conn.space.space-name:insert(...)`` is the remote-call equivalent
        of the local call ``db.space.space-name:insert(...)``.

    .. method:: conn.space.<space-name>:replace{field-value, ...}

        ``conn.space.space-name:replace(...)`` is the remote-call equivalent
        of the local call ``db.space.space-name:replace(...)``.

    .. method:: conn.space.<space-name>:update{field-value, ...}

        ``conn.space.space-name:update(...)`` is the remote-call equivalent
        of the local call ``db.space.space-name:update(...)``.

    .. method:: conn.space.<space-name>:delete{field-value, ...}

        ``conn.space.space-name:delete(...)`` is the remote-call equivalent
        of the local call ``db.space.space-name:delete(...)``.

    .. method:: call(function-name [, arguments])

        ``conn:call('func', '1', '2', '3')`` is the remote-call equivalent of
        ``func('1', '2', '3')``. That is, ``conn:call`` is a remote
        stored-procedure call.

        .. code-block:: lua

            conn:call('function5').

    .. method:: timeout(timeout)

        ``timeout(...)`` is a wrapper which sets a timeout for the request that
        follows it.

        .. code-block:: lua

            conn:timeout(0.5).space.tester:update({1}, {{'=', 2, 15}}).

        All remote calls support execution timeouts. Using a wrapper object makes
        the remote connection API compatible with the local one, removing the need
        for a separate timeout argument, which the local version would ignore. Once
        a request is sent, it cannot be revoked from the remote server even if a
        timeout expires: the timeout expiration only aborts the wait for the remote
        server response, not the request itself.

============================================================================
                        Example showing use of most of the net.db methods
============================================================================

This example will work with the sanddb configuration described in the preface.
That is, there is a space named tester with a numeric primary key. Assume that
the database is nearly empty. Assume that the bee server is running on
``localhost 127.0.0.1:3301``.

.. code-block:: lua

    bee> db.schema.user.grant('guest', 'read,write,execute', 'universe')
    ---
    ...
    bee> console = require('console'); console.delimiter('!')
    ---
    ...
    bee> net_db = require('net.db')!
    ---
    ...
    bee> function example()
             > if net_db.self:ping() then
             >   table.insert(ta, 'self:ping() succeeded')
             >   table.insert(ta, '  (no surprise -- self connection is pre-established)')
             > end
             > if db.cfg.listen == '3301' then
             >   table.insert(ta,'The local server listen address = 3301')
             > else
             >   table.insert(ta, 'The local server listen address is not 3301')
             >   table.insert(ta, '(  (maybe db.cfg{...listen="3301"...} was not stated)')
             >   table.insert(ta, '(  (so connect will fail)')
             > end
             > conn = net_db:new('127.0.0.1', 3301)
             > conn.space.tester:delete{800}
             > table.insert(ta, 'conn delete done on tester.')
             > conn.space.tester:insert{800, 'data'}
             > table.insert(ta, 'conn insert done on tester, index 0')
             > table.insert(ta, '  primary key value = 800.')
             > wtuple = conn.space.tester:select{800}
             > table.insert(ta, 'conn select done on tester, index 0')
             > table.insert(ta, '  number of fields = ' .. #wtuple)
             > conn.space.tester:delete{800}
             > table.insert(ta, 'conn delete done on tester')
             > conn.space.tester:replace{800, 'New data', 'Extra data'}
             > table.insert(ta, 'conn:replace done on tester')
             > conn:timeout(0.5).space.tester:update({800}, {{'=', 2, 'Fld#1'}})
             > table.insert(ta, 'conn update done on tester')
             > conn:close()
             > table.insert(ta, 'conn close done')
             > end!
    ---
    ...
    bee> console.delimiter('')!
    ---
    ...
    bee> ta = {}
    ---
    ...
    bee> example()
    ---
    ...
    bee> ta
    ---
    - - self:ping() succeeded
      - '  (no surprise -- self connection is pre-established)'
      - The local server listen address = 3301
      - conn delete done on tester.
      - conn insert done on tester, index 0
      - '  primary key value = 800.'
      - conn select done on tester, index 0
      - '  number of fields = 1'
      - conn delete done on tester
      - conn:replace done on tester
      - conn update done on tester
      - conn close done
    ...
    bee> db.space.tester:select{800} -- Prove that the update succeeded.
    ---
    - [800, 'Fld#1', 'Extra data']
    ...

