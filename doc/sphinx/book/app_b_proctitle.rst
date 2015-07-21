.. _book-proctitle:

-------------------------------------------------------------------------------
                        Appendix B. Process title
-------------------------------------------------------------------------------

Linux and FreeBSD operating systems allow a running process to modify its title,
which otherwise contains the program name. Bee uses this feature to help
meet the needs of system administration, such as figuring out what services are
running on a host, their status, and so on.

A Bee server process title follows the following naming scheme:
**program_name: role[@** :confval:`custom_proc_title` **]**

**program_name** is typically **bee**. The role can be one of the following:

* **running** -- ordinary node "ready to accept requests",
* **loading** -- ordinary node recovering from old snap and wal files,
* **orphan** -- not in a cluster,
* **hot_standby** -- see section :confval:`local_hot_standby`,
* **dumper + process-id** -- saving a snapshot,

For example:

.. code-block:: bash

    $ ps -A -f | grep bee
    1000     17701  2778  0 08:27 pts/0    00:00:00 bee: running
