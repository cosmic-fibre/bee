# bee [![Build Status](https://travis-ci.org/bee/bee.png?branch=master)](https://travis-ci.org/bee/bee)

[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/bee/bee?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

http://bee.org

Bee is an efficient NoSQL database and a
Lua application server.

Key features of the Lua application server:
 * 100% compatible drop-in replacement for Lua 5.1,
   based on LuaJIT 2.0.
   Simply use #!/usr/bin/bee instead of
   #!/usr/bin/lua in your script.
 * full support for Lua modules and a rich set of
   own modules, including cooperative multitasking,
   non-blocking I/O, access to external databases, etc

Key features of the database:
 * MsgPack data format and MsgPack based
   client-server protocol
 * two data engines: 100% in-memory with
   optional persistence and a 2-level disk-based
   B-tree, to use with large data sets
 * multiple index types: HASH, TREE, BITSET
 * asynchronous master-master replication
 * authentication and access control
 * the database is just a C extension to the
   app server and can be turned off

Supported platforms are Linux/x86 and FreeBSD/x86, Mac OS X.

Bee is ideal for data-enriched components of
scalable Web architecture: queue servers, caches,
stateful Web applications.

## Compilation and install

Bee is written in C and C++.
To build, you will need GCC or Apple CLang compiler.

CMake is used for configuration management.
3 standard CMake build types are supported:
 * Debug -- used by project maintainers
 * RelWithDebugInfo -- the most common release configuration,
 also provides debugging capabilities
 * Release -- use only if the highest performance is required

The build depends on the following external libraries:

- libreadline and libreadline-dev
- GNU bfd (part of GNU binutils).

Please follow these steps to compile Bee:

    # If compiling from git
    bee $ git submodule update --init

    bee $ cmake .
    bee $ make

To use a different release type, say, RelWithDebugInfo, use:

    bee $ cmake . -DCMAKE_BUILD_TYPE=RelWithDebugInfo

Additional build options can be set similarly:

    bee $ cmake . -DCMAKE_BUILD_TYPE=RelWithDebugInfo -DENABLE_DOC=true # builds the docs

'make' creates 'bee' executable in directory src/.

There is no 'make install' goal, but no installation
is required either.

To start the server, try:

    bee $ ./src/bee

This will start Bee in interactive mode.

To run Bee regression tests (test/test-run.py),
a few additional Python modules are ncessary:
 * daemon
 * pyyaml
 * msgpack-python

Simply type 'make test' to fire off the test coverage.

Please report bugs at http://github.com/bee/bee/issues
We also warmly welcome your feedback in the discussion mailing
list, bee@googlegroups.com.

Thank you for your interest in Bee!
