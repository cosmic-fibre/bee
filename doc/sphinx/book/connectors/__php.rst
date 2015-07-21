=====================================================================
                            PHP
=====================================================================

The PHP driver is `bee-php`_. It is not supplied as part of the Bee
repository; it must be installed separately. It can be installed with git. It
requires other modules which should be installed first. For example, on Ubuntu,
the installation could look like this:

.. code-block:: bash

    sudo apt-get install php5-cli
    sudo apt-get install php5-dev
    sudo apt-get install php-pear
    cd ~
    git clone https://github.com/bee/bee-php.git
    cd bee-php
    phpize
    ./configure
    make
    # make install is optional

At this point there is a file named ``~/bee-php/modules/bee.so``.
PHP will only find it if the PHP initialization file ``php.ini`` contains a
line like ``extension=./bee.so``. So copy ``bee.so`` to the working
directory and tell PHP where to find the ``php.ini`` file that contains that line ...

.. code-block:: bash

    cd ~
    cp ./bee-php/modules/bee.so .
    export PHP_INI_SCAN_DIR=~/bee-php/test/shared

Here is a complete PHP program that inserts [99999,'BB'] into a space named 'examples'
via the PHP API. Before trying to run, check that the server is listening and that
``examples`` exists, as `described earlier`_. To run, paste the code into a file named
example.php and say ``php example.php``. The program will open a socket connection with
the bee server at localhost:3301, then send an INSERT request, then — if all is
well — print "Insert succeeded". If the tuple already exists, the program will print
“Duplicate key exists in unique index 'primary' in space 'examples'”.

.. code-block:: php

    <?php
    $bee = new Bee("localhost", 3301);
    try {
      $bee->insert("examples", array(99999, "BB"));
      print "Insert succeeded\n";
      }
    catch (Exception $e) {
      echo "Exception: ", $e->getMessage(), "\n";
      }
    ?>

After running the example, it is good practice to delete the file ./bee.so,
since it is only compatible with PHP and its existence could confuse non-PHP
applications.

The example program only shows one command and does not show all that's necessary
for good practice. For that, please see `bee-php`_ project at GitHub.

.. _described earlier: https://en.wikipedia.org/wiki/Cpan
.. _bee-php: https://github.com/bee/bee-php
