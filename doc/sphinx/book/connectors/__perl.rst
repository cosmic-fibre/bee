=====================================================================
                            Perl
=====================================================================

The most commonly used Perl driver is `DR::Bee`_. It is not supplied as
part of the Bee repository; it must be installed separately. The most
common way to install it is with `CPAN, the Comprehensive Perl Archive Network`_.
`DR::Bee`_ requires other modules which should be installed first. For
example, on Ubuntu, the installation could look like this:

.. code-block:: bash

    sudo cpan install AnyEvent
    sudo cpan install Devel::GlobalDestruction
    sudo cpan install Coro
    sudo cpan install Test::Pod
    sudo cpan install Test::Spelling
    sudo cpan install PAR::Dist
    sudo cpan install List::MoreUtils
    sudo cpan install DR::Bee

Here is a complete Perl program that inserts [99999,'BB'] into space[999] via
the Perl API. Before trying to run, check that the server is listening and
that ``examples`` exists, as `described earlier`_. To run, paste the code into
a file named example.pl and say ``perl example.pl``. The program will connect
using an application-specific definition of the space. The program will open a
socket connection with the bee server at localhost:3301, then send an
INSERT request, then — if all is well — end without displaying any messages.
If bee is not running on localhost with listen address = 3301, the program
will print “Connection refused”.

.. code-block:: perl

    #!/usr/bin/perl
    use DR::Bee ':constant', 'bee';
    use DR::Bee ':all';
    use DR::Bee::MsgPack::SyncClient;

    my $tnt = DR::Bee::MsgPack::SyncClient->connect(
      host    => '127.0.0.1',                      # look for bee on localhost
      port    => 3301,                             # assume bee listen address = default
      user    => 'guest',                          # username. one could also say 'password=>...'

      spaces  => {
        999 => {                                   # definition of space[999] ...
          name => 'examples',                      #   space[999] name = 'examples'
          default_type => 'STR',                   #   space[999] field type is 'STR' if undefined
          fields => [ {                            #   definition of space[512].fields ...
              name => 'field1', type => 'NUM' } ], #     space[999].field[1] name='field1',type='NUM'
          indexes => {                             #   definition of space[999] indexes ...
            0 => {
              name => 'primary', fields => [ 'field1' ] } } } } );

    $tnt->insert('examples' => [ 99999, 'BB' ]);

The example program only shows one command and does not show all that's
necessary for good practice. For that, please see `DR::Bee`_ CPAN repository.

.. _DR::Bee: http://search.cpan.org/~unera/DR-Bee/
.. _CPAN, the Comprehensive Perl Archive Network: https://en.wikipedia.org/wiki/Cpan
.. _described earlier: https://en.wikipedia.org/wiki/Cpan
