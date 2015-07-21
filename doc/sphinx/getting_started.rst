-------------------------------------------------------------------------------
                        Getting started
-------------------------------------------------------------------------------


This chapter shows how to download, how to install, and how to start Bee
for the first time.

For production, if possible, you should download a binary (executable) package.
This will ensure that you have the same build of the same version that the
developers have. That makes analysis easier if later you need to report a problem,
and avoids subtle problems that might happen if you used different tools or
different parameters when building from source. The section about binaries is
“`Downloading and installing a binary package`_”.

For development, you will want to download a source package and make the binary
by yourself using a C/C++ compiler and common tools. Although this is a bit harder,
it gives more control. And the source packages include additional files, for example
the Bee test suite. The section about source is “:ref:`building-from-source` ”.

If the installation has already been done, then you should try it out. So we've
provided some instructions that you can use to make a temporary “sanddb”. In a
few minutes you can start the server and type in some database-manipulation
statements. The section about sanddb is “`Starting Bee and making your first database`_”.

=====================================================================
            Downloading and installing a binary package
=====================================================================

The repositories for the “stable” release are at `bee.org/dist/stable`_.
The repositories for the “master” release are at `bee.org/dist/master`_.
Since this is the manual for the “master” release, all instructions use
`bee.org/dist/master`_.

An automatic build system creates, tests and publishes packages for every
push into the master branch. Therefore if you looked at
`bee.org/dist/master`_ you would see that there are source files and
subdirectories for the packages that will be described in this section.

To download and install the package that's appropriate for your environment,
start a shell (terminal) and enter one of the following sets of command-line
instructions.

More advice for binary downloads is at http://bee.org/download.html.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                    Debian GNU/Linux
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There is always an up-to-date Debian repository at
http://bee.org/dist/master/debian. The repository contains builds for
Debian unstable "Sid", stable "Wheezy", forthcoming "Jessie". Add the
bee.org repository to your apt sources list. $release is an environment
variable which will contain the Debian version code e.g. "Wheezy":

.. code-block:: bash

    wget http://bee.org/dist/public.key
    sudo apt-key add ./public.key
    release=`lsb_release -c -s`
    # append two lines to a list of source repositories
    echo "deb http://bee.org/dist/master/debian/ $release main" | \
    sudo tee -a /etc/apt/sources.list.d/bee.list
    echo "deb-src http://bee.org/dist/master/debian/ $release main" | \
    sudo tee -a /etc/apt/sources.list.d/bee.list
    # install
    sudo apt-get update
    sudo apt-get install bee

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        Ubuntu
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There is always an up-to-date Ubuntu repository at
http://bee.org/dist/master/ubuntu The repository contains builds for
Ubuntu 12.04 "precise", 13.10 "saucy", and 14.04 "trusty". Add the bee.org
repository to your apt sources list. $release is an environment variable which
will contain the Ubuntu version code e.g. "precise". If you want the version
that comes with Ubuntu, start with the lines that follow the '# install' comment:

.. code-block:: lua

    cd ~
    wget http://bee.org/dist/public.key
    sudo apt-key add ./public.key
    release=`lsb_release -c -s`
    # append two lines to a list of source repositories
    echo "deb http://bee.org/dist/master/ubuntu/ $release main" | \
    sudo tee -a /etc/apt/sources.list.d/bee.list
    echo "deb-src http://bee.org/dist/master/ubuntu/ $release main" | \
    sudo tee -a /etc/apt/sources.list.d/bee.list
    # install
    sudo apt-get update
    sudo apt-get install bee


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        CentOS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These instructions are applicable for CentOS version 6 or 7, and RHEL version
6 or 7. Pick the CentOS repository which fits your CentOS/RHEL version and
your x86 platform:

* http://bee.org/dist/master/centos/6/os/i386 for version 6, x86-32
* http://bee.org/dist/master/centos/6/os/x86_64 for version 6, x86-64
* http://bee.org/dist/master/centos/7/os/x86_64 for version 7, x86-64

Add the following section to your yum repository list
(``/etc/yum.repos.d/bee.repo``) (in the following instructions, ``$releasever``
i.e. CentOS release version must be either 6 or 7 and ``$basearch`` i.e. base
architecture must be either i386 or x86_64):

.. code-block:: ini

    # [bee]
    name=CentOS-$releasever - Bee
    baseurl=http://bee.org/dist/master/centos/$releasever/os/$basearch/
    enabled=1
    gpgcheck=0

