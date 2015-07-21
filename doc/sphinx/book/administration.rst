-------------------------------------------------------------------------------
                        Server administration
-------------------------------------------------------------------------------

Typical server administration tasks include starting and stopping the server,
reloading configuration, taking snapshots, log rotation.

=====================================================================
                        Server signal handling
=====================================================================

The server is configured to shut down gracefully on SIGTERM and SIGINT
(keyboard interrupt) or SIGHUP. SIGUSR1 can be used to save a snapshot. All
other signals are blocked or ignored. The signals are processed in the main
thread event loop. Thus, if the control flow never reaches the event loop
(thanks to a runaway stored procedure), the server stops responding to any
signal, and can only be killed with SIGKILL (this signal can not be ignored).

=====================================================================
                        Using ``bee`` as a client
=====================================================================

.. program:: bee

If ``bee`` is started without a Lua script to run, it automatically
enters interactive mode. There will be a prompt ("``bee>``") and it will
be possible to enter requests. When used this way, ``bee`` can be 
a client for a remote server.

This section shows all legal syntax for the bee program, with short notes
and examples. Other client programs may have similar options and request
syntaxes. Some of the information in this section is duplicated in the
:ref:`book-cfg` chapter.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            Conventions used in this section
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Tokens are character sequences which are treated as syntactic units within
requests. Square brackets [ and ] enclose optional syntax. Three dots in a
row ... mean the preceding tokens may be repeated. A vertical bar | means
the preceding and following tokens are mutually exclusive alternatives.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Options when starting client from the command line
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

General form:

.. code-block:: bash

    $ bee
    OR
    $ bee <options>
    OR
    $ bee <lua-initialization-file> [arguments]

<lua-initialization-file> can be any script containing code for initializing.
Effect: The code in the file is executed during startup. Example: ``init.lua``.
Notes: If a script is used, there will be no prompt. The script should contain
configuration information including "``db.cfg{...listen=...}``" or
"``db.listen(...)``" so that a separate program can connect to the server via
one of the ports.

Option is one of the following (in alphabetical order by the long form of the
option):

.. option:: -?, -h, --help

    Client displays a help message including a list of options.

    .. code-block:: bash

        bee --help

    The program stops after displaying the help.

.. option:: -V, --version

    .. code-block:: bash

        bee --version

    The program stops after displaying the version.


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      Tokens, requests, and special key combinations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Procedure identifiers are: Any sequence of letters, digits, or underscores
which is legal according to the rules for Lua identifiers. Procedure
identifiers are also called function names. Notes: function names are case
insensitive so ``insert`` and ``Insert`` are not the same thing.

String literals are: Any sequence of zero or more characters enclosed in
single quotes. Double quotes are legal but single quotes are preferred.
Enclosing in double square brackets is good for multi-line strings as
described in `Lua documentation`_.

.. _Lua documentation: http://www.lua.org/pil/2.4.html

Example:

.. code-block:: lua

    'Hello, world', 'A', [[A\B!]].

Numeric literals are: Character sequences containing only digits, optionally
preceded by + or -. Examples: 55, -. Notes: Bee NUM data type is
unsigned, so -1 is understood as a large unsigned number.

Single-byte tokens are: * or , or ( or ). Examples: * , ( ).

Tokens must be separated from each other by one or more spaces, except that
spaces are not necessary around single-byte tokens or string literals.

.. _setting delimiter:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        Requests
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Generally requests are entered following the prompt in interactive mode while
``bee`` is running. (A prompt will be the word bee and a
greater-than sign, for example ``bee>``). The end-of-request marker is by
default a newline (line feed).

For multi-line requests, it is possible to change the end-of-request marker.
Syntax: ``console = require('console'); console.delimiter(string-literal)``.
The string-literal must be a value in single quotes. Effect: string becomes
end-of-request delimiter, so newline alone is not treated as end of request.
To go back to normal mode: ``console.delimiter('')string-literal``. Example:

.. code-block:: lua

    console = require('console'); console.delimiter('!')
    function f ()
      statement_1 = 'a'
      statement_2 = 'b'
    end!
    console.delimiter('')!

In *interactive* mode, one types requests and gets results. Typically the
requests are typed in by the user following prompts. Here is an example of
an interactive-mode bee client session:

.. code-block:: bash

    $ bee
                    [ bee will display an introductory message
                      including version number here ]
    bee> db.cfg{listen=3301}
                    [ bee will display configuration information
                      here ]
    bee> s = db.schema.space.create('tester')
                    [ bee may display an in-progress message here ]
    ---
    ...
    bee> s:create_index('primary', {type = 'hash', parts = {1, 'NUM'}})
    ---
    ...
    bee> db.space.tester:insert{1,'My first tuple'}
    ---
    - [1, 'My first tuple']
    ...
    bee> db.space.tester:select(1)
    ---
    - - [1, 'My first tuple']
    ...
    bee> db.space.tester:drop()
    ---
    ...
    bee> os.exit()
    2014-04-30 10:28:00.886 [20436] main/101/spawner I> Exiting: master shutdown
    $

Explanatory notes about what bee displayed in the above example:

* Many requests return typed objects. In the case of "``db.cfg{listen=3301}``",
  this result is displayed on the screen. If the request had assigned the result
  to a variable, for example "``c = db.cfg{listen=3301}``", then the result
  would not have been displayed on the screen.
* A display of an object always begins with "``---``" and ends with "``...``".
* The insert request returns an object of type = tuple, so the object display line begins with a single dash ('``-``'). However, the select request returns an object of type = table of tuples, so the object display line begins with two dashes ('``- -``').

=====================================================================
                        Utility ``beectl``
=====================================================================

.. program:: beectl

With ``beectl`` one can say: "start an instance of the Bee server
which runs a single user-written Lua program, allocating disk resources
specifically for that program, via a standardized deployment method."
If Bee was downloaded from source, then the script is in
:file:`[bee]/extra/dist/beectl`. If Bee was installed with Debian or
Red Hat installation packages, the script is renamed :program:`beectl`
and is in :file:`/usr/bin/beectl`. The script handles such things as:
starting, stopping, rotating logs, logging in to the application's console,
and checking status.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            configuring for beectl
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The :program:`beectl` script will read a configuration file named
:file:`~/.config/bee/default`, or 
:file:`/etc/sysconfig/bee`, or :file:`/etc/default/bee`. Most
of the settings are similar to the settings used by ``db.cfg{...};``
however, beectl adjusts some of them by adding an application name.
A copy of :file:`/etc/sysconfig/bee`, with defaults for all settings,
would look like this:

.. code-block:: lua

    default_cfg = {
        pid_file   = "/var/run/bee",
        wal_dir    = "/var/lib/bee",
        snap_dir   = "/var/lib/bee",
        sham_dir = "/var/lib/bee",
        logger     = "/var/log/bee",
        username   = "bee",
    }
    instance_dir = "/etc/bee/instances.enabled"

The settings in the above script are:

``pid_file``
    The directory for the pid file and control-socket file. The
    script will add ":file:`/instance-name`" to the directory name.

``wal_dir``
    The directory for the write-ahead :file:`*.xlog` files. The
    script will add ":file:`/instance-name`" to the directory-name.

``snap_dir``
    The directory for the snapshot :file:`*.snap` files. The script
    will add ":file:`/instance-name`" to the directory-name.

``sham_dir``
    The directory for the sham-storage-engine files. The script
    will add ":file:`/sham/instance-name`" to the directory-name.

``logger``
    The place where the application log will go. The script will
    add ":file:`/instance-name.log`" to the name.

``username``
    the user that runs the bee server. This is the operating-system
    user name rather than the Bee-client user name.

``instance_dir``
    the directory where all applications for this host are stored. The user
    who writes an application for :program:`beectl` must put the
    application's source code in this directory, or a symbolic link. For
    examples in this section the application name my_app will be used, and
    its source will have to be in :file:`instance_dir/my_app.lua`.


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            commands for beectl
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The command format is ``beectl operation application-name``, where
operation is one of: start, stop, enter, logrotate, status, reload. Thus ...

.. option:: start <app_name>

    starts application <app_name>

.. option:: stop <app_name>

    stops <app_name>

.. option:: enter <app_name>

    shows <app_name>'s admin console, if it has one

.. option:: logrotate <app_name>

    rotates <app_name>'s log files (make new, remove old)

.. option:: status <app_name>

    checks <app_name>'s status

.. option:: reload <file_name>

   reloads <file_name>.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     typical code snippets for beectl
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A user can check whether my_app is running with these lines:

.. code-block:: bash

    if beectl status my_app; then
    ...
    fi

A user can initiate, for boot time, an init.d set of instructions:

.. code-block:: bash

    for (each file mentioned in the instance_dir directory):
        beectl start `basename $ file .lua`

