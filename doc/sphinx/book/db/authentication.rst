.. _db-authentication:

-------------------------------------------------------------------------------
                    Authentication and access control
-------------------------------------------------------------------------------

Understanding the details of security is primarily an issue for administrators,
but ordinary users should at least skim this section so that they will have an
idea of how Bee makes it possible for administrators to prevent
unauthorized access to the database and to certain functions.

Briefly: there is a method to guarantee with password checks that users really
are who they say they are ("authentication"). There is a _user space where user
names and password-hashes are stored. There are functions for saying that
certain users are allowed to do certain things ("privileges"). There is a _priv
space where privileges are stored. Whenever a user tries to do an operation,
there is a check whether the user has the privilege to do the operation
("access control").

===========================================================
                        Passwords
===========================================================

Each user may have a password. The password is any alphanumeric string.
Administrators should advise users to choose long unobvious passwords, but it
is ultimately up to the users to choose or change their own passwords.

Bee passwords are stored in the ``_user`` space with a `Cryptographic hash function`_
so that, if the password is 'x', the stored hashed-password is a long string
like '``lL3OvhkIPOKh+Vn9Avlkx69M/Ck=``'. When a client connects to a Bee
server, the server sends a random `Salt Value`_ which the client must mix with the
hashed-password before sending to the server. Thus the original value 'x' is
never stored anywhere except in the user's head, and the hashed value is never
passed down a network wire except when mixed with a random salt. This system
prevents malicious onlookers from finding passwords by snooping in the log
files or snooping on the wire. It is the same system that `MySQL introduced
several years ago`_ which has proved adequate for medium-security installations.
Nevertheless administrators should warn users that no system is foolproof against
determined long-term attacks, so passwords should be guarded and changed occasionally.

.. NOTE:

    To get the hash-password of a string '``X``', say ``db.schema.user.password('X')``.
    To see more about the details of the algorithm for the purpose of writing a new
    client application, read the `scramble.h`_ header file.

.. _Cryptographic hash function: https://en.wikipedia.org/wiki/Cryptographic_hash
.. _Salt Value: https://en.wikipedia.org/wiki/Salt_%28cryptography%29
.. _MySQL introduced several years ago: http://dev.mysql.com/doc/refman/4.1/en/password-hashing.html
.. _scramble.h: https://github.com/bee/bee/blob/master/src/scramble.h

===========================================================
                Users and the _user space
===========================================================

The fields in the ``_user`` space are: the numeric id of the tuple, the numeric
id of the tuple's creator, the user name, the type, and the optional password.

There are four special tuples in the _user space: 'guest', 'admin', 'public', and 'replication'.

.. container:: table

    +-------------+----+------+--------------------------------------------------------+
    | Name        | ID | Type | Description                                            |
    +=============+====+======+========================================================+
    | guest       | 0  | user | Default when connecting remotely. Usually an untrusted |
    |             |    |      | user with few privileges.                              |
    +-------------+----+------+--------------------------------------------------------+
    | admin       | 1  | user | Default when using ``bee`` as a console. Usually |
    |             |    |      | an administrative user with all privileges.            |
    +-------------+----+------+--------------------------------------------------------+
    | public      | 2  | role | Not a user in the usual sense. Described later in      |
    |             |    |      | section `Roles`_.                                      |
    +-------------+----+------+--------------------------------------------------------+
    | replication | 3  | role | Not a user in the usual sense. Described later in      |
    |             |    |      | section `Roles`_.                                      |
    +-------------+----+------+--------------------------------------------------------+


To select a row from the ``_user`` space, use ``db.space._user:select``. For
example, here is what happens with a select for user id = 0, which is the
'guest' user, which by default has no password:

.. code-block:: lua

    bee> db.space._user:select{0}
    ---
    - - [0, 1, 'guest', 'user']
    ...

To change tuples in the ``_user`` space, do not use ordinary ``db.space``
functions for insert or update or delete - the _user space is special so
there are special functions which have appropriate error checking.

To create a new user, say ``db.schema.user.create(user-name)`` or
``db.schema.user.create(user-name, {if_not_exists=true})`` or
``db.schema.user.create(user-name, {password=password})``. The form
``db.schema.user.create(user-name, {password=password})`` is better because
in a :ref:`URI` (Uniform Resource Identifier) it is usually illegal to include a
user-name without a password.

