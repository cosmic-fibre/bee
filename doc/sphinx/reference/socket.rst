-------------------------------------------------------------------------------
                            Package `socket`
-------------------------------------------------------------------------------

The ``socket`` package allows exchanging data via BSD sockets with a local or
remote host in connection-oriented (TCP) or datagram-oriented (UDP) mode.
Semantics of the calls in the ``socket`` API closely follow semantics of the
corresponding POSIX calls. Function names and signatures are mostly compatible
with `luasocket`_.

The functions for setting up and connecting are ``socket``, ``sysconnect``,
``tcp_connect``. The functions for sending data are ``send``, ``sendto``,
``write``, ``syswrite``. The functions for receiving data are ``recv``,
``recvfrom``, ``read``. The functions for waiting before sending/receiving
data are ``wait``, ``readable``, ``writable``. The functions for setting
flags are ``nonblock``, ``setsockopt``. The functions for stopping and
disconnecting are ``shutdown``, ``close``. The functions for error checking
are ``errno``, ``error``.

.. container:: table

    **Socket functions**

    +----------------+-------------+
    |    Purposes    |    Names    |
    +================+=============+
    |                | socket      |
    |                +-------------+
    |      setup     | sysconnect  |
    |                +-------------+
    |                | tcp_connect |
    +----------------+-------------+
    |                | send        |
    |                +-------------+
    |                | sendto      |
    |    sending     +-------------+
    |                | write       |
    |                +-------------+
    |                | syswrite    |
    +----------------+-------------+
    |                | recv        |
    |                +-------------+
    |   receiving    | recvfrom    |
    |                +-------------+
    |                | read        |
    +----------------+-------------+
    |                | nonblock    |
    |                +-------------+
    |  flag setting  | setsockopt  |
    |                +-------------+
    |                | linger      |
    +----------------+-------------+
    |                | listen      |
    | client/server  +-------------+
    |                | accept      |
    +----------------+-------------+
    |                | shutdown    |
    |    teardown    +-------------+
    |                | close       |
    +----------------+-------------+
    |                | error       |
    | error checking +-------------+
    |                | errno       |
    +----------------+-------------+
    |                | getaddrinfo |
    |                +-------------+
    |                | getsockopt  |
    |  information   +-------------+
    |                | peer        |
    |                +-------------+
    |                | name        |
    +----------------+-------------+


Typically a socket session will begin with the setup functions, will set one
or more flags, will have a loop with sending and receiving functions, will
end with the teardown functions -- as an example at the end of this section
will show. Throughout, there may be error-checking and waiting functions for
synchronization. Some functions may "block" if a non-default option flag is
set, therefore the fiber that they are in will yield so that other processes
may take over, as is the norm for cooperative multitasking.

For all examples in this section the socket name will be sock and
the function invocations will look like ``sock:function_name(...)``.

.. module:: socket

.. function:: __call(domain, type, protocol)

    Create a new TCP or UDP socket. The argument values
    are the same as in the `Linux man page <http://man7.org/linux/man-pages/man2/socket.2.html>`_.

    :param domain:
    :param type:
    :param protocol:
    :return: a new socket, or nil.
    :rtype:  userdata

.. function:: tcp_connect(host, port)
              tcp_connect(host)

    Connect a socket to a remote host.

    :param string host: URL or IP address
    :param number port: port number
    :return: a connected socket, if no error.
    :rtype: userdata

.. function:: getaddrinfo(host, type, [, {option-list}])

    The ``socket.getaddrinfo()`` function is useful for finding information
    about a remote site so that the correct arguments for
    ``sock:sysconnect()`` can be passed.

    :return: A table containing these fields: "host", "family", "type", "protocol", "port".
    :rtype:  table

    .. code-block:: lua

        bee> socket.getaddrinfo('bee.org', 'http')

    will return variable information such as

    .. code-block:: yaml

        ---
        - - host: 188.93.56.70
            family: AF_INET
            type: SOCK_STREAM
            protocol: tcp
            port: 80
          - host: 188.93.56.70
            family: AF_INET
            type: SOCK_DGRAM
            protocol: udp
            port: 80
        ...