A user can set up a further configuration file for log rotation, like this:

.. code-block:: lua

    /path/to/bee/*.log {
        daily
        size 512k
        missingok
        rotate 10
        compress
        delaycompress
        create 0640 bee adm
        postrotate
            /path/to/beectl logrotate `basename $ 1 .log`
        endscript
    }

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      A detailed example for beectl
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The example's objective is: make a temporary directory where beectl
can start a long-running application and monitor it.

The assumptions are: the root password is known, the computer is only being used
for tests, the Bee server is ready to run but is not currently running,
and there currently is no directory named :file:`bee_test`.

Create a directory named /bee_test:

.. code-block:: bash

    sudo mkdir /bee_test

Copy beectl to /bee_test. If you made a source
download to ~/bee-master, then

.. code-block:: bash

    sudo cp ~/bee-master/extra/dist/beectl /bee_test/beectl

If the file was named beectl and placed on /usr/bin/beectl, then

.. code-block:: bash

    sudo cp /usr/bin/beectl /bee_test/beectl

Check and possibly change the first line of /bee_test/beectl.
Initially it says

.. code-block:: bash

    #!/usr/bin/env bee

If that is not correct, edit beectl and change the line. For example,
if the Bee server is actually on /home/user/bee-master/src/bee,
change the line to

.. code-block:: bash

    #!/usr/bin/env /home/user/bee-master/src/bee

Save a copy of /etc/sysconfig/bee, if it exists.

Edit /etc/sysconfig/bee. It might be necessary to say sudo mkdir /etc/sysconfig first. Let the new file contents be:

.. code-block:: lua

    default_cfg = {
        pid_file = "/bee_test/my_app.pid",
        wal_dir = "/bee_test",
        snap_dir = "/bee_test",
        sham_dir = "/bee_test",
        logger = "/bee_test/log",
        username = "bee",
    }
    instance_dir = "/bee_test"

Make the my_app application file, that is, /bee_test/my_app.lua. Let the file contents be:

.. code-block:: lua

    db.cfg{listen = 3301}
    db.schema.user.passwd('Gx5!')
    db.schema.user.grant('guest','read,write,execute','universe')
    fiber = require('fiber')
    db.schema.space.create('tester')
    db.space.tester:create_index('primary',{})
    i = 0
    while 0 == 0 do
        fiber.sleep(5)
        i = i + 1
        print('insert ' .. i)
        db.space.tester:insert{i, 'my_app tuple'}
    end

Tell beectl to start the application ...

.. code-block:: bash

    cd /bee_test
    sudo ./beectl start my_app

... expect to see messages indicating that the instance has started. Then ...

.. code-block:: bash

    ls -l /bee_test/my_app

... expect to see the .snap file, .xlog file, and sham directory. Then ...

.. code-block:: bash

    less /bee_test/log/my_app.log

... expect to see the contents of my_app's log, including error messages, if any. Then ...

.. code-block:: bash

    cd /bee_test
    #assume that 'bee' invokes the bee server
    sudo bee
    db.cfg{}
    console = require('console')
    console.connect('localhost:3301')
    db.space.tester:select({0},{iterator='GE'})

... expect to see several tuples that my_app has created.

Stop. The only clean way to stop my_app is with beectl, thus:


.. code-block:: bash

    sudo ./beectl stop my_app

Clean up. Restore the original contents of /etc/sysconfig/bee, and ...

.. code-block:: bash

    cd /
    sudo rm -R bee_test

=====================================================================
            System-specific administration notes
=====================================================================

This section will contain information about issue or features which exist
on some platforms but not others - for example, on certain versions of a
particular Linux distribution.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Administrating with Debian GNU/Linux and Ubuntu
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Setting up an instance:
``ln -s /etc/bee/instances.available/instance-name.cfg /etc/bee/instances.enabled/``

Starting all instances:
``service bee start``

Stopping all instances:
``service bee stop``

Starting/stopping one instance:
``service bee-instance-name start/stop``


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                 Fedora, RHEL, CentOS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There are no known permanent issues. For transient issues, go to
http://github.com/bee/bee/issues and enter "RHEL" or
"CentOS" or "Fedora" or "Red Hat" in the search db.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                       FreeBSD
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There are no known permanent issues. For transient issues, go to
http://github.com/bee/bee/issues and enter "FreeBSD"
in the search db.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                       Mac OS X
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There are no known permanent issues. For transient issues, go to
http://github.com/bee/bee/issues and enter "OS X" in
the search db.
