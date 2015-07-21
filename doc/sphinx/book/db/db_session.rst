-------------------------------------------------------------------------------
                            Package `db.session`
-------------------------------------------------------------------------------

The ``db.session`` package allows querying the session state, writing to a
session-specific temporary Lua table, or setting up triggers which will fire
when a session starts or ends. A *session* is an object associated with each
client connection.

.. module:: db.session

.. function:: id()

    :return: the unique identifier (ID) for the current session.
             The result can be 0 meaning there is no session.
    :rtype:  number

.. function:: exists(id)

    :return: 1 if the session exists, 0 if the session does not exist.
    :rtype:  number

.. function:: peer(id)

    :return: If the specified session exists, the host address and port of
             the session peer, for example "127.0.0.1:55457". If the
             specified session does not exist, "0.0.0.0:0". The command is
             executed on the server, so the "local name" is the server's host
             and administrative port, and the "peer name" is the client's host
             and port.
    :rtype:  string

.. data:: storage

    A Lua table that can hold arbitrary unordered session-specific
    names and values, which will last until the session ends.

=================================================
                      Example
=================================================

.. code-block:: lua

    bee> db.session.peer(session.id())
    ---
    - 127.0.0.1:45129
    ...
    bee> db.session.storage.random_memorandum = "Don't forget the eggs."
    ---
    ...
    bee> db.session.storage.radius_of_mars = 3396
    ---
    ...
    bee> m = ''
    ---
    ...
    bee> for k, v in pairs(db.session.storage) do m=m..k..'='..v..' ' end
    ---
    ...
    bee> m
    ---
    - 'radius_of_mars=3396 random_memorandum=Don''t forget the eggs. '
    ...

See the section :ref:`Triggers on connect and disconnect <db-triggers>`
for instructions about defining triggers for connect and disconnect
events with ``db.session.on_connect()`` and ``db.session.on_disconnect()``.
See the section :ref:`Authentication and access control <db-authentication>`
for instructions about ``db.session`` functions that affect user
identification and security.