To change the current user's password, say ``db.schema.user.passwd(password)``.

To change a different user's password, say ``db.schema.user.passwd(user-name, password)``.
(Only the admin user can perform this function.)

To drop a user, say ``db.schema.user.drop(user-name)``.

To check whether a user exists, say ``db.schema.user.exists(user-name)``,
which returns true or false.

To find what privileges a user has, say ``db.schema.user.info(user-name)``.

For example, here is a session which creates a new user with a strong password,
selects a tuple in the ``_user`` space, and then drops the user.

.. code-block:: lua

    bee> db.schema.user.create('JeanMartin', {password = 'Iwtso_6_os$$'})
    ---
    ...

    bee> db.space._user.index.name:select{'JeanMartin'}
    ---
    - - [17, 1, 'JeanMartin', 'user', {'chap-sha1': 't3xjUpQdrt857O+YRvGbMY5py8Q='}]
    ...

    bee> db.schema.user.drop('JeanMartin')
    ---
    ...

.. NOTE::

    The maximum number of users is 32.

===========================================================
               Privileges and the _priv space
===========================================================

The fields in the ``_priv`` space are: the numeric id of the user who gave the
privilege ("grantor_id"), the numeric id of the user who received the
privilege ("grantee_id"), the id of the object, the type of object - "space"
or "function" or "universe", the type of operation - "read" or "write" or
"execute" or a combination such as "read,write,execute".

The function for granting a privilege is:
``db.schema.user.grant(user-name-of-grantee, operation-type, object-type, object-name)`` or
``db.schema.user.grant(user-name-of-grantee, operation-type, 'universe')``.

The function for revoking a privilege is:
``db.schema.user.revoke(user-name-of-grantee, operation-type, object-type, object-name)`` or
``db.schema.user.revoke(user-name-of-grantee, operation-type, 'universe')``.

For example, here is a session where the admin user gave the guest user the
privilege to read from a space named space55, and then took the privilege away:

.. code-block:: lua

    bee> db.schema.user.grant('guest', 'read', 'space', 'space55')
    ---
    ...
    bee> db.schema.user.revoke('guest', 'read', 'space', 'space55')
    ---
    ...

.. NOTE::

    Generally privileges are granted or revoked by the owner of the object (the
    user who created it), or by the 'admin' user. Before dropping any objects
    or users, steps should be taken to ensure that all their associated
    privileges have been revoked. Only the 'admin' user can grant privileges
    for the 'universe'.


===========================================================
                Functions and _func space
===========================================================

The fields in the ``_func`` space are: the numeric function id, a number,
the function name, and a flag.

The ``_func`` space does not include the function's body. One continues to
create Lua functions in the usual way, by saying
"``function function_name () ... end``", without adding anything in the
``_func`` space. The _func space only exists for storing function tuples so
that their names can be used within grant/revoke functions.

The function for creating a ``_func`` tuple is:
``db.schema.func.create(function-name [, {if_not_exists=true} ])``.

The variant function for creating a ``_func`` tuple is:
``db.schema.func.create(function-name , {setuid=true} )``.
This causes the flag (the fourth field in the _func tuple) to have
a value meaning "true", and the effect of that is that the
function's caller is treated as the function's creator,
with full privileges. The setuid behavior does not apply for
users who connect via console.connect.

The function for dropping a ``_func`` tuple is:
``db.schema.func.drop(function-name)``.

The function for checking whether a ``_func`` tuple exists is:
``db.schema.func.exists(function-name)``.

In the following example, a function named 'f7' is created, then it is put in
the ``_func`` space, then it is used in a ``db.schema.user.grant`` function,
then it is dropped:

.. code-block:: lua

    bee> function f7() db.session.uid() end
    ---
    ...
    bee> db.schema.func.create('f7')
    ---
    ...
    bee> db.schema.user.grant('guest', 'execute', 'function', 'f7')
    ---
    ...
    bee> db.schema.user.revoke('guest', 'execute', 'function', 'f7')
    ---
    ...
    bee> db.schema.func.drop('f7')
    ---
    ...

===========================================================
             ``db.session`` and security
===========================================================

After a connection has taken place, the user has access to a "session" object
which has several functions. The ones which are of interest for security
purposes are:

.. code-block:: lua

    db.session.uid()         -- returns the id of the current user
    db.session.user()        -- returns the name of the current user
    db.session.su(user-name) -- allows changing current user to 'user-name'

If a user types requests directly on the Bee server in its interactive
mode, or if a user connects via telnet to the administrative port (using :ref:`admin <admin_port>`
instead of listen), then the user by default is 'admin' and has many privileges.
If a user connects from an application program via one of the :ref:`connectors <db-connectors>`, then
the user by default is 'guest' and has few privileges. Typically an admin user
will set up and configure objects, then grant privileges to appropriate non-admin
users. Typically a guest user will use ``db.session.su()`` to change into a non-generic
user to whom admin has granted more than the default privileges. For example,
admin might say:

.. _connectors: :doc:`../connectors/index`

.. code-block:: lua

    db.space._user:insert{123456,0,'manager'}
    db.schema.user.grant('manager', 'read', 'space', '_space')
    db.schema.user.grant('manager', 'read', 'space', 'payroll')

and later a guest user, who wishes to see the payroll, might say:

.. code-block:: lua

    db.session.su('manager')
    db.space.payroll:select{'Jones'}

===========================================================
                         Roles
===========================================================

A role is a container for privileges which can be granted to regular users.
Instead of granting and revoking individual privileges, one can put all the
privileges in a role and then grant or revoke the role. Role information is
in the ``_user`` space but the third field - the type field - is 'role' rather
than 'user'.

If a role R1 is granted a privilege X, and user U1 is granted a privilege
"role R1", then user U1 in effect has privilege X. Then if a role R2 is
granted a privilege Y, and role R1 is granted a privilege "role R2",
then user U1 in effect has both privilege X and privilege Y. In other words,
a user gets all the privileges that are granted to a user's roles, directly
or indirectly.

.. module:: db.schema.role

.. function:: create(role-name [, {if_not_exists=true} ] )

    Create a new role.

.. function:: grant(role-name, privilege)

    Put a privilege in a role.

.. function:: revoke(role-name, privilege)

    Take a privilege out of a role.

.. function:: drop(role-name)

    Drop a role.

.. function:: info()

    Get information about a role, including what privileges have been granted to the role.

.. function:: grant(role-name, 'execute', 'role', role-name)

    Grant a role to a role.

.. function:: revoke(role-name, 'execute', 'role', role-name)

    Revoke a role from a role.

.. function:: exists(role-name)

    Check whether a role exists.
    :return: true if role-name identifies a role, otherwise false.
    :rtype:  boolean

.. module:: db.schema.user

.. function:: grant(user-name, 'execute', 'role', role-name)

    Grant a role to a user.

.. function:: revoke(user-name, 'execute', 'role', role-name)

    Revoke a role from a user.

There are two predefined roles. The first predefined role, named 'public', is automatically assigned
to new users when they are created with ``db.schema.user.create(user-name)`` --
Therefore a convenient way to grant 'read' on space '``t``' to every user that
will ever exist is: db.schema.role.grant('public','read','space','t').
The second predefined role, named 'replication', can be assigned
by the 'admin' user to users who need to use
replication features.

================================================================
                         Example showing a role within a role
================================================================

In this example, a new user named U1 will insert a new tuple into a new space
named T, and will succeed even though user U1 has no direct privilege to do
such an insert -- that privilege is inherited from role R1, which in turn
inherits from role R2.

.. code-block:: lua

    -- This example will work for a user with many privileges, such as 'admin'
    db.schema.space.create('T')
    db.space.T:create_index('primary',{})
    -- Create a user U1 so that later it's possible to say db.session.su('U1')
    db.schema.user.create('U1')
    -- Create two roles, R1 and R2
    db.schema.role.create('R1')
    db.schema.role.create('R2')
    -- Grant role R2 to role R1 and role R1 to U1 (order doesn't matter)
    db.schema.role.grant('R1','execute','role','R2')
    db.schema.role.grant('U1','execute','role','R1')
    -- Grant read and execute privileges to R2 (but not to R1 and not to U1)
    db.schema.role.grant('R2','read,write','space','T')
    db.schema.role.grant('R2','execute','universe')
    -- Use db.session.su to say "now become user U1"
    db.session.su('U1')
    -- The following insert succeeds because U1 in effect has write privilege on T
    db.space.T:insert{1}