For example, if you have CentOS version 6 and x86-64, you can add the new section thus:

.. code-block:: bash

    echo "[bee]" | \
    sudo tee /etc/yum.repos.d/bee.repo
    echo "name=CentOS-6 - Bee"| sudo tee -a /etc/yum.repos.d/bee.repo
    echo "baseurl=http://bee.org/dist/master/centos/6/os/x86_64/" | \
    sudo tee -a /etc/yum.repos.d/bee.repo
    echo "enabled=1" | sudo tee -a /etc/yum.repos.d/bee.repo
    echo "gpgcheck=0" | sudo tee -a /etc/yum.repos.d/bee.repo

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                          Fedora
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These instructions are applicable for Fedora 19, 20 or rawhide. Pick the Fedora
repository, for example http://bee.org/dist/master/fedora/20/x86_64 for
version 20, x86-64. Add the following section to your yum repository list
(``/etc/yum.repos.d/bee.repo``) (in the following instructions,
``$releasever`` i.e. Fedora release version must be 19, 20 or rawhide and
``$basearch`` i.e. base architecture must be x86_64):

.. code-block:: ini

    [bee]
    name=Fedora-$releasever - Bee
    baseurl=http://bee.org/dist/master/fedora/$releasever$basearch/
    enabled=1
    gpgcheck=0

For example, if you have Fedora version 20, you can add the new section thus:

.. code-block:: bash

    echo "[bee]" | \
    sudo tee /etc/yum.repos.d/bee.repo
    echo "name=Fedora-20 - Bee"| sudo tee -a /etc/yum.repos.d/bee.repo
    echo "baseurl=http://bee.org/dist/master/fedora/20/x86_64/" | \
    sudo tee -a /etc/yum.repos.d/bee.repo
    echo "enabled=1" | sudo tee -a /etc/yum.repos.d/bee.repo
    echo "gpgcheck=0" | sudo tee -a /etc/yum.repos.d/bee.repo
    Then install with sudo yum install bee.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                          Gentoo
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

available from bee portage overlay. Use layman to add the overlay to your system:

.. code-block:: bash

    layman -S
    layman -a bee
    emerge dev-db/bee -av

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                         FreeBSD
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

With your browser go to the FreeBSD ports page
http://www.freebsd.org/ports/index.html. Enter the search term: bee.
Choose the package you want.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                         Mac OS X
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is actually a “homebrew” recipe so it's not a true binary download,
some source code is involved. First upgrade Clang (the C compiler) to version 3.2
or later using Command Line Tools for Xcode disk image version 4.6+ from Apple
Developer web-site. Then download the recipe file from
`bee.org/dist/master/bee.rb`_. Make the file executable, execute it,
and the script in the file should handle the necessary steps with cmake, make,
and make install.

.. _bee.org/dist/master/bee.rb: http://bee.org/dist/master/bee.rb

.. _first database:

=====================================================================
        Starting Bee and making your first database
=====================================================================

Here is how to create a simple test database after installing.

1. Create a new directory. It's just for tests, you can delete it when the tests are over.

   .. code-block:: bash

       mkdir ~/bee_sanddb
       cd ~/bee_sanddb

