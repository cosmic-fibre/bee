.. _db-connectors:

-------------------------------------------------------------------------------
                            Connectors
-------------------------------------------------------------------------------

This chapter documents APIs for various programming languages.

=====================================================================
                            Protocol
=====================================================================

Bee protocol was designed with a focus on asynchronous I/O and easy integration
with proxies. Each client request starts with a variable-length binary header,
containing request id, request type, server id, log sequence number, and so on.

The mandatory length, present in request header simplifies client or proxy I/O.
A response to a request is sent to the client as soon as it is ready. It always
carries in its header the same type and id as in the request. The id makes it
possible to match a request to a response, even if the latter arrived out of order.

Unless implementing a client driver, one needn't concern oneself with the
complications of the binary protocol. `Language-specific drivers`_ provide a
friendly way to store domain language data structures in Bee. A complete
description of the binary protocol is maintained in annotated Backus-Naur form
in the source tree: please see :ref:`iproto protocol`.

====================================================================
                          Packet example
====================================================================

The Bee API exists so that a client program can send a request packet to
the server, and receive a response. Here is an example of a what the client
would send for ``db.space[513]:insert{'A', 'BB'}``. The BNF description of the
components is in file :ref:`iproto protocol`.

.. _Language-specific drivers: `Connectors`_

.. container:: table

    +---------------------------------+---------+---------+---------+---------+
    | Component                       | Byte #0 | Byte #1 | Byte #2 | Byte #3 |
    +=================================+=========+=========+=========+=========+
    | code for insert                 |    2    |         |         |         |
    +---------------------------------+---------+---------+---------+---------+
    | rest of header                  |   ...   |   ...   |   ...   |   ...   |
    +---------------------------------+---------+---------+---------+---------+
    | 2-digit number: space id        |   cd    |   02    |   01    |         |
    +---------------------------------+---------+---------+---------+---------+
    | code for tuple                  |   21    |         |         |         |
    +---------------------------------+---------+---------+---------+---------+
    | 1-digit number: field count = 2 |   92    |         |         |         |
    +---------------------------------+---------+---------+---------+---------+
    | 1-character string: field[1]    |   a1    |   41    |         |         |
    +---------------------------------+---------+---------+---------+---------+
    | 2-character string: field[2]    |   a2    |   42    |   42    |         |
    +---------------------------------+---------+---------+---------+---------+

Now, one could send that packet to the bee server, and interpret the response
(:ref:`iproto protocol` has a description of the packet format for responses as
well as requests). But it would be easier, and less error-prone, if one could invoke
a routine that formats the packet according to typed parameters. Something like
``response=bee_routine("insert",513,"A","B");``. And that is why APIs exist for
drivers for Perl, Python, PHP, and so on.

====================================================================
          Setting up the server for connector examples
====================================================================

This chapter has examples that show how to connect to the Bee server via
the Perl, PHP, and Python connectors. The examples contain hard code that will
work if and only if the server (bee) is running on localhost (127.0.0.1)
and is listening on port 3301 (``db.cfg.listen='3301'``) and space 'examples'
has id = 999 (``db.space.tester.id = 999``), and space 'examples' has a
primary-key index for a numeric field (``db.space[999].index[0].parts[1].type
= "NUM"``) and user 'guest' has privileges for reading and writing.

It is easy to meet all the conditions by starting the server and executing this
script:

.. code-block:: lua

    db.cfg{listen=3301}
    db.schema.space.create('examples',{id=999})
    db.space.examples:create_index('primary', {type = 'hash', parts = {1, 'NUM'}})
    db.schema.user.grant('guest','read,write','space','examples')
    db.schema.user.grant('guest','read','space','_space')

.. include:: __java.rst

.. include:: __go.rst

.. include:: __perl.rst

.. include:: __php.rst

.. include:: __python.rst