.. function:: tcp_server(host, port, handler-function)

    The ``socket.tcp_server()`` function makes Bee act as a server that
    can accept connections. Usually the same objective
    is accomplished with ``db.cfg{listen=...)``.

    .. code-block:: lua

        socket.tcp_server('localhost', 3302, function () end).

.. class:: socket_object

    .. method:: sysconnect(host, port)

        Connect a socket to a remote host. The argument values are the same as
        in the Linux man page [1]_.
        The host must be an IP address.

        Parameters:
            * Either:
               * host - a string representation of an IPv4 address
                 or an IPv6 address;
               * port - a number.
            * Or:
               * host - a string containing "unix/";
               * port - a string containing a path to a unix socket.
            * Or:
               * host - a number, 0 (zero), meaning "all local
                 interfaces";
               * port - a number. If a port number is 0 (zero),
                 the socket will be bound to a random local port.


        :return: a connected socket, if no error.
        :rtype:  userdata

        .. code-block:: lua

            sock:sysconnect('127.0.0.1', 80)

    .. method:: send(data)
                write(data)

        Send data over a connected socket.

        :param string data:
        :return: the number of bytes sent.
        :rtype:  number
        :exception: nil on error.

    .. method:: syswrite(size)

        Write as much as possible data to the socket buffer if non-blocking.
        Rarely used. For details see `this description`_.

    .. method:: recv(size)

        Read ``size`` bytes from a connected socket. An internal read-ahead
        buffer is used to reduce the cost of this call.

        :param integer size:
        :return: a string of the requested length on success.
        :rtype:  string
        :exception: On error, returns an empty string, followed by status,
                    errno, errstr. In case the writing side has closed its
                    end, returns the remainder read from the socket (possibly
                    an empty string), followed by "eof" status.

    .. method:: read(limit [, timeout])
                read(delimiter [, timeout])
                read({limit=limit} [, timeout])
                read({delimiter=delimiter} [,timeout])
                read({limit=limit, delimiter=delimiter} [, timeout])

        Read from a connected socket until some condition is true, and return
        the bytes that were read.
        Reading goes on until ``limit`` bytes have been read, or a delimiter
        has been read, or a timeout has expired.

        :param integer    limit: maximum number of bytes to read for
                                 example 50 means "stop after 50 bytes"
        :param string delimiter: separator or `Lua pattern`_ for example
                                 '[0-9]' means "stop after a digit"
        :param number   timeout: maximum number of seconds to wait for
                                 example 50 means "stop after 50 seconds".

        :return: an empty string if there is nothing more to read, or a nil
                 value if error, or a string up to ``limit`` bytes long,
                 which may include the bytes that matched the ``delimiter``
                 expression.
        :rtype: string

    .. method:: sysread(size)

        Return all available data from the socket buffer if non-blocking.
        Rarely used. For details see `this description`_.

    .. method:: bind(host [, port])

        Bind a socket to the given host/port. A UDP socket after binding
        can be used to receive data (see :func:`socket_object.recvfrom`).
        A TCP socket can be used to accept new connections, after it has
        been put in listen mode.

        :param host:
        :param port:

        :return: a socket object on success
        :rtype:  userdata
        :exception: nil, status, errno, errstr on error.


    .. method:: listen(backlog)

        Start listening for incoming connections.

        :param backlog: On Linux the listen ``backlog`` backlog may be from
                        /proc/sys/net/core/somaxconn, on BSD the backlog
                        may be ``SOMAXCONN``.

        :return: true for success, false for error.
        :rtype: boolean.

    .. method:: accept()

        Accept a new client connection and create a new connected socket.
        It is good practice to set the socket's blocking mode explicitly
        after accepting.

        :return: new socket if success.
        :rtype: userdata
        :exception: nil

    .. method:: sendto(host, port, data)

        Send a message on a UDP socket to a specified host.

        :param string host:
        :param number port:
        :param string data:

        :return: the number of bytes sent.
        :rtype:  number
        :exception: status, errno, errstr.

    .. method:: recvfrom(limit)

        Receive a message on a UDP socket.

        :param integer limit:
        :return: message, a table containing "host", "family" and "port" fields.
        :rtype:  string, table
        :exception: status, errno, errstr.

        After

        .. code-block:: lua

            message_content, message_sender = recvfrom(1)

        the value of ``message_content`` might be a string containing 'X' and
        the value of ``message_sender`` might be a table containing
        ``message_sender.host = '18.44.0.1'``,
        ``message_sender.family = 'AF_INET'``,
        ``message_sender.port = 43065``.

    .. method:: shutdown(how)

        Shutdown a reading end, a writing end, or both ends of a socket.

        :param how: socket.SHUT_RD, socket.SHUT_WR, or socket.SHUT_RDWR.

        :return: true or false.
        :rtype:  boolean

    .. method:: close()

        Close (destroy) a socket. A closed socket should not be used any more.
        A socket is closed automatically when its userdata is garbage collected by Lua.

        :return: true on success, false on error. For example, if
                 sock is already closed, sock:close() returns false.
        :rtype:  boolean

    .. method:: error()
                errno()

        Retrieve information about the last error that occurred on a socket, if any.
        Errors do not cause throwing of exceptions so these functions are usually necessary.

        :return: result for ``sock:errno()``, result for ``sock:error()``.
                 If there is no error, then ``sock:errno()`` will return 0 and ``sock:error()``.
        :rtype:  number, string

    .. method:: setsockopt(level, name, value)

        Set socket flags. The argument values are the same as in the
        Linux man page [2]_.
        The ones that Bee accepts are:

            * SO_ACCEPTCONN
            * SO_BINDTODEVICE
            * SO_BROADCAST
            * SO_BSDCOMPAT
            * SO_DEBUG
            * SO_DOMAIN
            * SO_ERROR
            * SO_DONTROUTE
            * SO_KEEPALIVE
            * SO_MARK
            * SO_OOBINLINE
            * SO_PASSCRED
            * SO_PEERCRED
            * SO_PRIORITY
            * SO_PROTOCOL
            * SO_RCVBUF
            * SO_RCVBUFFORCE
            * SO_RCVLOWAT
            * SO_SNDLOWAT
            * SO_RCVTIMEO
            * SO_SNDTIMEO
            * SO_REUSEADDR
            * SO_SNDBUF
            * SO_SNDBUFFORCE
            * SO_TIMESTAMP
            * SO_TYPE

        Setting SO_LINGER is done with ``sock:linger(active)``.

    .. method:: getsockopt(level, name)

        Get socket flags. For a list of possible flags see ``sock:setsockopt()``.

    .. method:: linger([active])

        Set or clear the SO_LINGER flag. For a description of the flag, see
        Linux man page [3]_.

        :param boolean active:

        :return: new active and timeout values.

    .. method:: nonblock([flag])

        ``sock:nonblock()`` returns the current flag value. |br|
        ``sock:nonblock(false)`` sets the flag to false and returns false. |br|
        ``sock:nonblock(true)`` sets the flag to true and returns true.
        This function may be useful before invoking a function which might
        otherwise block indefinitely.

    .. method:: readable([timeout])
                writeable([timeout])
                wait([timeout])

        ``sock:readable()`` waits until something is readable, or until a timeout value expires. |br|
        ``sock:writable()`` waits until something is writable, or until a timeout value expires. |br|
        ``sock:wait()`` waits until something is either readable or writable, or until a timeout value expires.

    .. method:: name()

        The ``sock:name()`` function is used to get information about the
        near side of the connection. If a socket was bound to ``xyz.com:45``,
        then ``sock:name`` will return information about ``[host:xyz.com, port:45]``.
        The equivalent POSIX function is ``getsockname()``.

        :return: A table containing these fields: "host", "family", "type", "protocol", "port".
        :rtype:  table

    .. method:: peer()

        The ``sock:peer()`` function is used to get information about the far side of a connection.
        If a TCP connection has been made to a distant host ``bee.org:80``, ``sock:peer()``
        will return information about ``[host:bee.org, port:80]``.
        The equivalent POSIX function is ``getpeername()``.

        :return: A table containing these fields: "host", "family", "type", "protocol", "port".
        :rtype:  table

