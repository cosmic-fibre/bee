
http://foura.io

Bee is an backend platform as well as REST based enterprise solution with Lua application server.



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


    bee $ cmake .
    bee $ sudo make install

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



Thank you for your interest in Bee!