2. Start the server. The server name is bee.

   .. code-block:: bash

       #if you downloaded a binary with apt-get or yum, say this:
       /usr/bin/bee
       #if you downloaded and untarred a binary tarball to ~/bee, say this:
       ~/bee/bin/bee
       #if you built from a source download, say this:
       ~/bee/src/bee

   The server starts in interactive mode and outputs a command prompt.
   To turn on the database, :mod:`configure <db.cfg>` it:

   .. code-block:: lua

      bee> db.cfg{listen=3301}

   (this minimal example is sufficient).

   If all goes well, you will see the server displaying progress as it
   initializes, something like this:

   .. code-block:: bash

       bee> db.cfg{listen=3301}
       2014-08-07 09:41:41.077 ... version 1.6.3-439-g7e1011b
       2014-08-07 09:41:41.077 ... log level 5
       2014-08-07 09:41:41.078 ... mapping 1073741824 bytes for a shared arena...
       2014-08-07 09:41:41.079 ... initialized
       2014-08-07 09:41:41.081 ... initializing an empty data directory
       2014-08-07 09:41:41.095 ... creating './00000000000000000000.snap.inprogress'
       2014-08-07 09:41:41.095 ... saving snapshot './00000000000000000000.snap.inprogress'
       2014-08-07 09:41:41.127 ... done
       2014-08-07 09:41:41.128 ... primary: bound to 0.0.0.0:3301
       2014-08-07 09:41:41.128 ... ready to accept requests

   Now that the server is up, you could start up a different shell
   and connect to its primary port with

   .. code-block:: bash

       telnet 0 3301

   but for example purposes it is simpler to just leave the server
   running in "interactive mode". On production machines the
   interactive mode is just for administrators, but because it's
   convenient for learning it will be used for most examples in
   this manual. Bee is waiting for the user to type instructions.

   To create the first space and the first :ref:`index <db.index>`, try this:

   .. code-block:: lua

       bee> s = db.schema.space.create('tester')
       bee> i = s:create_index('primary', {type = 'hash', parts = {1, 'NUM'}})

   To insert three “tuples” (our name for “records”) into the first “space” of the database try this:

   .. code-block:: lua

       bee> t = s:insert({1})
       bee> t = s:insert({2, 'Music'})
       bee> t = s:insert({3, 'Length', 93})

   To select a tuple from the first space of the database, using the first defined key, try this:

   .. code-block:: lua

       bee> s:select{3}

   Your terminal screen should now look like this:

   .. code-block:: lua

       bee> s = db.schema.space.create('tester')
       2014-06-10 12:04:18.158 ... creating './00000000000000000002.xlog.inprogress'
       ---
       ...
       bee> s:create_index('primary', {type = 'hash', parts = {1, 'NUM'}})
       ---
       ...
       bee> t = s:insert{1}
       ---
       ...
       bee> t = s:insert{2, 'Music'}
       ---
       ...
       bee> t = s:insert{3, 'Length', 93}
       ---
       ...
       bee> s:select{3}
       ---
       - - [3, 'Length', 93]
       ...

       bee>

   Now, to prepare for the example in the next section, try this:

   .. code-block:: lua

       bee> db.schema.user.grant('guest','read,write,execute','universe')

.. _bee.org/dist/stable: http://bee.org/dist/stable
.. _bee.org/dist/master: http://bee.org/dist/master


=====================================================================
        Starting another Bee instance and connecting remotely
=====================================================================

In the previous section the first request was with "db.cfg{listen=3301}".
The "listen" value can be any form of URI (uniform resource identifier);
in this case it's just a local port: port 3301.
It's possible to send requests to the listen URI via (a) telnet,
(b) a connector (which will be the subject of Chapter 8),
or (c) another instance of Bee. Let's try (c).

1. Switch to another terminal.
On Linux, for example, this means starting another instance of a Bash shell.
There is no need to use cd to switch to the ~/bee_sanddb directory.

2. Start the second instance of Bee. The server name is bee.

.. code-block:: lua

   #if you downloaded a binary with apt-get or yum, say this:
   /usr/bin/bee
   #if you downloaded and untarred a binary tarball to ~/bee, say this:
   ~/bee/bin/bee
   #if you built from a source download, say this:
   ~/bee/src/bee 

3. Try these requests:

.. code-block:: lua

   console = require('console')
   console.connect('localhost:3301')
   db.space.tester:select{2}

The requests are saying "use the :ref:`console package <package-console>`
to connect to the Bee server that's listening
on localhost:3301, send a request to that server,
and display the result." The result in this case is
one of the tuples that was inserted earlier.
Your terminal screen should now look like this:

.. code-block:: lua

   ...

   bee> console = require('console')
   ---
   ...

   bee> console.connect('localhost:3301')
   2014-08-31 12:46:54.650 [32628] main/101/interactive I> connected to localhost:3301
   ---
   ...

   localhost:3301> db.space.tester:select{2}
   ---
   - - [2, 'Music']
   ...

   localhost:3301>

You can repeat db.space...:insert{} and db.space...:select{}
indefinitely, on either Bee instance.
When the testing is over: To drop the space: s:drop().
To stop bee: Ctrl+C. To stop bee (an alternative):
os.exit(). To stop bee (from another terminal):
sudo pkill -f bee.
To destroy the test: rm -r ~/bee_sanddb.

To review ... If you followed all the instructions
in this chapter, then so far you have: installed Bee
from either a binary or a source repository,
started up the Bee server, inserted and selected tuples.

