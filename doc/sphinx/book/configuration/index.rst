.. _book-cfg:

-------------------------------------------------------------------------------
                        Configuration reference
-------------------------------------------------------------------------------

This chapter provides a reference of options which can be set on the command
line or in an initialization file.

Bee is started by entering the command:

.. program:: bee

.. code-block:: bash

    $ bee
    OR
    $ bee <options>
    OR
    $ bee <lua-initialization-file> [arguments]

=====================================================================
                        Command options
=====================================================================

.. option:: -h, --help

    Print an annotated list of all available options and exit.

.. option:: -V, --version

    Print product name and version, for example:

    .. code-block:: bash

        $  ./bee --version
        Bee 1.6.3-439-g7e1011b
        Target: Linux-x86_64-Debug
        ...

    In this example:
    “Bee” is the name of the reusable asynchronous networking
    programming framework.

    The 3-number version follows the standard ``<major>-<minor>-<patch>``
    scheme, in which ``<major>`` number is changed only rarely, ``<minor>`` is
    incremented for each new milestone and indicates possible incompatible
    changes, and ``<patch>`` stands for the number of bug fix releases made after
    the start of the milestone. For non-released versions only, there may be a
    commit number and commit SHA1 to indicate how much this particular build has
    diverged from the last release.

    “Target” is the platform bee was built on. Some platform-specific details
    may follow this line.

    .. NOTE::

        Bee uses `git describe`_ to produce its version id, and this id
        can be used at any time to check out the corresponding source from our
        `git repository`_.

.. _git describe: http://www.kernel.org/pub/software/scm/git/docs/git-describe.html
.. _git repository: http://github.com/bee/bee.git


.. _URI:

=====================================================================
                                URI
=====================================================================

Some configuration parameters and some functions depend on a URI, or
"Universal Resource Identifier". The URI string format is similar to the
`generic syntax for a URI schema`_. So it may contain (in order) a user name
for login, a password, a host name or host IP address, and a port number. Only
the port number is always mandatory. The password is mandatory if the user
name is specified, unless the user name is 'guest'. So, formally, the URI
syntax is ``[host:]port`` or ``[username:password@]host:port``
If host is omitted, then 'localhost' is assumed.
If username:password is omitted, then 'guest' is assumed. Some examples:

.. _generic syntax for a URI schema: http://en.wikipedia.org/wiki/URI_scheme#Generic_syntax

.. container:: table

    +-----------------------------+------------------------------+
    | URI fragment                | Example                      |
    +=============================+==============================+
    | port                        | 3301                         |
    +-----------------------------+------------------------------+
    | host:port                   | 127.0.0.1:3301               |
    +-----------------------------+------------------------------+
    | username:password@host:port | notguest:sesame@mail.ru:3301 |
    +-----------------------------+------------------------------+

In certain circumstances a Unix socket may be used where a URI is required.

=====================================================================
                       Initialization file
=====================================================================

If the command to start Bee includes ``<lua-initialization-file>``, then
Bee begins by invoking the Lua program in the file, which by convention
may have the name "``script.lua``". The Lua program may get further arguments
from the command line or may use operating-system functions, such as ``getenv()``.
The Lua program almost always begins by invoking ``db.cfg()``, if the database
server will be used or if ports need to be opened. For example, suppose
``script.lua`` contains the lines

.. code-block:: lua

    #!/usr/bin/env bee
    db.cfg{
        listen              = os.getenv("LISTEN_URI"),
        slab_alloc_arena    = 0.1,
        pid_file            = "bee.pid",
        rows_per_wal        = 50
    }
    print('Starting ', arg[1])

and suppose the environment variable LISTEN_URI contains 3301,
and suppose the command line is ``~/bee/src/bee script.lua ARG``.
Then the screen might look like this:

.. code-block:: lua

    $ export LISTEN_URI=3301
    $ ~/bee/src/bee script.lua ARG
    ... main/101/script.lua C> version 1.6.3-439-g7e1011b
    ... main/101/script.lua C> log level 5
    ... main/101/script.lua I> mapping 107374184 bytes for a shared arena...
    ... main/101/script.lua I> recovery start
    ... main/101/script.lua I> recovering from `./00000000000000000000.snap'
    ... main/101/script.lua I> primary: bound to 0.0.0.0:3301
    ... main/102/leave_local_hot_standby I> ready to accept requests
    Starting  ARG
    ... main C> entering the event loop

.. _local_hot_standby:
.. _replication_port:
.. _slab_alloc_arena:
.. _replication_source:
.. _admin_port:
.. _snap_dir:
.. _wal_dir:
.. _wal_mode:
.. _snapshot daemon:
.. _logger:

=====================================================================
                Configuration parameters
=====================================================================

Configuration parameters have the form ``db.cfg{ key = value [, key = value ...]}``.
Since ``db.cfg`` may contain many configuration parameters and since some of the
parameters (such as directory addresses) are semi-permanent, it's best to keep
``db.cfg`` in a Lua file. Typically this Lua file is the initialization file
which is specified on the bee command line.

Most configuration parameters are for allocating resources, opening ports, and
specifying database behavior. All parameters are optional. A few parameters are
dynamic, that is, they can be changed at runtime by calling ``db.cfg{}``
a second time.

To see all the non-null parameters, say ``db.cfg`` (no parentheses). To see a
particular parameter, for example the listen address, say ``db.cfg.listen``.

The following tables describe all parameters for basic operation, for storage,
for binary logging and snapshots, for replication, for networking, and for logging.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                 Basic parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: cfg-basic.rst

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                 Configuring the storage
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: cfg-storage.rst

.. _book-cfg-snapshot_daemon:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                    Snapshot daemon
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: cfg-snapshot_daemon.rst

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            Binary logging and snapshots
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: cfg-binary_logging_snapshots.rst

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                    Replication
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: cfg-replication.rst

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                       Networking
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: cfg-networking.rst

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                         Logging
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: cfg-logging.rst

.. _book-cfg-local_hot_standy:

=====================================================================
                         Local hot standby
=====================================================================

Local hot standby is a feature which provides a simple form of failover without
replication. To initiate it, start a second instance of the Bee server on
the same computer with the same :func:`db.cfg` configuration settings -
including the same directories and same non-null URIs. A warning should appear with a
message like

.. code-block:: lua

    W> primary: [URI] is already in use, will retry binding after [n] seconds

This is fine. It means that the second instance is ready to take over if the
first instance goes down.

The expectation is that there will be two instances of the server using the
same configuration. The first one to start will be the "primary" instance.
The second one to start will be the "standby" instance. The standby instance
will initialize and will try to connect on listen address,
but will fail because the primary instance has already taken it. So the
standby instance goes into a loop, reading the write ahead log which the
primary instance is writing (so the two instances are always in synch),
and trying to connect on the port. If the primary instance goes down for any
reason, the port will become free so the standby instance will succeed in
connecting, and will become the primary instance. Thus there is no noticeable
downtime if the primary instance goes down.

If this ``local_hot_standby`` feature is being used, then ``wal_mode`` should
not be equal to "none".