.. _Lua pattern:      http://www.lua.org/pil/20.2.html
.. _this description: https://github.com/bee/bee/wiki/sockets%201.6

=================================================
                    Example
=================================================

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Use of a TCP socket over the Internet
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this example a connection is made over the internet between the Bee
server and bee.org, then an HTTP "head" message is sent, and a response
is received: "``HTTP/1.1 200 OK``". This is not a useful way to communicate
with this particular site, but shows that the system works.


.. code-block:: lua

    bee> socket = require('socket')
    ---
    ...
    bee> sock = socket.tcp_connect('bee.org', 80)
    ---
    ...
    bee> type(sock)
    ---
    - table
    ...
    bee> sock:error()
    ---
    - null
    ...
    bee> sock:send("HEAD / HTTP/1.0\r\nHost: bee.org\r\n\r\n")
    ---
    - true
    ...
    bee> sock:read(17)
    ---
    - "HTTP/1.1 200 OK\r\n"
    ...
    bee> sock:close()
    ---
    - true
    ...

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Use of a UDP socket on localhost
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here is an example with datagrams. Set up two connections on 127.0.0.1
(localhost): ``sock_1`` and ``sock_2``. Using ``sock_2``, send a message
to ``sock_1``. Using ``sock_1``, receive a message. Display the received
message. Close both connections. |br| This is not a useful way for a
computer to communicate with itself, but shows that the system works.

.. code-block:: lua

    bee> socket = require('socket')
    ---
    ...
    bee>  sock_1 = socket('AF_INET', 'SOCK_DGRAM', 'udp')
    ---
    ...
    bee>  sock_1:bind('127.0.0.1')
    ---
    - true
    ...
    bee>  sock_2 = socket('AF_INET', 'SOCK_DGRAM', 'udp')
    ---
    ...
    bee>  sock_2:sendto('127.0.0.1', sock_1:name().port,'X')
    ---
    - true
    ...
    bee>  message = sock_1:recvfrom()
    ---
    ...
    bee>  message
    ---
    - X
    ...
    bee>  sock_1:close()
    ---
    - true
    ...
    bee>  sock_2:close()
    ---
    - true
    ...

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Use tcp_server to accept file contents sent with socat
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here is an example of the tcp_server function, reading
strings from the client and printing them. On the client
side, the Linux socat utility will be used to ship a
whole file for the tcp_server function to read.

Start two shells. The first shell will be the server.
The second shell will be the client.

On the first shell, start Bee and say:

.. code-block:: lua

    db.cfg{}
    socket = require('socket')
    console = require('console'); console.delimiter('!')
    socket.tcp_server('0.0.0.0',
                  3302,
                  function(s)
                    while true do
                      request = s:read("\n");
                      if request == "" then break end
                      if request == nil then break end
                      print(request)
                      end
                    end)
    console.delimiter('')!

The above code means: use tcp_server() to wait for a
connection from any host on port 3302. When it happens,
enter a loop that reads on the socket and prints what it
reads. The "delimiter" for the read function is "\\n" so
each read() will read a string as far as the next line feed,
including the line feed.

On the second shell, create a file that contains a few
lines. The contents don't matter. Suppose the first line
contains A, the second line contains B, the third line
contains C. Call this file "tmp.txt".

On the second shell, use the socat utility to ship the
tmp.txt file to the server's host and port:

.. code-block:: lua

    socat TCP:localhost:3302 ./tmp.txt

Now watch what happens on the first shell.
The strings "A", "B", "C" are printed.


.. _luasocket: https://github.com/diegonehab/luasocket

.. [1] http://man7.org/linux/man-pages/man2/connect.2.html
.. [2] http://man7.org/linux/man-pages/man2/setsockopt.2.html
.. [3] http://man7.org/linux/man-pages/man1/loginctl.1.html

